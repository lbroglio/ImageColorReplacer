#include <iostream>
#include "errors.hpp"


void fatal_error(const std::string& message)
{

    std::cerr << RED << "Error: " << message << RESET << std::endl;
    std::exit(EXIT_FAILURE);
}