#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include <wgpu.h>


#include "../file-io/ppmIO.hpp"
#include "../terminal-io/errors.hpp"
#include "../graphics/graphicsUtils.hpp"
#include "../graphics/graphics.hpp"



// Globals 

// TODO Log for openGL errors

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
bool replaceColor(PPMImage* replaceIn, Pixel colorToChange, Pixel newColor, float distForChange){
    // TODO - Maybe switch to reading this in as a 1D array instead of a 2D array to avoid flattening it here
    // Flatten the 2D image data into a 1D array for GPU processing
    std::vector<Pixel> flatImageData;
    for (const auto& row : replaceIn->imageData) {
        flatImageData.insert(flatImageData.end(), row.begin(), row.end());
    }
    
    
    ColorReplaceCreateInfo info;
    info.image_details = {
        .length = static_cast<uint32_t>(replaceIn->length),
        .width = static_cast<uint32_t>(replaceIn->width)
    };
    info.replace_color = colorToChange;
    info.new_color = newColor;
    info.image = std::span(flatImageData);
    info.shader_code = std::string_view(shader_code, sizeof(shader_code));

    ColorReplacePipelineResources pipeline_resources;
    if (!pipeline_resources.create(info)) {
        return false;
    }



    WGPUCommandBufferDescriptor encoder_desc = {
        .nextInChain = NULL,
        .label = "Buffer Copy Encoder"
    };

    // TODO - Ensure this is freed after use
    WGPUCommandBuffer commandBuffer =
        wgpuCommandEncoderFinish(
            pipeline_resources.encoder,
            &encoder_desc
        );

    wgpuQueueSubmit(pipeline_resources.queue, 1, &commandBuffer);
    
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

    // TODO - Ensure this is freed after use
    WGPUBuffer staging_buffer = init_storage_buffer(
        pipeline_resources.device,
        sizeof(Pixel) * replaceIn->length * replaceIn->width,
        "StagingBuffer"
    );

    size_t buffer_size = sizeof(Pixel) * replaceIn->length * replaceIn->width;

    // Copy the results from the GPU back to the staging buffer
    wgpuCommandEncoderCopyBufferToBuffer(
        pipeline_resources.encoder,
        pipeline_resources.image_buffer,
        0,
        staging_buffer,
        0,
        buffer_size
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

    wgpuBufferMapAsync(
        staging_buffer,
        WGPUMapMode_Read,
        0,
        buffer_size,
        callback_info
    );

    // 4. Wait for mapping to complete
    while (!result.done) {
        wgpuDevicePoll(pipeline_resources.device, true, NULL);
    }

    if (!result.success) {
        wgpuBufferRelease(staging_buffer);
        wgpuCommandBufferRelease(commands);
       return false; // Mapping failed
    }

    // Read the data from the mapped staging buffer and update the original image
    const Pixel* data = (const Pixel*)
    wgpuBufferGetConstMappedRange(
        staging_buffer,
        0,
        buffer_size
    );

    if (data != NULL) {
        for (int i = 0; i < buffer_size / sizeof(Pixel); i++) {
            int row = i / replaceIn->width;
            int col = i % replaceIn->width;
            replaceIn->imageData[row][col] = data[i];
        }
    }
    // TODO - Else error

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
    const char* changeColorNums[3];
    const char* newColorNums[3];

    //TODO - Break out into functions in terminal-io
    int firstComma = changeColorStr.find_first_of(',');
    int secondComma =  changeColorStr.find_last_of(',');
    changeColorNums[0] = changeColorStr.substr(0, firstComma).c_str();
    changeColorNums[1] = changeColorStr.substr(firstComma + 1, secondComma - (firstComma + 1)).c_str();
    changeColorNums[2] = changeColorStr.substr(secondComma + 1).c_str();

    // TODO Remove after dev is done
    std::cout << "Color to change: " << changeColorNums[0] << ", " << changeColorNums[1] << ", " << changeColorNums[2] << std::endl;

    firstComma = newColorStr.find_first_of(',');
    secondComma =  newColorStr.find_last_of(',');
    newColorNums[0] = newColorStr.substr(0, firstComma).c_str();
    newColorNums[1] = newColorStr.substr(firstComma + 1, secondComma - (firstComma + 1)).c_str();
    newColorNums[2] = newColorStr.substr(secondComma + 1).c_str();

    // TODO Remove after dev is done
    std::cout << "New Color " << newColorNums[0] << ", " << newColorNums[1] << ", " << newColorNums[2] << std::endl;


    // Initialize those colors as pixel objects
    Pixel changeColor;
    changeColor.r = atoi(changeColorNums[0]);
    changeColor.g = atoi(changeColorNums[1]);
    changeColor.b = atoi(changeColorNums[2]);

    Pixel newColor;
    newColor.r = atoi(newColorNums[0]);
    newColor.g = atoi(newColorNums[1]);
    newColor.b = atoi(newColorNums[2]);

    // Read in image
    PPMImage toProc = readInPPM(filePath);

    // Change any pixels within the the given dist of change color to new color using the GPU
    // TODO -- Alert User on error
    // TODO -- Don't hard code the distance
    replaceColor(&toProc, changeColor, newColor, 1);

    // Output to a new PPM
    std::ofstream outFile("swapppepColors.ppm");
    PPMOut(&outFile, toProc.imageData, toProc.length, toProc.width);

    outFile.close();

}