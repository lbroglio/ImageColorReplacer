#pragma once

#include<wgpu.h>
#include<string>

using AdapterReadyCallback = void (*)(WGPUAdapter adapter, void* userdata);


/**
 * @brief Callback function for handling adapter requests.
 */
static void on_adapter_request(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata, void *userdata2);

/**
 * @brief Callback function for handling device requests.
 */
static void on_device_request(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata, void *userdata2);

/**
 * @brief Initializes a WebGPU instance with default settings.
 * 
 * @return WGPUInstance The created WebGPU instance or NULL if creation failed.
 */
WGPUInstance init_instance();



/**
 * @brief Requests a WebGPU adapter.
 * 
 * @param instance The WebGPU instance to use.
 * @param callback The callback function to be called when the adapter is ready.
 * @param userdata User data to be passed to the callback function.
 */
void request_adapter(WGPUInstance instance, AdapterReadyCallback callback, void* userdata);

/**
 * @brief Initializes a WebGPU device with default settings.
 * 
 * @param adapter The WebGPU adapter to use.
 * 
 * @return WGPUDevice The created WebGPU device or NULL if creation failed.
 */
WGPUDevice init_device(WGPUAdapter adapter);

/**
 * @brief Initializes a WebGPU queue with default settings.
 * 
 * @param device The WebGPU device to use.
 * 
 * @return WGPUQueue The created WebGPU queue or NULL if creation failed.
 */
WGPUQueue init_queue(WGPUDevice device);


/**
 * @brief Initializes a WebGPU shader module with default settings.
 * 
 * @param device The WebGPU device to use.
 * @param shader_code The shader code to use.
 * @param code_size The size of the shader code.
 * @param shader_name The name of the shader.
 * 
 * @return WGPUShaderModule The created WebGPU shader module or NULL if creation failed.
 */
WGPUShaderModule init_shader_module(WGPUDevice device, const char* shader_code, std::string shader_name);
