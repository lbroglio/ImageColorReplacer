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
    if (input_image_texture_view) {
        wgpuTextureViewRelease(input_image_texture_view);
    }
    if (new_image_texture_view) {
        wgpuTextureViewRelease(new_image_texture_view);
    }
    if (input_image_texture) {
        wgpuTextureDestroy(input_image_texture);
        wgpuTextureRelease(new_image_texture);
    }
    if (new_image_texture) {
        wgpuTextureDestroy(new_image_texture);
        wgpuTextureRelease(new_image_texture);
    }
    if (target_color_buffer) {
        wgpuBufferRelease(target_color_buffer);
    }
    if (new_color_buffer) {
        wgpuBufferRelease(new_color_buffer);
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
    if (distance_threshold_buffer) {
        wgpuBufferRelease(distance_threshold_buffer);
    }
}

bool ColorReplacePipelineResources::create(const ColorReplaceCreateInfo& info) {
    // Initialize WebGPU objects
    this->instance = init_instance();
    if (!this->instance) {
        return false;
    }

    AdapterRequestState adapter_state;
    request_adapter(this->instance, on_adapter_ready, &adapter_state);
    while (!adapter_state.complete) {
        wgpuInstanceProcessEvents(this->instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    this->adapter = adapter_state.adapter;
    if (!this->adapter) {
        return false;
    }

    DeviceRequestState device_state;
    request_device(this->adapter, on_device_ready, &device_state);
    while (!device_state.complete) {
        wgpuInstanceProcessEvents(this->instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    this->device = device_state.device;
    if (!this->device) {
        return false;
    }
    this->queue = init_queue(this->device);
    this->shader_module = init_shader_module(this->device, info.shader_code, "ColorReplaceShader");
    this->pipeline = init_compute_pipeline(this->device, this->shader_module, "main", "ColorReplacePipeline");
    this->encoder = init_command_encoder(this->device);
    this->pass_encoder = init_compute_pass_encoder(this->encoder);
    this->bind_group_layout = init_bind_group_layout(this->pipeline, "ColorReplaceBindGroupLayout");
    
    // Setup the image details buffer
    this->image_details_buffer = 
        init_uniform_buffer(this->device, sizeof(ImageDetails), "ImageDetailsBuffer");
    void* mappedMemory = wgpuBufferGetMappedRange(this->image_details_buffer, 0, sizeof(ImageDetails));
    std::memcpy(mappedMemory, &info.image_details, sizeof(ImageDetails));
    wgpuBufferUnmap(this->image_details_buffer);



    this->target_color_buffer = 
        init_uniform_buffer(this->device, sizeof(Pixel), "TargetColorBuffer");
    mappedMemory = wgpuBufferGetMappedRange(this->target_color_buffer, 0, sizeof(Pixel));
    std::memcpy(mappedMemory, &info.target_color, sizeof(Pixel));
    wgpuBufferUnmap(this->target_color_buffer);
    
    this->new_color_buffer = 
        init_uniform_buffer(this->device, sizeof(Pixel), "NewColorBuffer");
    mappedMemory = wgpuBufferGetMappedRange(this->new_color_buffer, 0, sizeof(Pixel));
    std::memcpy(mappedMemory, &info.new_color, sizeof(Pixel));
    wgpuBufferUnmap(this->new_color_buffer);
    

    // TODO - Fix

    this->input_image_texture = init_texture(this->device, info.image_details.width, info.image_details.height, "InputImageTexture");
    this->input_image_texture_view = init_texture_view(this->input_image_texture, "InputImageTextureView");
    
    this->new_image_texture = init_texture(this->device, info.image_details.width, info.image_details.height, "NewImageTexture");
    this->new_image_texture_view = init_texture_view(this->new_image_texture, "NewImageTextureView");

        
    WGPUExtent3D texture_extent = {
        .width = info.image_details.width,
        .height = info.image_details.height,
        .depthOrArrayLayers = 1
    };

    WGPUTexelCopyTextureInfo destination = {
        .texture = this->input_image_texture,
        .mipLevel = 0,
        .origin = (WGPUOrigin3D){ .x = 0, .y = 0, .z = 0 },
        .aspect = WGPUTextureAspect_All
    };

    WGPUTexelCopyBufferLayout data_layout = {
        .bytesPerRow = info.image_details.width * 4, // 4 bytes per pixel for RGBA8
        .rowsPerImage = info.image_details.height
    };

    // Enqueue the copy command using the device queue
    size_t data_size = info.image_details.width * info.image_details.height * 4;
    wgpuQueueWriteTexture(queue, &destination, &info.image_details.image_data, data_size, &data_layout, 
        &texture_extent);

    this->distance_threshold_buffer = 
        init_uniform_buffer(this->device, sizeof(std::uint32_t), "DistanceThresholdBuffer");
    mappedMemory = wgpuBufferGetMappedRange(this->distance_threshold_buffer, 0, sizeof(std::uint32_t));
    std::memcpy(mappedMemory, &info.distance_threshold, sizeof(std::uint32_t));
    wgpuBufferUnmap(this->distance_threshold_buffer);

    // Setup the bind group entries
    WGPUBindGroupEntry entries[6] = {
        init_bind_group_entry(0, this->image_details_buffer, 0, sizeof(ImageDetails)),
        init_bind_group_entry(1, this->target_color_buffer, 0, sizeof(Pixel)),
        init_bind_group_entry(2, this->new_color_buffer, 0, sizeof(Pixel)),
        init_bind_group_entry_for_texture(3, this->input_image_texture_view, 0),
        init_bind_group_entry_for_texture(4, this->new_image_texture_view, 0),
        init_bind_group_entry(5, this->distance_threshold_buffer, 0, sizeof(std::uint32_t))
    };

    this->bind_group = init_bind_group(
        this->device, this->bind_group_layout, entries, 6, "ColorReplaceBindGroup");

    // If any item in the above initialization failed, return false
    if (!this->bind_group) {
        return false;
    }

    // Finish setup and dispatch the compute shader
    wgpuComputePassEncoderSetPipeline(this->pass_encoder, this->pipeline);
    wgpuComputePassEncoderSetBindGroup(this->pass_encoder, 0, this->bind_group, 0, NULL);

    // Calculate the number of workgroups needed based on the image size
    uint32_t workgroupCountX = (info.image_details.width + 7) / 8; 
    uint32_t workgroupCountY = (info.image_details.height + 7) / 8; 

    wgpuComputePassEncoderDispatchWorkgroups(this->pass_encoder, workgroupCountX, workgroupCountY, 1);

    wgpuComputePassEncoderEnd(this->pass_encoder);

    return true;
}