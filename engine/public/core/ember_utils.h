#pragma once
#include "imports.h"

/*!
    @brief Create a formatted string

    @return String
    @version 0.0.8
*/
std::string text_format(const char* fmt, ...);

int get_memory_usage();


int get_available_memory();

int get_cpu_cores_num();
