#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>


#include "input.hpp"
#include "../graphics/graphics.hpp"


Pixel parse_color_string(const std::string& color_str){
    // Implementation for parsing color string

    size_t first_comma = color_str.find_first_of(',');
    size_t second_comma = color_str.find_last_of(',');

    if(first_comma == std::string::npos || second_comma == std::string::npos || first_comma == second_comma) {
        throw std::invalid_argument("Invalid color string format. Expected format: R,G,B");
    }


    std::string colorNums[3];
    colorNums[0] = color_str.substr(0, first_comma);
    colorNums[1] = color_str.substr(first_comma + 1, second_comma - (first_comma + 1));
    colorNums[2] = color_str.substr(second_comma + 1);

    // Initialize those colors as pixel objects
    Pixel changeColor;
    changeColor.r = map_255_to_1(atoi(colorNums[0].c_str()));
    changeColor.g = map_255_to_1(atoi(colorNums[1].c_str()));
    changeColor.b = map_255_to_1(atoi(colorNums[2].c_str()));
    changeColor.a = 1.0f; // Set alpha to fully opaque

    return changeColor;
}

std::string extract_argument_value(std::vector<std::string>* argv, const std::string&  argument_name) {
    
    if (!std::ranges::contains(*argv, argument_name)) {
       return "";
    }
    
    size_t arg_pos = std::find(argv->begin() + 1, argv->end(), argument_name) - argv->begin();

    if (arg_pos == argv->size()) {
        throw std::invalid_argument("Argument not found: " + argument_name);
    }

    if (arg_pos + 1 >= argv->size()) {
        throw std::invalid_argument("No value provided for argument: " + argument_name);
    }

    std::string value = argv->at(arg_pos + 1);
    // Remove the argument and its value from the vector
    argv->erase(argv->begin() + arg_pos, argv->begin() + arg_pos + 2);

    return value;
}