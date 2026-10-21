#include<wgpu.h>
#include<string>

#include "graphicsUtils.hpp"

static void on_adapter_request(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2) {
    // Cast the user data to the appropriate callback type.
    auto callback = static_cast<AdapterReadyCallback>(userdata1);

    if (status != WGPURequestAdapterStatus_Success) {
        callback(nullptr, userdata2);
        return;
    }

    callback(adapter, userdata2);
}

static void on_device_request(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata, void *userdata2) {
    *(WGPUDevice*)userdata = device;
}
static void on_adapter_ready(WGPUAdapter adapter, void* userdata) {
    if (adapter == nullptr) {
        // Handle failure.
        return;
    }

    // The adapter is valid here.
    // Request the device here or notify the application.
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
    WGPURequestAdapterCallbackInfo callback_info = {
        .nextInChain = nullptr,
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_adapter_request,
        .userdata1 = reinterpret_cast<void*>(callback),
        .userdata2 = userdata
    };

    wgpuInstanceRequestAdapter(
        instance,
        nullptr,
        callback_info
    );
}


WGPUDevice init_device(WGPUAdapter adapter) {
    WGPUDevice device = NULL;
    wgpuAdapterRequestDevice(adapter, NULL, (WGPURequestDeviceCallbackInfo){
        .callback = on_device_request, .userdata1 = &device
    });

    return device;
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


