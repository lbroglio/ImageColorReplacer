struct ImageDetails {
    width: u32,
    height: u32,
};

struct Color {
    r: u32,
    g: u32,
    b: u32,
    padding: u32, // Padding to align to 16 bytes``
};

@group(0) @binding(0) var<uniform> image_details: ImageDetails;
@group(0) @binding(1) var<uniform> target_color: Color;
@group(0) @binding(2) var<uniform> new_color: Color;
@group(0) @binding(3) var<storage, read_write> image: array<Color>;
@group(0) @binding(4) var<uniform> distance_threshold: u32;

fn color_distance(c1: Color, c2: Color) -> u32 {
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

    let index = (id.y * image_details.width) + id.x;

    var pixel = image[index];

    // TODO - Does this need a tolerance for color matching?
    // Check if the pixel color matches the target color
    if (color_distance(pixel, target_color) < distance_threshold) {
        // Replace the pixel color with the new color
        image[index] = new_color;
    }
}