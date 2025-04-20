#include "Htj2kProcessor.h"

std::vector<uint8_t> Htj2kProcessor::compress(
    const uint8_t* image_data, int width, int height,
    int components, int bits_per_sample,
    const CompressionParams& params)
{
    try {
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
        //if (num_precincts != -1)
        //    cod.set_precinct_size(num_precincts, precinct_size);

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
        if (params.precincts.size() > 0) {
            // Convert our precinct size format to ojph::size format
            std::vector<ojph::size> ojph_precincts;
            for (const auto& prec : params.precincts) {
                ojph_precincts.push_back(ojph::size{ prec.width, prec.height });
            }
            cod.set_precinct_size(ojph_precincts.size(), ojph_precincts.data());
        }

        //if (profile_string[0] != '\0')
        //    codestream.set_profile(profile_string);
        //codestream.set_tilepart_divisions(tileparts_at_resolutions,
        //    tileparts_at_components);
        //codestream.request_tlm_marker(tlm_marker);

        // Prepare for interleaved or planar data
        codestream.set_planar(params.is_planar);

        // Setup memory output
        std::vector<uint8_t> compressed_data;
        ojph::mem_outfile outfile;
        // Initialize with a reasonable buffer size (1MB)
        outfile.open();

        // Write headers
        codestream.write_headers(&outfile);

        // Create buffer for encoding
        //codestream.create();

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

        // Flush and close
        codestream.flush();

        // Copy data from mem_outfile to our vector
        size_t data_size = static_cast<size_t>(outfile.tell());
        const uint8_t* data_ptr = outfile.get_data();
        compressed_data.resize(data_size);
        std::memcpy(compressed_data.data(), data_ptr, data_size);

        codestream.close();
        outfile.close();

        return compressed_data;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K compression error: ") + e.what());
    }
}

std::vector<uint8_t> Htj2kProcessor::decompress(
    const uint8_t* compressed_data, size_t compressed_size,
    int& width, int& height, int& components, int& bits_per_sample,
    bool resilient, int reduce_level)
{
    try {
        // Initialize decoder
        ojph::codestream codestream;
        ojph::mem_infile infile;

        // Open memory file
        infile.open(compressed_data, compressed_size);

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
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K decompression error: ") + e.what());
    }
    catch (...) {
        throw std::runtime_error("Unknown error during HTJ2K decompression");
    }
}

bool Htj2kProcessor::compressToFile(
    const uint8_t* image_data, int width, int height,
    int components, int bits_per_sample, const std::string& output_filename,
    const CompressionParams& params)
{
    try {
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
        if (params.precincts.size() > 0) {
            // Convert our precinct size format to ojph::size format
            std::vector<ojph::size> ojph_precincts;
            for (const auto& prec : params.precincts) {
                ojph_precincts.push_back(ojph::size{ prec.width, prec.height });
            }
            cod.set_precinct_size(ojph_precincts.size(), ojph_precincts.data());
        }

        // Setup file output
        ojph::j2c_outfile j2c_file;
        j2c_file.open(output_filename.c_str());

        // Write headers
        codestream.write_headers(&j2c_file);

        // Create buffer for encoding
        //codestream.create();

        // Prepare for interleaved or planar data
        codestream.set_planar(params.is_planar);

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

        // Flush and close
        codestream.flush();
        codestream.close();
        j2c_file.close();

        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K file compression error: ") + e.what());
    }
}

std::vector<uint8_t> Htj2kProcessor::decompressFromFile(
    const std::string& input_filename,
    int& width, int& height, int& components, int& bits_per_sample,
    bool resilient, int reduce_level)
{
    try {
        // Initialize decoder
        ojph::codestream codestream;
        ojph::j2c_infile j2c_file;

        // Open file - using c_str() to convert std::string to const char*
        j2c_file.open(input_filename.c_str());

        // Enable resilience if requested
        if (resilient) {
            codestream.enable_resilience();
        }

        // Read headers
        codestream.read_headers(&j2c_file);

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
        }

        codestream.close();
        j2c_file.close();

        return image_data;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTJ2K file decompression error: ") + e.what());
    }
}

void Htj2kProcessor::fill_line_buffer(ojph::line_buf* line_buffer,
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