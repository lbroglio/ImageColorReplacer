#include<iostream>
#include<fstream>



/**
 * @brief Class which represents an individual pixel in an image
 * 
 */
class Pixel{
    public:
        /**
         * @brief Red component of this pixel
         * 
         */
        int r;
        /**
         * @brief Green component of this pixel
         * 
         */
        int g;
        /**
         * @brief Blue component of this pixel
         * 
         */
        int b;
};


/**
 * @brief Stores information from a PPM file read in by the program.
 * 
 */
class PPMImage{
    public:
        /**
         * @brief Store the data that makes up the image as a 2D array of pixels
         */
        Pixel** imageData;
        /**
         * @brief The number of pixels long this image is
         * 
         */
        int length;
        /**
         * @brief The number of pixels wide this image is
         * 
         */
        int width;
        /**
         * @brief Construct a new PPMImage object with the given length and width
         * 
         * @param length The number of pixels long this image should be
         * @param width The number of pixels wide this image should be 
         */
        PPMImage(int length, int width);
        /**
         * @brief Destroy the PPMImage object and free its array.
         * 
         */
        ~PPMImage();
};

/**
 * @brief Read in a PPm image from a file
 * 
 * @param filePath Path to the file to read the image from
 * @return An object holding the read in data
 */
PPMImage readInPPM(std::string filePath);


/**
 * @brief Output an Array of Pixels to a P6 PPM image
 * 
 * @param file Pointer to the filestream to print the image data to
 * @param outputFrom The array of Pixel objects to use as the image
 * @param imgWidth The width of the image to output in pixels
 * @param imgHeight The height of the image to output in pixels
 * @return The fiel stream being printed to after the image has been written
 */
void PPMOut(std::ofstream* file, Pixel** outputFrom, int imgLength, int imgWidth);


