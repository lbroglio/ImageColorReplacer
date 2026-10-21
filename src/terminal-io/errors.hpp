#pragma once

#include <iostream>

#define RED     "\033[31m"
#define RESET   "\033[0m"

/**
 * @brief Write an error message to stderr and exit the program with a failure code
 * 
 * @param message The error message to write to stderr
 */
void fatal_error(const std::string& message);
