#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include <wgpu.h>

#include "../file-io/pngIO.hpp"
#include "../terminal-io/errors.hpp"
#include "../graphics/graphicsUtils.hpp"
#include "../graphics/graphics.hpp"



// Globals 


// Embedded shader code
const char shader_code[] = {
    #embed "../../resources/ICRShader.wgsl"
};

struct MapResult {
    bool done;
    bool success;
};

// TODO - Maybe move this elsewhere
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
bool replaceColor(ImageDetails* image_details, Pixel color_to_change, Pixel new_color, std::float32_t change_distance_threshold){
        
    ColorReplaceCreateInfo info(
        std::string_view(shader_code, sizeof(shader_code)),
        *image_details,
        color_to_change,
        new_color,
        change_distance_threshold
    );

    ColorReplacePipelineResources pipeline_resources;
    if (!pipeline_resources.create(info)) {
        return false;
    }



    WGPUCommandBufferDescriptor encoder_desc = {
        .nextInChain = NULL,
        .label = "Buffer Copy Encoder"
    };

    // Setup callback 
    bool done = false;
    WGPUQueueWorkDoneCallbackInfo callbackInfo = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_gpu_work_done,
        .userdata1 = &done,
        .userdata2 = NULL
    };

    wgpuQueueOnSubmittedWorkDone(pipeline_resources.queue, callbackInfo);

    while (!done) {
        wgpuInstanceProcessEvents(pipeline_resources.instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // TODO - Add timeout to avoid infinite loop
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


    // TODO - Ensure this is freed after use
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(pipeline_resources.encoder, NULL);

    wgpuQueueSubmit(pipeline_resources.queue, 1, &commands);

    MapResult map_result = { .done = false, .success = false };

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
        return false; // Failed to get mapped range
    }

    std::memcpy(image_details->image_data, data, buffer_size);

    // TODO - Move the
    wgpuBufferRelease(staging_buffer);
    wgpuCommandBufferRelease(commands);

    return true;
}

int main(int argc, char* argv[]){
    // TODO -- Add usage with -h

    // TODO -- Add error handling for args
    // Parse arguments
    // TODO -- IO with PNGs converted to/or from PPMs
    std::string filePath = argv[1];
    std::string changeColorStr = argv[2];
    std::string newColorStr = argv[3];

    // TODO - Add distance arg

    // Parse color args 
    std::string changeColorNums[3];
    std::string newColorNums[3];

    //TODO - Break out into functions in terminal-io
    int firstComma = changeColorStr.find_first_of(',');
    int secondComma =  changeColorStr.find_last_of(',');
    changeColorNums[0] = changeColorStr.substr(0, firstComma);
    changeColorNums[1] = changeColorStr.substr(firstComma + 1, secondComma - (firstComma + 1));
    changeColorNums[2] = changeColorStr.substr(secondComma + 1);

    firstComma = newColorStr.find_first_of(',');
    secondComma =  newColorStr.find_last_of(',');
    newColorNums[0] = newColorStr.substr(0, firstComma);
    newColorNums[1] = newColorStr.substr(firstComma + 1, secondComma - (firstComma + 1));
    newColorNums[2] = newColorStr.substr(secondComma + 1);

    // Initialize those colors as pixel objects
    Pixel changeColor;
    changeColor.r = map_255_to_1(atoi(changeColorNums[0].c_str()));
    changeColor.g = map_255_to_1(atoi(changeColorNums[1].c_str()));
    changeColor.b = map_255_to_1(atoi(changeColorNums[2].c_str()));
    changeColor.a = 1.0f; // Set alpha to fully opaque

    Pixel newColor;
    newColor.r = map_255_to_1(atoi(newColorNums[0].c_str()));
    newColor.g = map_255_to_1(atoi(newColorNums[1].c_str()));
    newColor.b = map_255_to_1(atoi(newColorNums[2].c_str()));
    newColor.a = 1.0f; // Set alpha to fully opaque

    // Read in image
    ImageDetails* image_details = readin_png_image(filePath);

    // Change any pixels within the the given dist of change color to new color using the GPU
    // TODO -- Alert User on error
    // TODO -- Don't hard code the distance
    replaceColor(image_details, changeColor, newColor, 0.05f);

    // Output to a new PPM
    int result = writeout_png_image("output.png", image_details);

    delete image_details;

    // TODO - Terminal error handling for result
}