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


/*!
    @brief Get MIME type from file extension

    @param ext File extension

    @return MIME type string or "application/octet-stream"

    @version 1.2.0
*/
const char* file_extension_to_mime_type(const char* ext);


/*!
 * @brief Decode a UTF-8 string to a Unicode codepoint.
 * @param utf8 Input UTF-8 string.
 * @param i Current index in the string (will be updated).
 * @param codepoint Output Unicode codepoint.
 * @param c Current byte
 * @return true if an invalid byte was encountered, false otherwise.
 *
 */
bool utf8_to_unicode(const std::string& utf8, size_t& i, uint32_t& codepoint, unsigned char c);
