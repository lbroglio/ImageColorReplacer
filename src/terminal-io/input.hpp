#pragma once

#include <string>

#include "../graphics/graphics.hpp"

/**
 * @brief Parse a color string in the format "R,G,B" into a Pixel object
 * 
 * @param color_str The color string to parse
 * @return Pixel The parsed Pixel object representing the color
 */
Pixel parse_color_string(const std::string& color_str);


/**
 * @brief Extract the value of a specific argument from a string of arguments and return it as a string. 
 * If the argument is not found, an empty string is returned.
 * The original string of arguments is modified to remove the extracted argument and its value.
 * 
 * @param args The string of arguments to extract the value from. 
 * This string will be modified to remove the extracted argument and its value.
 * @param argument_name  The name of the argument to extract
 * @return std::string  The value of the extracted argument as a string. 
 * If the argument is not found, an empty string is returned.
 */
std::string extract_argument_value(std::vector<std::string>* argv, const std::string&  argument_name);

