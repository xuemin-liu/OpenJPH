#pragma once

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include "ojph_mem.h"
#include "ojph_file.h"
#include "ojph_codestream.h"
#include "ojph_params.h"
#include "ojph_message.h"

/**
 * @class Htj2kProcessor
 * @brief Class for handling HTJ2K compression and decompression
 *
 * This class provides methods to compress raw image data into HTJ2K format
 * and decompress HTJ2K data back to raw image format.
 */
class Htj2kProcessor {
public:
    /**
     * @struct CompressionParams
     * @brief Parameters for HTJ2K compression
     */
    struct CompressionParams {
        int num_decompositions;      ///< Number of wavelet decompositions
        int block_width;             ///< Codeblock width
        int block_height;            ///< Codeblock height
        float quantization_step;     ///< Quality factor for lossy compression (lower = better quality)
        bool lossless;               ///< Whether to use lossless compression
        bool color_transform;        ///< Whether to apply color transform (for RGB)
        bool chroma_downsample;      ///< Whether to downsample chroma components
        bool is_signed;              ///< Whether sample values are signed
        bool is_planar;              ///< Whether data is organized in planar fashion
        int progression_order;       ///< Progression order (0=LRCP, 1=RLCP, 2=RPCL, 3=PCRL, 4=CPRL)
        
        // For precinct sizes if needed
        struct PrecintSize {
            ojph::ui32 width;
            ojph::ui32 height;
        };
        
        std::vector<PrecintSize> precincts; ///< Precinct sizes (if any)
        
        // Default constructor with initializers
        CompressionParams() :
            num_decompositions(5),
            block_width(64),
            block_height(64),
            quantization_step(0.001f),
            lossless(true),
            color_transform(true),
            chroma_downsample(false),
            is_signed(false),
            is_planar(false),
            progression_order(2) // RPCL
        {
        }
    };

    /**
     * @brief Default constructor
     */
    Htj2kProcessor() = default;

    /**
     * @brief Compress raw image data to HTJ2K codestream
     * 
     * @param image_data Pointer to raw image data
     * @param width Image width
     * @param height Image height
     * @param components Number of image components
     * @param bits_per_sample Bits per sample (e.g., 8, 10, 12, 16)
     * @param params Additional compression parameters
     * @return Compressed data as vector of bytes
     */
    std::vector<uint8_t> compress(const uint8_t* image_data, int width, int height, 
                                 int components, int bits_per_sample, 
                                 const CompressionParams& params = CompressionParams());

    /**
     * @brief Decompress HTJ2K codestream to raw image data
     * 
     * @param compressed_data Pointer to compressed HTJ2K data
     * @param compressed_size Size of compressed data in bytes
     * @param width Output parameter to receive image width
     * @param height Output parameter to receive image height
     * @param components Output parameter to receive number of components
     * @param bits_per_sample Output parameter to receive bits per sample
     * @param resilient Whether to enable resilient decoding mode
     * @param reduce_level Resolution reduction level (0 = full resolution)
     * @return Decompressed data as vector of bytes
     */
    std::vector<uint8_t> decompress(const uint8_t* compressed_data, size_t compressed_size,
                                   int& width, int& height, int& components, int& bits_per_sample,
                                   bool resilient = false, int reduce_level = 0);
    
    /**
     * @brief Compress raw image data to HTJ2K file
     * 
     * @param image_data Pointer to raw image data
     * @param width Image width
     * @param height Image height
     * @param components Number of image components
     * @param bits_per_sample Bits per sample (e.g., 8, 10, 12, 16)
     * @param output_filename Path to output HTJ2K file
     * @param params Additional compression parameters
     * @return True if successful, false otherwise
     */
    bool compressToFile(const uint8_t* image_data, int width, int height, 
                       int components, int bits_per_sample, const std::string& output_filename,
                       const CompressionParams& params = CompressionParams());
    
    /**
     * @brief Decompress HTJ2K file to raw image data
     * 
     * @param input_filename Path to HTJ2K input file
     * @param width Output parameter to receive image width
     * @param height Output parameter to receive image height
     * @param components Output parameter to receive number of components
     * @param bits_per_sample Output parameter to receive bits per sample
     * @param resilient Whether to enable resilient decoding mode
     * @param reduce_level Resolution reduction level (0 = full resolution)
     * @return Decompressed data as vector of bytes
     */
    std::vector<uint8_t> decompressFromFile(const std::string& input_filename,
                                          int& width, int& height, int& components, int& bits_per_sample,
                                          bool resilient = false, int reduce_level = 0);
    
private:
    /**
     * @brief Fill a line buffer with image data
     * 
     * @param line_buffer The line buffer to fill
     * @param image_data Source image data
     * @param width Image width
     * @param height Image height
     * @param components Number of components
     * @param component Current component index
     * @param y Current line index
     * @param bits_per_sample Bits per sample
     */
    void fill_line_buffer(ojph::line_buf* line_buffer, const uint8_t* image_data,
                         int width, int height, int components, int component, int y,
                         int bits_per_sample);
};
