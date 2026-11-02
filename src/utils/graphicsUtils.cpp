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


WGPUShaderModule init_shader_module(WGPUDevice device, const char* shader_code, std::string shader_name) {
   WGPUShaderSourceWGSL shader_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceWGSL 
        },
        .code = shader_code
    };

   WGPUShaderModuleDescriptor shader_desc = {
        .nextInChain = &shader_source.chain,
        .label = shader_name.c_str() 
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


WGPUComputePipeline init_compute_pipeline(WGPUDevice device, WGPUShaderModule shader_module, std::string entry_point,
    std::string pipeline_label) {
    WGPUComputePipelineDescriptor pipeline_desc = {
        .nextInChain = NULL,
        .label = pipeline_label.c_str(),
        .layout = NULL,
        .compute = {
            .module = shader_module,
            .entryPoint = entry_point.c_str()
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
