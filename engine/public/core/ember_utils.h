#pragma once
#include "imports.h"

/*!
    @brief Create a formatted string

    @return Formatted String

    @version 0.0.8
*/
std::string text_format(const char* fmt, ...);

/*!
    @brief Get the current memory usage in MB

    @note  Not implemented yet

    @version 1.2.0

    @return  Memory usage in MB
*/
int get_memory_usage();

/*!
    @brief Get the available system memory in MB

    @return Available memory in MB

    @version 0.0.8
*/
int get_available_memory();


/*!
    @brief Get the number of CPU cores ( LOGICAL )


    @return Number of CPU cores

    @version 1.2.0
*/
int get_cpu_cores_num();
