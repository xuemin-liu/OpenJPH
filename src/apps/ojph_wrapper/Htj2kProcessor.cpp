#include "Htj2kProcessor.h"

#include "ojph_mem.h"
#include "ojph_file.h"
#include "ojph_codestream.h"
#include "ojph_params.h"
#include "ojph_message.h"

#include <memory>
#include <stdexcept>

namespace {
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
    void fill_line_buffer(ojph::line_buf* line_buffer,
        const uint8_t* image_data,
        int width, int height,
        int components, int component, int y,
        int bits_per_sample)
    {
        // Fill the line buffer with data for the given component and line
        ojph::si32* dest = line_buffer->i32;

        if (bits_per_sample <= 8) {
            // 8-bit data
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * components + component;
                dest[x] = static_cast<ojph::si32>(image_data[idx]);
            }
        }
        else {
            // 16-bit data
            const uint16_t* src = reinterpret_cast<const uint16_t*>(image_data);
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * components + component;
                dest[x] = static_cast<ojph::si32>(src[idx]);
            }
        }
    }

    /**
     * @brief Common internal compression method used by both compress and compressToFile
     *
     * @tparam OutFile Type of output file (ojph::mem_outfile or ojph::j2c_outfile)
     * @param image_data Pointer to raw image data
     * @param width Image width
     * @param height Image height
     * @param components Number of components
     * @param bits_per_sample Bits per sample
     * @param params Compression parameters
     * @param outfile Output file object
     * @return true if compression succeeded
     * @return false if compression failed
     */
    template<typename OutFile>
    bool compress_internal(
        const uint8_t* image_data, int width, int height,
        int components, int bits_per_sample,
        const Htj2kProcessor::CompressionParams& params,
        OutFile& outfile)
    {
        // Initialize codestream
        ojph::codestream codestream;

        // Set image parameters
        ojph::param_siz siz = codestream.access_siz();
        siz.set_image_extent(ojph::point(width, height));
        siz.set_num_components(components);

        // Set component parameters
        for (int c = 0; c < components; ++c) {
            ojph::point downsampling(1, 1);  // Default: no downsampling
            if (c > 0 && params.chroma_downsample) {
                downsampling = ojph::point(2, 2);  // YUV 4:2:0 downsampling for chroma
            }
            siz.set_component(c, downsampling, bits_per_sample, params.is_signed);
        }
        siz.set_tile_size(ojph::size(width, height));  // No tiling
        siz.set_image_offset(ojph::point(0, 0));  // No offset
        siz.set_tile_offset(ojph::point(0, 0));  // No tile offset

        // Set coding parameters
        ojph::param_cod cod = codestream.access_cod();
        cod.set_num_decomposition(params.num_decompositions);
        cod.set_block_dims(params.block_width, params.block_height);

        // Convert int progression_order to string as required by ojph API
        const char* progression_orders[] = { "LRCP", "RLCP", "RPCL", "PCRL", "CPRL" };
        if (params.progression_order < 0 || params.progression_order > 4) {
            throw std::runtime_error("Invalid progression order");
        }
        cod.set_progression_order(progression_orders[params.progression_order]);

        cod.set_color_transform(params.color_transform && components >= 3);
        cod.set_reversible(params.lossless);

        // Set quantization if lossy compression
        if (!params.lossless) {
            codestream.access_qcd().set_irrev_quant(params.quantization_step);
        }

        // Set up precincts if specified
        //if (params.precincts.size() > 0) {
        //    // Convert our precinct size format to ojph::size format
        //    std::vector<ojph::size> ojph_precincts;
        //    for (const auto& prec : params.precincts) {
        //        ojph_precincts.push_back(ojph::size{ prec.width, prec.height });
        //    }
        //    cod.set_precinct_size(ojph_precincts.size(), ojph_precincts.data());
        //}

        // Prepare for interleaved or planar data
        codestream.set_planar(params.is_planar);

        // Write headers
        codestream.write_headers(&outfile);

        // Keep track of how many lines we've processed for each component
        std::vector<int> comp_lines(components, 0);

        // Encode using the exchange mechanism
        ojph::ui32 next_component;
        ojph::line_buf* line_buffer = nullptr;

        // First call gets the initial buffer and component to fill
        line_buffer = codestream.exchange(line_buffer, next_component);

        while (line_buffer != nullptr) {
            int comp = next_component;
            int y = comp_lines[comp]++;

            // Fill the buffer with data
            fill_line_buffer(line_buffer, image_data, width, height,
                components, comp, y, bits_per_sample);

            // Exchange buffer for next one
            line_buffer = codestream.exchange(line_buffer, next_component);
        }

        // Flush and close codestream
        codestream.flush();
        codestream.close();

        return true;
    }

    /**
     * @brief Common internal decompression method used by both decompress and decompressFromFile
     *
     * @tparam InFile Type of input file (ojph::mem_infile or ojph::j2c_infile)
     * @param infile Input file object
     * @param width Output parameter to receive image width
     * @param height Output parameter to receive image height
     * @param components Output parameter to receive number of components
     * @param bits_per_sample Output parameter to receive bits per sample
     * @param resilient Whether to enable resilient decoding mode
     * @param reduce_level Resolution reduction level (0 = full resolution)
     * @return Decompressed data as vector of bytes
     */
    template<typename InFile>
    std::vector<uint8_t> decompress_internal(
        InFile& infile, int& width, int& height, int& components, int& bits_per_sample,
        bool resilient, int reduce_level)
    {
        // Initialize decoder
        ojph::codestream codestream;

        // Enable resilience if requested
        if (resilient) {
            codestream.enable_resilience();
        }

        // Read headers
        codestream.read_headers(&infile);

        // Restrict resolution if requested
        if (reduce_level > 0) {
            codestream.restrict_input_resolution(reduce_level, reduce_level);
        }

        // Get image parameters
        ojph::param_siz siz = codestream.access_siz();
        width = static_cast<int>(siz.get_image_extent().x);
        height = static_cast<int>(siz.get_image_extent().y);
        components = static_cast<int>(siz.get_num_components());
        bits_per_sample = static_cast<int>(siz.get_bit_depth(0));

        // Determine pixel size
        int pixel_size = (bits_per_sample > 8) ? 2 : 1;

        // Allocate output buffer
        std::vector<uint8_t> image_data(width * height * components * pixel_size);

        // Create decoder
        codestream.create();

        // Track lines per component
        std::vector<int> comp_lines(components, 0);

        // Decode using pull method
        ojph::ui32 comp_num;
        ojph::line_buf* line;

        while ((line = codestream.pull(comp_num)) != nullptr) {
            int y = comp_lines[comp_num]++;

            // Copy data from line buffer to our image data
            if (pixel_size == 1) {
                for (int x = 0; x < width; ++x) {
                    int idx = (y * width + x) * components + comp_num;
                    image_data[idx] = static_cast<uint8_t>(line->i32[x]);
                }
            }
            else {
                // For 16-bit samples
                uint16_t* dest = reinterpret_cast<uint16_t*>(image_data.data());
                for (int x = 0; x < width; ++x) {
                    int idx = (y * width + x) * components + comp_num;
                    dest[idx] = static_cast<uint16_t>(line->i32[x]);
                }
            }

            if (y == height - 1) {
                // Last line for this component, flush it
                break;
            }
        }

        codestream.close();
        return image_data;
    }
}

std::vector<uint8_t> Htj2kProcessor::compress(
    const uint8_t* image_data, int width, int height,
    int components, int bits_per_sample,
    const CompressionParams& params)
{
    try {
        // Setup memory output
        std::vector<uint8_t> compressed_data;
        ojph::mem_outfile outfile;

        // Initialize with a reasonable buffer size
        outfile.open();

        // Use internal compression function
        if (!compress_internal(image_data, width, height, components, bits_per_sample, params, outfile)) {
            return std::vector<uint8_t>();
        }

        // Copy data from mem_outfile to our vector
        outfile.seek(0, ojph::outfile_base::seek::OJPH_SEEK_END);
        size_t data_size = static_cast<size_t>(outfile.tell());
        const uint8_t* data_ptr = outfile.get_data();
        compressed_data.resize(data_size);
        std::memcpy(compressed_data.data(), data_ptr, data_size);

        outfile.close();

        return compressed_data;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K compression error: ") + e.what());
    }
}

bool Htj2kProcessor::compress(
    const uint8_t* image_data, int width, int height,
    int components, int bits_per_sample,
    uint8_t* compressed_data, size_t& compressed_size,
    const CompressionParams& params)
{
    try {
        // Use the existing compress function to get the compressed data
        std::vector<uint8_t> result = compress(image_data, width, height, components, bits_per_sample, params);

        // Check if the provided buffer is large enough
        if (compressed_size < result.size()) {
            // Update the size requirement for the caller
            compressed_size = result.size();
            return false; // Buffer too small
        }

        // Copy the compressed data to the provided buffer
        std::memcpy(compressed_data, result.data(), result.size());

        // Update the output size
        compressed_size = result.size();

        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K compression error: ") + e.what());
    }
    catch (...) {
        throw std::runtime_error("Unknown error during HTJ2K compression");
    }
}

bool Htj2kProcessor::compressToFile(
    const uint8_t* image_data, int width, int height,
    int components, int bits_per_sample, const std::string& output_filename,
    const CompressionParams& params)
{
    try {
        // Setup file output
        ojph::j2c_outfile j2c_file;
        j2c_file.open(output_filename.c_str());

        // Use internal compression function
        return compress_internal(image_data, width, height, components, bits_per_sample, params, j2c_file);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K file compression error: ") + e.what());
    }
}

std::vector<uint8_t> Htj2kProcessor::decompress(
    const uint8_t* compressed_data, size_t compressed_size,
    int& width, int& height, int& components, int& bits_per_sample,
    bool resilient, int reduce_level)
{
    try {
        // Setup memory input
        ojph::mem_infile infile;
        infile.open(compressed_data, compressed_size);

        // Use internal decompression function
        return decompress_internal(infile, width, height, components, bits_per_sample, resilient, reduce_level);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K decompression error: ") + e.what());
    }
    catch (...) {
        throw std::runtime_error("Unknown error during HTJ2K decompression");
    }
}

/**
 * @brief Decompress HTJ2K codestream to raw image data using pre-allocated buffer
 *
 * This variant of the decompress function allows the caller to provide their own
 * pre-allocated buffer to store the decompressed data.
 *
 * @param compressed_data Pointer to compressed HTJ2K data
 * @param compressed_size Size of compressed data in bytes
 * @param decompressed_data Pointer to pre-allocated buffer for decompressed data
 * @param decompressed_size Reference to size variable. On input, it contains the size of the buffer.
 *                          On output, it contains the actual size of the decompressed data.
 * @param width Output parameter to receive image width
 * @param height Output parameter to receive image height
 * @param components Output parameter to receive number of components
 * @param bits_per_sample Output parameter to receive bits per sample
 * @param resilient Whether to enable resilient decoding mode
 * @param reduce_level Resolution reduction level (0 = full resolution)
 * @return true if successful, false otherwise
 */
bool Htj2kProcessor::decompress(
    const uint8_t* compressed_data, size_t compressed_size,
    uint8_t* decompressed_data, size_t& decompressed_size,
    int& width, int& height, int& components, int& bits_per_sample,
    bool resilient, int reduce_level)
{
    try {
        // Use the original function to decode the data
        std::vector<uint8_t> result = decompress(
            compressed_data, compressed_size,
            width, height, components, bits_per_sample,
            resilient, reduce_level);

        // Calculate the required buffer size
        size_t required_size = result.size();

        // Check if the provided buffer is large enough
        if (decompressed_size < required_size) {
            // Update the size requirement for the caller
            decompressed_size = required_size;
            return false; // Buffer too small
        }

        // Copy the decompressed data to the provided buffer
        std::memcpy(decompressed_data, result.data(), required_size);

        // Update the output size
        decompressed_size = required_size;

        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K decompression error: ") + e.what());
    }
    catch (...) {
        throw std::runtime_error("Unknown error during HTJ2K decompression");
    }
}

std::vector<uint8_t> Htj2kProcessor::decompressFromFile(
    const std::string& input_filename,
    int& width, int& height, int& components, int& bits_per_sample,
    bool resilient, int reduce_level)
{
    try {
        // Setup file input
        ojph::j2c_infile j2c_file;
        j2c_file.open(input_filename.c_str());

        // Use internal decompression function
        std::vector<uint8_t> image_data = decompress_internal(j2c_file, width, height, components,
            bits_per_sample, resilient, reduce_level);

        return image_data;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K file decompression error: ") + e.what());
    }
}
