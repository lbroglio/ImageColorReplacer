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
WGPUShaderModule init_shader_module(WGPUDevice device, std::string_view shader_code, std::string_view shader_name);

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
WGPUComputePipeline init_compute_pipeline(WGPUDevice device, WGPUShaderModule shader_module, std::string_view entry_point,
    std::string_view pipeline_label);

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

/**
 * @brief Initializes a WebGPU uniform buffer with default settings.
 * 
 * @param device The WebGPU device to use.
 * @param size The size of the buffer in bytes.
 * @param buffer_label The label of the buffer.
 * @return WGPUBuffer The created WebGPU buffer or NULL if creation failed.
 */
WGPUBuffer init_uniform_buffer(WGPUDevice device, uint64_t size, std::string buffer_label);

/**
 * @brief Initializes a WebGPU storage buffer with default settings.
 *
 * @param device The WebGPU device to use.
 * @param size The size of the buffer in bytes.
 * @param buffer_label The label of the buffer.
 * @return WGPUBuffer The created WebGPU buffer or NULL if creation failed.
 */
WGPUBuffer init_storage_buffer(WGPUDevice device, uint64_t size, std::string buffer_label);

/**
 * @brief Initializes a WebGPU bind group entry with default settings.
 *
 * @param binding The binding index.
 * @param buffer The WebGPU buffer to use.
 * @param offset The offset within the buffer.
 * @param size The size of the entry.
 * @return WGPUBindGroupEntry The created WebGPU bind group entry or NULL if creation failed.
 */
WGPUBindGroupEntry init_bind_group_entry(uint32_t binding, WGPUBuffer buffer, uint64_t offset, uint64_t size);

/**
 * @brief Initializes a WebGPU bind group layout with default settings.
 *
 * @param pipeline The WebGPU compute pipeline to use.
 * @param bind_group_layout_label The label of the bind group layout.
 *
 * @return WGPUBindGroupLayout The created WebGPU bind group layout or NULL if creation failed.
 */
WGPUBindGroupLayout init_bind_group_layout(WGPUComputePipeline pipeline, std::string bind_group_layout_label);

/**
 * @brief Initializes a WebGPU bind group with default settings.
 * 
 * @param device The WebGPU device to use.
 * @param bind_group_layout The WebGPU bind group layout to use.
 * @param entries The entries for the bind group.
 * @param entry_count The number of entries in the bind group.
 * @param bind_group_label The label of the bind group.
 * @return WGPUBindGroup The created WebGPU bind group or NULL if creation failed.
 */
WGPUBindGroup init_bind_group(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUBindGroupEntry* entries, size_t entry_count, std::string bind_group_label);

