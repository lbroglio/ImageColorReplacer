#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <algorithm>

#include <wgpu.h>

#include "../file-io/pngIO.hpp"
#include "../terminal-io/errors.hpp"
#include "../graphics/graphicsUtils.hpp"
#include "../graphics/graphics.hpp"
#include "../terminal-io/input.hpp"



// Globals 


// Embedded shader code
const char shader_code[] = {
    #embed "../../resources/ICRShader.wgsl"
};

struct MapResult {
    bool done;
    bool success;
};

static void on_buffer_mapped(
    WGPUMapAsyncStatus status,
    WGPUStringView message,
    void* userdata1,
    void* userdata2
) {
    MapResult* result = static_cast<MapResult*>(userdata1);

    result->success = (status == WGPUMapAsyncStatus_Success);
    result->done = true;
}


static void on_gpu_work_done(WGPUQueueWorkDoneStatus status, WGPUStringView message, void *userdata1, void *userdata2) {
    bool *done = (bool *)userdata1;

    if (status != WGPUQueueWorkDoneStatus_Success) {
        printf("GPU work failed: %.*s\n",
            (int)message.length,
            message.data);
    }

    *done = true;
}



/**
 * @brief For each Pixel in the image calculate if it should be changed and reset it using the GPU
 * 
 * @param replaceIn The PPMImage to operate on
 * @param colorToChange The color that should be changed
 * @param newColor The color to replace colorToChangeWith
 * @param distForChange If the difference (found with 3D distance formula) of a color is less than this it will be replaced
 * @return true The operation was successful
 * @return false The operation failed for some reason (related to the GPU)
 */
bool replaceColor(ImageDetails* image_details, Pixel color_to_change, Pixel new_color, std::float32_t change_distance_threshold,
    std::string* error_message = nullptr) {
        
    ColorReplaceCreateInfo info(
        std::string_view(shader_code, sizeof(shader_code)),
        *image_details,
        color_to_change,
        new_color,
        change_distance_threshold
    );

    ColorReplacePipelineResources pipeline_resources;
    if (!pipeline_resources.create(info)) {
        *error_message = "Failed to create pipeline resources.";
        return false;
    }


    // Setup callback 
    bool done = false;
    WGPUQueueWorkDoneCallbackInfo callbackInfo = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_gpu_work_done,
        .userdata1 = &done,
        .userdata2 = NULL
    };

    wgpuQueueOnSubmittedWorkDone(pipeline_resources.queue, callbackInfo);

    int wait_iterations = 0;
    while (!done) {
        wgpuInstanceProcessEvents(pipeline_resources.instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        wait_iterations++;
        if (wait_iterations > 10000) {
            *error_message = "Timeout waiting for GPU work to complete.";
            return false;
        }
    }


    WGPUBuffer staging_buffer = init_staging_buffer(
        pipeline_resources.device,
        4 * image_details->height * image_details->width,
        "StagingBuffer"
    );

    WGPUTexelCopyTextureInfo source_texture = {
        .texture = pipeline_resources.new_image_texture,
        .mipLevel = 0,
        .origin = { 0, 0, 0 },
        .aspect = WGPUTextureAspect_All
    };

    WGPUTexelCopyBufferInfo dest_buf = {
        .layout = {
            .offset = 0,
            .bytesPerRow = 4 * image_details->width,
            .rowsPerImage = image_details->height
        },
        .buffer = staging_buffer
    };

    WGPUExtent3D copy_size = {
        .width = image_details->width,
        .height = image_details->height,
        .depthOrArrayLayers = 1
    };

    // Copy the results from the GPU back to the staging buffer
    wgpuCommandEncoderCopyTextureToBuffer(
        pipeline_resources.encoder,
        &source_texture,
        &dest_buf,
        &copy_size
    );


    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(pipeline_resources.encoder, NULL);

    wgpuQueueSubmit(pipeline_resources.queue, 1, &commands);

    // Map the staging buffer to read back the results
    MapResult result = {0};

    WGPUBufferMapCallbackInfo callback_info = {
        .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback = on_buffer_mapped,
        .userdata1 = &result
    };

    std::uint64_t buffer_size = 4 * image_details->width * image_details->height;

    wgpuBufferMapAsync(
        staging_buffer,
        WGPUMapMode_Read,
        0,
        buffer_size,
        callback_info
    );

    // Wait for mapping to complete
    while (!result.done) {
        wgpuDevicePoll(pipeline_resources.device, true, NULL);
    }

    if (!result.success) {
        wgpuBufferRelease(staging_buffer);
        wgpuCommandBufferRelease(commands);
        *error_message = "Failed to map buffer.";
        return false; // Mapping failed
    }

    // Read the data from the mapped staging buffer and update the original image
    unsigned char* data = (unsigned char*) wgpuBufferGetConstMappedRange(
        staging_buffer,
        0,
        buffer_size
    );

    if (!data) {
        wgpuBufferRelease(staging_buffer);
        wgpuCommandBufferRelease(commands);
        *error_message = "Failed to get mapped range from staging buffer.";
        return false; // Failed to get mapped range
    }

    std::memcpy(image_details->image_data, data, buffer_size);

    wgpuBufferRelease(staging_buffer);
    wgpuCommandBufferRelease(commands);

    return true;
}

int main(int argc, char* argv[]){
    


    std::vector<std::string> args(argv + 1, argv + argc);

    if (std::ranges::contains(args,  std::string{"-h"})) {
        std::cout << "Usage: imageColorReplace <image_path> <color_to_change> <new_color> [-d <distance_threshold>]\n"
                  "  <image_path>: Path to the input PNG image.\n"
                  "  <color_to_change>: Color to change in the format R,G,B (e.g., 255,0,0 for red).\n"
                  "  <new_color>: New color to replace with in the format R,G,B (e.g., 0,255,0 for green).\n"
                  "  -d <distance_threshold>: Optional. Distance threshold for color replacement (default is 0.05).\n";
        return 0;
    }

    // Parse arguments
    std::string filePath = args[0];
    std::string changeColorStr = args[1];
    std::string newColorStr = args[2];

    float distance_threshold = 0.05f;

    std::string distance_threshold_str = extract_argument_value(&args, "-d");
    if (!distance_threshold_str.empty()) {
        try {
            distance_threshold = std::stof(distance_threshold_str);
        } catch (const std::invalid_argument& e) {
            fatal_error("Invalid distance threshold value: " + distance_threshold_str);
        }
    }


    Pixel change_color;
    try{
        change_color = parse_color_string(changeColorStr);
    }
    catch (const std::invalid_argument& e) {
        fatal_error(e.what());
    }

    Pixel new_color;
    try{
        new_color = parse_color_string(newColorStr);
    }
    catch (const std::invalid_argument& e) {
        fatal_error(e.what());
    }

    // Read in image
    ImageDetails* image_details = readin_png_image(filePath);

    // Change any pixels within the the given dist of change color to new color using the GPU
    std::string error_message;
    bool success = replaceColor(image_details, change_color, new_color, distance_threshold, &error_message);
    if (!success) {
        fatal_error(error_message);
    }

    // Output to a new PPM
    int result = writeout_png_image("output.png", image_details);

    delete image_details;

    if (result != 1) {
        fatal_error("Failed to write output image.");
    }
}