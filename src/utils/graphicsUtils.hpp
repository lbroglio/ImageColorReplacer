#pragma once

#include<wgpu.h>
#include<string>

using AdapterReadyCallback = void (*)(WGPUAdapter adapter, void* userdata);
using DeviceReadyCallback = void (*)(WGPUDevice device, void* userdata);


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
 * @param adapter The WebGPU adapter to use.
 * @param callback The callback function to be called when the device is ready.
 * @param userdata User data to be passed to the callback function.
 */
void request_device(WGPUAdapter adapter, DeviceReadyCallback callback, void* userdata);

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

/**
 * @brief Initializes a WebGPU compute pipeline with default settings.
 *
 * @param device The WebGPU device to use.
 * @param shader_module The WebGPU shader module to use.
 * @param entry_point The entry point of the shader.
 * @param pipeline_label The label of the pipeline.
 *
 * @return WGPUComputePipeline  The created WebGPU compute pipeline or NULL if creation failed.
 */
WGPUComputePipeline init_compute_pipeline(WGPUDevice device, WGPUShaderModule shader_module, std::string entry_point,
    std::string pipeline_label);

/**
 * @brief Initializes a WebGPU command encoder with default settings.
 *
 * @param device The WebGPU device to use.
 *
 * @return WGPUCommandEncoder The created WebGPU command encoder or NULL if creation failed.
 */
WGPUCommandEncoder init_command_encoder(WGPUDevice device);

/**
 * @brief Initializes a WebGPU compute pass encoder with default settings.
 *
 * @param encoder The WebGPU command encoder to use.
 *
 * @return WGPUComputePassEncoder The created WebGPU compute pass encoder or NULL if creation failed.
 */
WGPUComputePassEncoder init_compute_pass_encoder(WGPUCommandEncoder encoder);
