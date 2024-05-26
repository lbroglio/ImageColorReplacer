#include<cstring>
#include <GL/glut.h>
#include <GLES3/gl31.h>
#include <GL/glext.h>

#include "./shaders.hpp"
#include "../file-io/ppmIO.hpp"



// Globals 

// Log for openGL errors
const int LOG_LENGTH = 1000;
GLchar glLog[LOG_LENGTH];

/**
 * @brief For each Pixel in the image calculate if it should be changed and reset it using the GPU
 * 
 * @param replaceIn The PPMImage to operate on
 * @param colorToChange The color that should be changed
 * @param newColor The color to replace colorToChangeWith
 * @param distForChange If the difference (found with 3D distance formula) of a color is less than this it will be replaced
 * @return true The operation was successful
 * @return false The operation failed for some reason (related to the GPU)
 */
bool replaceColor(PPMImage* replaceIn, Pixel colorToChange, Pixel newColor, float distForChange){
    // Setup input and output buffers for GPU
    GLfloat* inVBO = new GLfloat[4 * (replaceIn->length * replaceIn->width)];
    GLfloat* outVBO = new GLfloat[4 * (replaceIn->length * replaceIn->width)];

    // Load input buffer
    for(int j =0; j < replaceIn->width; j++){
        for(int i=0; i < replaceIn->length; i++){
            int offset = ((j * replaceIn->width) + i) * 4;
            GLfloat tst =  static_cast< GLfloat >(replaceIn->imageData[i][j].r);
            inVBO[offset] = tst;
            inVBO[offset + 1] = static_cast< GLfloat >(replaceIn->imageData[i][j].g);
            inVBO[offset + 2] = static_cast< GLfloat >(replaceIn->imageData[i][j].b);
            inVBO[offset + 3] = 1;

            // Fill out VBO with dummy data
            inVBO[offset] = 0.0;
            inVBO[offset + 1] = 0.0;
            inVBO[offset + 2] = 0.0;
            inVBO[offset + 3] = 0.0;
        }
    }
 

    // Create the compute program, to which the compute shader will be assigned
    GLuint gComputeProgram = glCreateProgram();

    // Compile shader code 
    GLuint mComputeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(mComputeShader, 1, &COMPUTE_SHADER_SRC, NULL);
    glCompileShader(mComputeShader);

    // Output any errors found when compiling
    int rvalue;
    glGetShaderiv(mComputeShader, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
        GLint outLength = 0;
        glGetShaderInfoLog(mComputeShader, LOG_LENGTH, &outLength, glLog);
        printf("Error: Compiler log:\n%s\n", glLog);
        return false;
    }

    // Attach/Link shader to program
    glAttachShader(gComputeProgram, mComputeShader);
    glLinkProgram(gComputeProgram);

    // Check if there were some issues when linking the shader.
    glGetProgramiv(gComputeProgram, GL_LINK_STATUS, &rvalue);
    if (!rvalue)
    {
        GLint outLength = 0;
        glGetProgramInfoLog(gComputeProgram, LOG_LENGTH, &outLength, glLog);
        printf("Error: Linker log:\n%s\n", glLog);
        return false;
    }


    // Bind the compute program for reading uniform locations
    glUseProgram(gComputeProgram);

    // Retrieve the location ofr the uniforms
    GLint toChangeLoc = glGetUniformLocation(gComputeProgram, "colorToChange");
    GLint newColorLoc = glGetUniformLocation(gComputeProgram, "newColor");
    GLint distLoc = glGetUniformLocation(gComputeProgram, "changeDist");
    GLint arrSizeLoc = glGetUniformLocation(gComputeProgram, "arraySize");

    // Binding of the buffer which goes into the GPU
    GLint indexInBufferBinding = 1;   

    // Binding of the buffer which goes out the GPU
    GLint indexOutBufferBinding = 0;   

    // Float used for setting vec3 uniforms
    
    // Set uniforms
    glUniform1f(distLoc, distForChange);

    const GLfloat colorToChangeVec[3] = {static_cast< float >(colorToChange.r), static_cast< float >(colorToChange.g), static_cast< float >(colorToChange.b)};
    glUniform3fv(toChangeLoc, 3, colorToChangeVec);

    const GLfloat newColorVec[3] = {static_cast< float >(newColor.r), static_cast< float >(newColor.g), static_cast< float >(newColor.b)};
    glUniform3fv(toChangeLoc, 3, newColorVec);

    const GLfloat arraySizeVec[2] = {static_cast< float >(replaceIn->length), static_cast< float >(replaceIn->width)};
    glUniform2fv(arrSizeLoc, 2, arraySizeVec);

    // Create buffers
    GLuint bufferNames[2]; 
    glGenBuffers(2, bufferNames);

    // Bind buffers

    // Bind input buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferNames[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexInBufferBinding, bufferNames[0]);
    // Load data
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(inVBO), inVBO, GL_DYNAMIC_READ);

    //Bind output buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferNames[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexOutBufferBinding, bufferNames[1]);
    // Fill Buffer
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outVBO), outVBO, GL_DYNAMIC_READ);

    // Dispatch compute jobs
    glDispatchCompute(replaceIn->length, replaceIn->width, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // Rebuild image from output data;
    void* tmpPointer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outVBO), GL_MAP_READ_BIT);
    float* outData = static_cast<float*>(tmpPointer);
    float tmp = 0.0;
    for(int j =0; j < replaceIn->width; j++){
        for(int i=0; i < replaceIn->length; i++){
            // Calculate offset
            int offset = ((j * replaceIn->width) + i) * 4;

            // Copy data to a temp and set this pixels values
            memcpy(&tmp, outData + offset, sizeof(float));
            replaceIn->imageData[i][j].r = static_cast<int>(tmp);

            memcpy(&tmp, outData + offset + 1, sizeof(float));
            replaceIn->imageData[i][j].g = static_cast<int>(tmp);

            memcpy(&tmp, outData + offset + 2, sizeof(float));
            replaceIn->imageData[i][j].b = static_cast<int>(tmp);
            
        }
    }

    // Clean buffers by unmapping, unbinding, and deleteing
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexInBufferBinding, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexOutBufferBinding, 0);

    glDeleteBuffers(2, bufferNames);

    //Clean up memory
    delete[] inVBO;
    delete[] outVBO;

    return true;
}

int main(int argc, char* argv[]){
    // TODO -- Add usage with -h

    // TODO -- Add error handling for args
    // Parse arguments
    // TODO -- Add python script for taking in and outputting pngs by converting ppms
    std::string filePath = argv[1];
    std::string changeColorStr = argv[2];
    std::string newColorStr = argv[3];

    // TODO - Add distance arg

    // Parse color args 
    const char* changeColorNums[3];
    const char* newColorNums[3];

    int firstComma = changeColorStr.find_first_of(',');
    int secondComma =  changeColorStr.find_last_of(',');
    changeColorNums[0] = changeColorStr.substr(0, firstComma).c_str();
    changeColorNums[1] = changeColorStr.substr(firstComma + 1, secondComma - (firstComma + 1)).c_str();
    changeColorNums[2] = changeColorStr.substr(secondComma + 1).c_str();

    // TODO Remove after dev is done
    std::cout << "Color to change: " << changeColorNums[0] << ", " << changeColorNums[1] << ", " << changeColorNums[2] << std::endl;

    firstComma = newColorStr.find_first_of(',');
    secondComma =  newColorStr.find_last_of(',');
    newColorNums[0] = newColorStr.substr(0, firstComma).c_str();
    newColorNums[1] = newColorStr.substr(firstComma + 1, secondComma - (firstComma + 1)).c_str();
    newColorNums[2] = newColorStr.substr(secondComma + 1).c_str();

    // TODO Remove after dev is done
    std::cout << "New Color " << newColorNums[0] << ", " << newColorNums[1] << ", " << newColorNums[2] << std::endl;


    // Initialize those colors as pixel objects
    Pixel changeColor;
    changeColor.r = atoi(changeColorNums[0]);
    changeColor.g = atoi(changeColorNums[1]);
    changeColor.b = atoi(changeColorNums[2]);

    Pixel newColor;
    newColor.r = atoi(newColorNums[0]);
    newColor.g = atoi(newColorNums[1]);
    newColor.b = atoi(newColorNums[2]);

    // Read in image
    PPMImage toProc = readInPPM(filePath);

    // Change any pixels within the the given dist of change color to new color using the GPU
    // TODO -- Alert User on error
    // TODO -- Don't hard code the distance
    replaceColor(&toProc, changeColor, newColor, 1);

    // Output to a new PPM
    std::ofstream outFile("swapppepColors.ppm");
    PPMOut(&outFile, toProc.imageData, toProc.length, toProc.width);

    outFile.close();

}