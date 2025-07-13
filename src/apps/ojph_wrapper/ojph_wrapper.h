//***************************************************************************/
// This file is part of the OpenJPH software implementation.
// File: ojph_wrapper.h
// Author: OpenJPH Project
// Date: 2025
//***************************************************************************/

#ifndef OJPH_WRAPPER_H
#define OJPH_WRAPPER_H

#ifdef _WIN32
    #ifdef OJPH_WRAPPER_EXPORTS
        #define OJPH_WRAPPER_API __declspec(dllexport)
    #else
        #define OJPH_WRAPPER_API __declspec(dllimport)
    #endif
#else
    #define OJPH_WRAPPER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Version information
OJPH_WRAPPER_API const char* ojph_wrapper_get_version(void);

// HTJ2K C interface functions
OJPH_WRAPPER_API int htj2k_compress(
    const unsigned char* image_data, int width, int height,
    int components, int bits_per_sample,
    int num_decompositions, int block_width, int block_height, float quantization_step,
    unsigned char** compressed_data, int* compressed_size
);

OJPH_WRAPPER_API int htj2k_decompress(
    const unsigned char* compressed_data, size_t compressed_size,
    unsigned char* decompressed_data, size_t* decompressed_size,
    int* width, int* height, int* components, int* bits_per_sample,
    int resilient, int reduce_level
);

// Error handling
OJPH_WRAPPER_API const char* ojph_wrapper_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif // OJPH_WRAPPER_H
