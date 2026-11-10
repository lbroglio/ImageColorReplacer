#include<wgpu.h>
#include<string>

#include "graphicsUtils.hpp"


struct AdapterRequestContext {
    AdapterReadyCallback callback;
    void* userdata;
};

struct DeviceRequestContext {
    DeviceReadyCallback callback;
    void* userdata;
};

WGPUStringView to_wgpu_string(std::string_view value) {
    return {
        .data = value.data(),
        .length = value.size()
    };
}

static void on_adapter_request(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2) {
    auto* context = static_cast<AdapterRequestContext*>(userdata1);

    context->callback(
        status == WGPURequestAdapterStatus_Success ? adapter : nullptr,
        context->userdata
    );
    delete context;
}

static void on_device_request(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata, void *userdata2) {
    auto* context = static_cast<DeviceRequestContext*>(userdata);
    context->callback(
        status == WGPURequestDeviceStatus_Success ? device : nullptr,
        context->userdata
    );
    delete context;
}

WGPUInstance init_instance(){
    WGPUInstanceDescriptor desc = {
        .nextInChain = NULL 
       };

    WGPUInstance instance = wgpuCreateInstance(&desc);

    if (!instance) {
        return NULL;
    }

    return instance;
}

void request_adapter(WGPUInstance instance,AdapterReadyCallback callback,void* userdata) {
    auto* context = new AdapterRequestContext{callback, userdata};
    WGPURequestAdapterCallbackInfo callback_info = {
        .nextInChain = nullptr,
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_adapter_request,
        .userdata1 = context,
        .userdata2 = nullptr
    };

    wgpuInstanceRequestAdapter(
        instance,
        nullptr,
        callback_info
    );
}


void request_device(WGPUAdapter adapter, DeviceReadyCallback callback, void* userdata) {
    auto* context = new DeviceRequestContext{callback, userdata};
    wgpuAdapterRequestDevice(adapter, NULL, (WGPURequestDeviceCallbackInfo){
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_device_request,
        .userdata1 = context
    });
}

WGPUQueue init_queue(WGPUDevice device) {
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    if (!queue) {
        return NULL;
    }
    return queue;
}



WGPUShaderModule init_shader_module(WGPUDevice device, std::string_view shader_code, std::string_view shader_name) {
   WGPUShaderSourceWGSL shader_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceWGSL 
        },
        .code = to_wgpu_string(shader_code)
    };

   WGPUShaderModuleDescriptor shader_desc = {
        .nextInChain = &shader_source.chain,
        .label = to_wgpu_string(shader_name)
    };

    WGPUShaderModule shader_module = wgpuDeviceCreateShaderModule(
        device, 
        &shader_desc
    );

    if (!shader_module) {
        return NULL;
    }

    return shader_module;
}



WGPUComputePipeline init_compute_pipeline(WGPUDevice device, WGPUShaderModule shader_module, std::string_view entry_point,
    std::string_view pipeline_label) {
    WGPUComputePipelineDescriptor pipeline_desc = {
        .nextInChain = NULL,
        .label = to_wgpu_string(pipeline_label),
        .layout = NULL,
        .compute = {
            .module = shader_module,
            .entryPoint = to_wgpu_string(entry_point),
        }
    };

    WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(device, &pipeline_desc);

    if (!pipeline) {
        return NULL;
    }

    return pipeline;
}

WGPUCommandEncoder init_command_encoder(WGPUDevice device) {
    WGPUCommandEncoderDescriptor encoder_desc = {
        .nextInChain = NULL,
        .label = "Compute Encoder"
    };

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoder_desc);

    if (!encoder) {
        return NULL;
    }

    return encoder;
}

WGPUComputePassEncoder init_compute_pass_encoder(WGPUCommandEncoder encoder) {
    WGPUComputePassDescriptor pass_desc = {
        .nextInChain = NULL,
        .label = "Compute Pass"
    };

    WGPUComputePassEncoder pass_encoder = wgpuCommandEncoderBeginComputePass(encoder, &pass_desc);

    if (!pass_encoder) {
        return NULL;
    }

    return pass_encoder;
}




WGPUBuffer init_uniform_buffer(WGPUDevice device, uint64_t size, std::string buffer_label) {
    WGPUBufferDescriptor buffer_desc = {
        .nextInChain = NULL,
        .label = buffer_label.c_str(),
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = size,
        .mappedAtCreation = true
    };

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);

    if (!buffer) {
        return NULL;
    }

    return buffer;
}

WGPUBuffer init_storage_buffer(WGPUDevice device, uint64_t size, std::string buffer_label) {
    WGPUBufferDescriptor buffer_desc = {
        .nextInChain = NULL,
        .label = buffer_label.c_str(),
        .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
        .size = size,
        .mappedAtCreation = true
    };

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);

    if (!buffer) {
        return NULL;
    }

    return buffer;
}

WGPUBindGroupEntry init_bind_group_entry(uint32_t binding, WGPUBuffer buffer, uint64_t offset, uint64_t size) {
    WGPUBindGroupEntry entry = {
        .nextInChain = NULL,
        .binding = binding,
        .buffer = buffer,
        .offset = offset,
        .size = size,
    };

    return entry;
}

WGPUBindGroupEntry init_bind_group_entry_for_texture(uint32_t binding, WGPUTextureView texture_view, uint64_t offset) {
    WGPUBindGroupEntry entry = {
        .nextInChain = NULL,
        .binding = binding,
        .offset = offset,
        .textureView = texture_view
    };

    return entry;
}

WGPUBindGroupLayout init_bind_group_layout(WGPUComputePipeline pipeline, std::string bind_group_layout_label) {
    WGPUBindGroupLayout bind_group_layout = wgpuComputePipelineGetBindGroupLayout(pipeline, 0);

    if (!bind_group_layout) {
        return NULL;
    }

    return bind_group_layout;

}


WGPUBindGroup init_bind_group(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUBindGroupEntry* entries, size_t entry_count, std::string bind_group_label) {
    WGPUBindGroupDescriptor bind_group_desc = {
        .nextInChain = NULL,
        .label = bind_group_label.c_str(),
        .layout = bind_group_layout,
        .entryCount = entry_count,
        .entries = entries
    };

    WGPUBindGroup bind_group = wgpuDeviceCreateBindGroup(device, &bind_group_desc);

    if (!bind_group) {
        return NULL;
    }

    return bind_group;
}

WGPUBuffer init_staging_buffer(WGPUDevice device, uint64_t size, std::string buffer_label) {
    WGPUBufferDescriptor buffer_desc = {
        .nextInChain = NULL,
        .label = buffer_label.c_str(),
        .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
        .size = size,
        .mappedAtCreation = false
    };

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);

    if (!buffer) {
        return NULL;
    }

    return buffer;
}


WGPUTexture init_texture(WGPUDevice device, uint32_t width, uint32_t height, std::string texture_label) {
    WGPUTextureDescriptor texture_desc = {
        .nextInChain = NULL,
        .label = texture_label.c_str(),
        .usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc | WGPUTextureUsage_TextureBinding |
            WGPUTextureUsage_StorageBinding,
        .dimension = WGPUTextureDimension_2D,
        .size = {width, height, 1},
        .format = WGPUTextureFormat_RGBA8Unorm,
        .mipLevelCount = 1,
        .sampleCount = 1
    };


    WGPUTexture texture = wgpuDeviceCreateTexture(device, &texture_desc);

    if (!texture) {
        return NULL;
    }

    return texture;
}


WGPUTextureView init_texture_view(WGPUTexture texture, std::string texture_view_label){
    WGPUTextureViewDescriptor texture_view_desc = {
        .nextInChain = NULL,
        .label = texture_view_label.c_str(),
        .format = WGPUTextureFormat_RGBA8Unorm,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1
    };

    WGPUTextureView texture_view = wgpuTextureCreateView(texture, &texture_view_desc);

    if (!texture_view) {
        return NULL;
    }

    return texture_view;
}