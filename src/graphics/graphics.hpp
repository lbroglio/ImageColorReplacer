#pragma once

#include <cstdint>
#include <string_view>
#include <stdfloat>

#include<wgpu.h>

#include "../file-io/pngIO.hpp"



/**
 * @brief Class which represents an individual pixel in an image
 * 
 */
struct alignas(16) Pixel{
    /**
     * @brief Red component of this pixel
     * 
     */
    std::float32_t r;
    /**
     * @brief Green component of this pixel
     * 
     */
    std::float32_t g;
    /**
     * @brief Blue component of this pixel
     * 
     */
    std::float32_t b;
    /**
     * @brief The opacity component of this pixel
     * 
     */
    std::float32_t a;
};


struct ColorReplaceCreateInfo {
    std::string_view shader_code;
    const ImageDetails& image_details;
    Pixel target_color;
    Pixel new_color;
    std::float32_t distance_threshold;

    ColorReplaceCreateInfo(std::string_view shader_code, const ImageDetails& image_details, Pixel target_color, Pixel new_color, std::float32_t distance_threshold)
        : shader_code(shader_code), image_details(image_details), target_color(target_color), new_color(new_color), distance_threshold(distance_threshold) {}
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
    WGPUTexture input_image_texture;
    WGPUTexture new_image_texture;
    WGPUTextureView input_image_texture_view;
    WGPUTextureView new_image_texture_view;
    WGPUBuffer distance_threshold_buffer;

    ColorReplacePipelineResources() : instance(nullptr), adapter(nullptr), device(nullptr), queue(nullptr), shader_module(nullptr),
                          pipeline(nullptr), encoder(nullptr), pass_encoder(nullptr), bind_group_layout(nullptr),
                          bind_group(nullptr), image_details_buffer(nullptr), target_color_buffer(nullptr),
                          new_color_buffer(nullptr), input_image_texture(nullptr), new_image_texture(nullptr), 
                          input_image_texture_view(nullptr), new_image_texture_view(nullptr), distance_threshold_buffer(nullptr) {}

    ~ColorReplacePipelineResources();

    bool create(const ColorReplaceCreateInfo& info);
};


/**
 * @brief Maps a value from the range [0, 255] to the range [0, 1]
 * 
 * @param color_value The color value in the range [0, 255]
 * @return std::float32_t The corresponding value in the range [0, 1]
 */
std::float32_t map_255_to_1(int color_value);