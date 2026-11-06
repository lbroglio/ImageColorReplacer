#pragma once

#include <cstdint>
#include <span>
#include <string_view>


#include<wgpu.h>

#include"../file-io/ppmIO.hpp"





struct ImageDetails {
    std::uint32_t length;
    std::uint32_t width;
};

struct ColorReplaceCreateInfo {
    std::string_view shader_code;
    ImageDetails image_details;
    Pixel target_color;
    Pixel new_color;
    std::uint32_t distance_threshold;
    std::span<const Pixel> image;
};

struct ColorReplacePipelineResources {
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUShaderModule shader_module;
    WGPUComputePipeline pipeline;
    WGPUCommandEncoder encoder;
    WGPUComputePassEncoder pass_encoder;
    WGPUBindGroupLayout bind_group_layout;
    WGPUBindGroup bind_group;
    WGPUBuffer image_details_buffer;
    WGPUBuffer target_color_buffer;
    WGPUBuffer new_color_buffer;
    WGPUBuffer image_buffer;
    WGPUBuffer distance_threshold_buffer;

    ColorReplacePipelineResources() : instance(nullptr), adapter(nullptr), device(nullptr), queue(nullptr), shader_module(nullptr),
                          pipeline(nullptr), encoder(nullptr), pass_encoder(nullptr), bind_group_layout(nullptr),
                          bind_group(nullptr), image_details_buffer(nullptr), target_color_buffer(nullptr),
                          new_color_buffer(nullptr), image_buffer(nullptr), distance_threshold_buffer(nullptr) {}

    ~ColorReplacePipelineResources();

    bool create(const ColorReplaceCreateInfo& info);
};


