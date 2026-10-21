struct ImageDetails {
    width: u32,
    height: u32,
};

struct Color {
    r: u32,
    g: u32,
    b: u32,
};

@group(0) @binding(0) var<uniform> image_details: ImageDetails;
@group(0) @binding(1) var<uniform> replace_color: Color;
@group(0) @binding(2) var<uniform> target_color: Color;
@group(0) @binding(3) var<storage, read_write> image: array<Color>;

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    // Exit if the thread is out of bounds of the image
    if (id.x >= image_details.width || id.y >= image_details.height) {
        return;
    }

    let index = (id.y * image_details.width) + id.x;

    Color pixel = image[index];

    // TODO - Does this need a tolerance for color matching?
    // Check if the pixel color matches the target color
    if (pixel.r == target_color.r && pixel.g == target_color.g && pixel.b == target_color.b) {
        // Replace the pixel color with the new color
        image[index] = replace_color;
    }
}