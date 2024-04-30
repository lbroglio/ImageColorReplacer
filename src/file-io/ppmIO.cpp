#include"./include/ppmIO.hpp"

/**
 * @brief Gets the next entry it can find in the given file stream.
 * Moves past white space until it finds the entry
 * 
 * @param imgFile Pointer to the file stream to get an entry from
 * @return The entry as a string
 */
std::string readEntryASCII(std::ifstream* imgFile){
    //Initialize needed varables
    std::string entry = "";
    char curr = 'x';
    
    //While the next character is not whitespace or the entry isn't empty
    while((imgFile->peek() != ' ' && imgFile->peek() != '\n' && imgFile->peek() != EOF) || (entry == ""  && imgFile->peek() != EOF)){
        //Get the next character in the file
        curr = imgFile->get();

        //If a comment is encountered bypass it 
        if(curr == '#'){
            if(entry == ""){
                while(curr != '\n'){
                    curr = imgFile->get();
                }
            }
        }

        //Ad the current character if it isn't white space
        if(curr != ' ' &&  curr != '\n'){
            entry += curr;
        }
        
    }

    return entry;
}

PPMImage::PPMImage(int length, int width): length(length), width(width){
    // Allocate array for image data
    imageData = new Pixel*[length];
    for(int i =0; i < width; i++){
        imageData[i] = new Pixel;
    }
}

PPMImage::~PPMImage(){
    // Free the array storing this pixels data
    for(int i =0; i < width; i++){
        delete imageData[i];
    }

    delete[] imageData;
}

/**
 * @brief Reads the three color values that make up a pixel from the given file. 
 * Returns a pixel object representing the parsed data
 * Reads in Binary color values
 * 
 * @param imgFile Pointer to the file stream to read from
 * 
 * @return Pixel object created from the parsed data
 */
Pixel readPixel(std::ifstream* imgFile){
    Pixel toReturn;

    toReturn.r = imgFile->get();
    toReturn.g = imgFile->get();
    toReturn.b = imgFile->get();;

    return toReturn;
}

PPMImage readInPPM(std::string filePath){
    //Load the file
    std::ifstream imgFile(filePath);

    //Check if the provided file exists throw exception if it doesn't
    if(!imgFile){
        throw std::invalid_argument("Provided file does not exist");
    }
    
    //Get the entry containting the magic number (Will be in P# format)
    std::string magicNumberStr = readEntryASCII(&imgFile);
    
    //Get magic number from its string
    int magicNumber = magicNumberStr[1] - '0';

    //Get the dimesions and color depth
    int length = atoi(readEntryASCII(&imgFile).c_str());
    int width = atoi(readEntryASCII(&imgFile).c_str());
    int maxColorVal = atoi(readEntryASCII(&imgFile).c_str());

    // Create an object to store the read in file
    PPMImage readInImg(length, width);

    //Save the current postion in the file
    std::streampos filePos = imgFile.tellg();

    //Close the file
    imgFile.close();

    //Reopen to read binary
    imgFile.open(filePath, std::ios::binary);

    //Return to saved position
    imgFile.seekg(filePos);

    //Move past the seperator between the header and body
    imgFile.get();

    //Read in all of the pixels 
    for(int i =0; i < length; i++){
        for(int j =0; j < width; j++){
            Pixel readIn = readPixel(&imgFile);

            // If this images color depth isn't 255 normalize it 
            if(maxColorVal != 255){
                readIn.r *= (255 / maxColorVal);
                readIn.g *= (255 / maxColorVal);
                readIn.b *= (255 / maxColorVal);
            }

            readInImg.imageData[i][j] = readIn;
        }
    }

    //Close the file
    imgFile.close();

    // Return object
    return readInImg;
}

void PPMOut(std::ofstream* file, Pixel** outputFrom, int imgLength, int imgWidth){
    //Output the header
    *file << "P6\n";
    *file << imgLength  << ' ' << imgWidth << '\n';
    *file << "255\n";

    //Output the data
    for(int i = 0; i < imgWidth; i++){
        for(int j = 0; j < imgLength; j++){
            file->put(outputFrom[i][j].r);
            file->put(outputFrom[i][j].g);
            file->put(outputFrom[i][j].b);
        }
    }

}