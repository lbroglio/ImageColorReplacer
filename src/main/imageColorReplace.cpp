#include <wgpu.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include "../file-io/ppmIO.hpp"
#include "../terminal-io/errors.hpp"
#include "../utils/graphicsUtils.hpp"



// Globals 

// TODO Log for openGL errors

// Embedded shader code
const char shader_code[] = {
    #embed "../../resources/ICRShader.wgsl"
};

struct AdapterRequestState {
    WGPUAdapter adapter = nullptr;
    bool complete = false;
};

struct DeviceRequestState {
    WGPUDevice device = nullptr;
    bool complete = false;
};

static void on_adapter_ready(WGPUAdapter adapter, void* userdata) {
    auto* state = static_cast<AdapterRequestState*>(userdata);
    state->adapter = adapter;
    state->complete = true;
}

static void on_device_ready(WGPUDevice device, void* userdata) {
    auto* state = static_cast<DeviceRequestState*>(userdata);
    state->device = device;
    state->complete = true;
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
    // TODO - Break out creation into more functions if needed 

    // Load the shader code from file
    
    // Initialize WebGPU objects
    WGPUInstance instance = init_instance();
    if (!instance) {
        return false;
    }

    AdapterRequestState adapter_state;
    request_adapter(instance, on_adapter_ready, &adapter_state);
    while (!adapter_state.complete) {
        wgpuInstanceProcessEvents(instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    WGPUAdapter adapter = adapter_state.adapter;
    if (!adapter) {
        return false;
    }

    DeviceRequestState device_state;
    request_device(adapter, on_device_ready, &device_state);
    while (!device_state.complete) {
        wgpuInstanceProcessEvents(instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    WGPUDevice device = device_state.device;
    if (!device) {
        return false;
    }
    WGPUQueue queue = init_queue(device);
    WGPUShaderModule shader_module = init_shader_module(device, shader_code, "ColorReplaceShader");
    WGPUComputePipeline pipeline = init_compute_pipeline(device, shader_module, "main", "ColorReplacePipeline");
    WGPUCommandEncoder encoder = init_command_encoder(device);
    WGPUComputePassEncoder pass_encoder = init_compute_pass_encoder(encoder);

    // If any item in the above initialization failed, return false
    if (!queue || !shader_module || !pipeline || !encoder || !pass_encoder) {
        return false;
    }

    


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