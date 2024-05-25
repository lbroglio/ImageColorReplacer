#version 310 es

uniform vec3 colorToChange;
uniform vec3 newColor;
uniform float changeDist;

uniform vec2 arraySize;

// Input buffer
layout(std140, binding = 0) buffer srcBuffer
{

      vec4 data[];

} srcBuffer;

// Output buffer
layout(std140, binding = 1) buffer destBuffer
{

      float data[];

} outBuffer;

// Work groups
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Main function
void main()
{
    // Read current global position for this thread
    ivec2 arrPos = ivec2(gl_GlobalInvocationID.xy);

    // Calculate offset for 1D array
    uint offset = (arrPos.y * arraySize.y) + arrPos.x;

    // Get input pixel
    vec3 thisPix = srcBuffer.data[offset].xyz;


    // Calculate distance between input pixel and color to replace
    float dist = pow(thisPix.x - colorToChange.x, 2.0) + pow(thisPix.y - colorToChange.y, 2.0) + pow(thisPix.z - colorToChange.z, 2.0);
    float aDist = abs(dist);

    // The pixel to write to the output buffer
    vec3 outPix = thisPix;


    // If the dist is within the distance to replace the color 
    if(aDist <= changeDist){
        outPix = newColor;
    }

    // Write to output buffer
    atomicExchange(outBuffer.data[offset], outPix.x)
    atomicExchange(outBuffer.data[offset + 1], outPix.y)
    atomicExchange(outBuffer.data[offset + 2], outPix.z)
    atomicExchange(outBuffer.data[offset + 3], 1)

    //outBuffer.data[offset] = vec4(outPix.xyz, 1);
}