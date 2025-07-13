//***************************************************************************/
// This file is part of the OpenJPH software implementation.
// File: ojph_wrapper.cpp
// Author: OpenJPH Project  
// Date: 2025
//***************************************************************************/

#include "ojph_wrapper.h"
#include "Htj2kProcessor.h"

#include <string>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstring>

static std::string last_error;

extern "C" {

OJPH_WRAPPER_API const char* ojph_wrapper_get_version(void)
{
    return "OpenJPH Wrapper v1.0 - Built with static runtime";
}

OJPH_WRAPPER_API const char* ojph_wrapper_get_last_error(void)
{
    return last_error.c_str();
}

OJPH_WRAPPER_API int htj2k_compress(
    const unsigned char* image_data, int width, int height,
    int components, int bits_per_sample,
    int num_decompositions, int block_width, int block_height, float quantization_step,
    unsigned char** compressed_data, int* compressed_size)
{
    try {
        last_error.clear();
        
        if (!image_data || !compressed_data || !compressed_size) {
            last_error = "Invalid parameters: null pointers";
            return -1;
        }
        
        if (width <= 0 || height <= 0 || components <= 0 || bits_per_sample <= 0) {
            last_error = "Invalid image dimensions or parameters";
            return -1;
        }
        
        // Set up compression parameters
        Htj2kProcessor::CompressionParams params;
        params.num_decompositions = num_decompositions;
        params.block_width = block_width;
        params.block_height = block_height;
        params.quantization_step = quantization_step;
        
        // Call the C++ compress method
        std::vector<uint8_t> result = Htj2kProcessor::compress(
            image_data, width, height, components, bits_per_sample, params);
        
        // Allocate memory for the result and copy data
        *compressed_size = static_cast<int>(result.size());
        *compressed_data = static_cast<unsigned char*>(malloc(*compressed_size));
        
        if (!*compressed_data) {
            last_error = "Failed to allocate memory for compressed data";
            return -1;
        }
        
        memcpy(*compressed_data, result.data(), *compressed_size);
        return 0; // Success
        
    } catch (const std::exception& e) {
        last_error = std::string("Exception in htj2k_compress: ") + e.what();
        return -1;
    } catch (...) {
        last_error = "Unknown exception in htj2k_compress";
        return -1;
    }
}

OJPH_WRAPPER_API int htj2k_decompress(
    const unsigned char* compressed_data, int compressed_size,
    int* width, int* height, int* components, int* bits_per_sample,
    int resilient, int reduce_level,
    unsigned char** image_data)
{
    try {
        last_error.clear();
        
        if (!compressed_data || !width || !height || !components || !bits_per_sample || !image_data) {
            last_error = "Invalid parameters: null pointers";
            return -1;
        }
        
        if (compressed_size <= 0) {
            last_error = "Invalid compressed data size";
            return -1;
        }
        
        // Call the C++ decompress method
        std::vector<uint8_t> result = Htj2kProcessor::decompress(
            compressed_data, static_cast<size_t>(compressed_size),
            *width, *height, *components, *bits_per_sample,
            resilient != 0, reduce_level);
        
        // Allocate memory for the result and copy data
        size_t image_size = result.size();
        *image_data = static_cast<unsigned char*>(malloc(image_size));
        
        if (!*image_data) {
            last_error = "Failed to allocate memory for image data";
            return -1;
        }
        
        memcpy(*image_data, result.data(), image_size);
        return 0; // Success
        
    } catch (const std::exception& e) {
        last_error = std::string("Exception in htj2k_decompress: ") + e.what();
        return -1;
    } catch (...) {
        last_error = "Unknown exception in htj2k_decompress";
        return -1;
    }
}

} // extern "C"
