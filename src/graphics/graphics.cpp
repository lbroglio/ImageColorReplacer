#include <thread>
#include <string>
#include <cstring>


#include <wgpu.h>

#include "graphics.hpp"
#include "graphicsUtils.hpp"

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


ColorReplacePipelineResources::~ColorReplacePipelineResources() {
    if (image_buffer) {
        wgpuBufferRelease(image_buffer);
    }
    if (new_color_buffer) {
        wgpuBufferRelease(new_color_buffer);
    }
    if (replace_color_buffer) {
        wgpuBufferRelease(replace_color_buffer);
    }
    if (image_details_buffer) {
        wgpuBufferRelease(image_details_buffer);
    }
    if (bind_group) {
        wgpuBindGroupRelease(bind_group);
    }
    if (bind_group_layout) {
        wgpuBindGroupLayoutRelease(bind_group_layout);
    }
    if (pass_encoder) {
        wgpuComputePassEncoderRelease(pass_encoder);
    }
    if (encoder) {
        wgpuCommandEncoderRelease(encoder);
    }
    if (pipeline) {
        wgpuComputePipelineRelease(pipeline);
    }
    if (shader_module) {
        wgpuShaderModuleRelease(shader_module);
    }
    if (queue) {
        wgpuQueueRelease(queue);
    }
    if (device) {
        wgpuDeviceRelease(device);
    }
    if (adapter) {
        wgpuAdapterRelease(adapter);
    }
    if (instance) {
        wgpuInstanceRelease(instance);
    }
}

bool ColorReplacePipelineResources::create(const ColorReplaceCreateInfo& info) {
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
    WGPUShaderModule shader_module = init_shader_module(device, info.shader_code, "ColorReplaceShader");
    WGPUComputePipeline pipeline = init_compute_pipeline(device, shader_module, "main", "ColorReplacePipeline");
    WGPUCommandEncoder encoder = init_command_encoder(device);
    WGPUComputePassEncoder pass_encoder = init_compute_pass_encoder(encoder);
    WGPUBindGroupLayout bind_group_layout = init_bind_group_layout(pipeline, "ColorReplaceBindGroupLayout");
    
    // Setup the image details buffer

    // Setup the buffers the shader needs
    WGPUBuffer image_details_buffer = 
        init_uniform_buffer(device, sizeof(ImageDetails), "ImageDetailsBuffer");
    void* mappedMemory = wgpuBufferGetMappedRange(image_details_buffer, 0, sizeof(ImageDetails));
    std::memcpy(mappedMemory, &info.image_details, sizeof(ImageDetails));
    wgpuBufferUnmap(image_details_buffer);


    WGPUBuffer replace_color_buffer = 
        init_uniform_buffer(device, sizeof(Pixel), "ColorToChangeBuffer");
    mappedMemory = wgpuBufferGetMappedRange(replace_color_buffer, 0, sizeof(Pixel));
    std::memcpy(mappedMemory, &info.replace_color, sizeof(Pixel));
    wgpuBufferUnmap(replace_color_buffer);
    
    WGPUBuffer new_color_buffer = 
        init_uniform_buffer(device, sizeof(Pixel), "NewColorBuffer");
    mappedMemory = wgpuBufferGetMappedRange(new_color_buffer, 0, sizeof(Pixel));
    std::memcpy(mappedMemory, &info.new_color, sizeof(Pixel));
    wgpuBufferUnmap(new_color_buffer);
    
    size_t image_buffer_size = sizeof(Pixel) * info.image_details.length * info.image_details.width;
    WGPUBuffer image_buffer = init_storage_buffer(device, image_buffer_size, "ImageBuffer");
    mappedMemory = wgpuBufferGetMappedRange(image_buffer, 0, image_buffer_size);
    std::memcpy(mappedMemory, info.image.data(), image_buffer_size);
    wgpuBufferUnmap(image_buffer);

    // Setup the bind group entries
    WGPUBindGroupEntry entries[4] = {
        init_bind_group_entry(0, image_details_buffer, 0, sizeof(ImageDetails)),
        init_bind_group_entry(1, replace_color_buffer, 0, sizeof(Pixel)),
        init_bind_group_entry(2, new_color_buffer, 0, sizeof(Pixel)),
        init_bind_group_entry(3, image_buffer, 0, image_buffer_size)
    };

    WGPUBindGroup bind_group = init_bind_group(device, bind_group_layout, entries, 4, "ColorReplaceBindGroup");

    // If any item in the above initialization failed, return false

    // Finish setup and dispatch the compute shader
    wgpuComputePassEncoderSetPipeline(pass_encoder, pipeline);
    wgpuComputePassEncoderSetBindGroup(pass_encoder, 0, bind_group, 0, NULL);

    // Calculate the number of workgroups needed based on the image size
    uint32_t workgroupCountX = (info.image_details.length + 7) / 8; 
    uint32_t workgroupCountY = (info.image_details.width + 7) / 8; 

    wgpuComputePassEncoderDispatchWorkgroups(pass_encoder, workgroupCountX, workgroupCountY, 1);

    wgpuComputePassEncoderEnd(pass_encoder);

    return true;
}