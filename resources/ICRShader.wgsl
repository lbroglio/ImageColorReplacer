struct ImageDetails {
    width: u32,
    height: u32,
};


@group(0) @binding(0) var<uniform> image_details: ImageDetails;
@group(0) @binding(1) var<uniform> target_color: vec4f;
@group(0) @binding(2) var<uniform> new_color: vec4f;
@group(0) @binding(3) var input_image: texture_storage_2d<rgba8unorm, read>;
@group(0) @binding(4) var new_image: texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(5) var<uniform> distance_threshold: u32;

fn color_distance(c1: vec4f, c2: vec4f) -> u32 {
    let weighted_r_dist = pow(f32(c1.r - c2.r), 2) * 0.299;
    let weighted_g_dist = pow(f32(c1.g - c2.g), 2) * 0.587;
    let weighted_b_dist = pow(f32(c1.b - c2.b), 2) * 0.114;
    let distance = sqrt(weighted_r_dist + weighted_g_dist + weighted_b_dist);

    return u32(distance);
}

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    // Exit if the thread is out of bounds of the image
    if (id.x >= image_details.width || id.y >= image_details.height) {
        return;
    }

    let coords = vec2i(i32(id.x), i32(id.y));
    let pixelColor: vec4f = textureLoad(input_image, coords);

    let index = (id.y * image_details.width) + id.x;

    // Check if the pixel color matches the target color
    if (color_distance(pixelColor, target_color) < distance_threshold) {
        // Replace the pixel color with the new color
        textureStore(new_image, coords, new_color);
    }
}