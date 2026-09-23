#include "snowboy_kws/detector.h"

#include <stdint.h>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string resource;
    std::string model;
    std::string input;
    std::string sensitivity = "0.5";
    float gain = 1.0f;
    bool frontend = false;
    bool raw = false;
    size_t chunk_samples = 1600;
};

uint16_t read_u16_le(const unsigned char *data)
{
    return static_cast<uint16_t>(data[0]) |
           static_cast<uint16_t>(data[1] << 8);
}

uint32_t read_u32_le(const unsigned char *data)
{
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
}

bool parse_size(const char *text, size_t *value)
{
    char *end = NULL;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if(!text[0] || !end || *end != '\0' || parsed == 0 || parsed > 4096) return false;
    *value = static_cast<size_t>(parsed);
    return true;
}

bool parse_float(const char *text, float *value)
{
    char *end = NULL;
    const float parsed = std::strtof(text, &end);
    if(!text[0] || !end || *end != '\0' || parsed <= 0.0f || parsed > 20.0f) return false;
    *value = parsed;
    return true;
}

void usage(const char *program)
{
    std::cerr
        << "Usage: " << program
        << " --resource FILE --model FILE --input FILE [options]\n"
        << "Options:\n"
        << "  --sensitivity VALUE  Snowboy sensitivity (default: 0.5)\n"
        << "  --gain VALUE         Input gain in (0, 20] (default: 1.0)\n"
        << "  --frontend           Enable the Snowboy audio frontend\n"
        << "  --raw                Input is raw 16 kHz mono S16_LE PCM\n"
        << "  --chunk SAMPLES      Samples per call, 1..4096 (default: 1600)\n";
}

bool parse_options(int argc, char **argv, Options *options)
{
    for(int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if(argument == "--frontend") {
            options->frontend = true;
        } else if(argument == "--raw") {
            options->raw = true;
        } else if(argument == "--resource" && index + 1 < argc) {
            options->resource = argv[++index];
        } else if(argument == "--model" && index + 1 < argc) {
            options->model = argv[++index];
        } else if(argument == "--input" && index + 1 < argc) {
            options->input = argv[++index];
        } else if(argument == "--sensitivity" && index + 1 < argc) {
            options->sensitivity = argv[++index];
        } else if(argument == "--gain" && index + 1 < argc) {
            if(!parse_float(argv[++index], &options->gain)) return false;
        } else if(argument == "--chunk" && index + 1 < argc) {
            if(!parse_size(argv[++index], &options->chunk_samples)) return false;
        } else {
            return false;
        }
    }
    return !options->resource.empty() && !options->model.empty() &&
           !options->input.empty();
}

bool seek_wave_data(std::ifstream *input, uint32_t *data_size, std::string *error)
{
    unsigned char header[12];
    if(!input->read(reinterpret_cast<char *>(header), sizeof(header)) ||
       std::string(reinterpret_cast<char *>(header), 4) != "RIFF" ||
       std::string(reinterpret_cast<char *>(header + 8), 4) != "WAVE") {
        *error = "input is not a RIFF/WAVE file (use --raw for headerless PCM)";
        return false;
    }

    bool format_ok = false;
    while(*input) {
        unsigned char chunk_header[8];
        if(!input->read(reinterpret_cast<char *>(chunk_header), sizeof(chunk_header))) break;
        const std::string chunk_id(reinterpret_cast<char *>(chunk_header), 4);
        const uint32_t chunk_size = read_u32_le(chunk_header + 4);

        if(chunk_id == "fmt ") {
            if(chunk_size < 16) {
                *error = "WAVE fmt chunk is too short";
                return false;
            }
            std::vector<unsigned char> format(chunk_size);
            if(!input->read(reinterpret_cast<char *>(format.data()), chunk_size)) {
                *error = "cannot read WAVE fmt chunk";
                return false;
            }
            const uint16_t encoding = read_u16_le(format.data());
            const uint16_t channels = read_u16_le(format.data() + 2);
            const uint32_t sample_rate = read_u32_le(format.data() + 4);
            const uint16_t bits = read_u16_le(format.data() + 14);
            format_ok = encoding == 1 && channels == 1 && sample_rate == 16000 && bits == 16;
        } else if(chunk_id == "data") {
            if(!format_ok) {
                *error = "WAVE must be 16 kHz mono 16-bit PCM with fmt before data";
                return false;
            }
            *data_size = chunk_size;
            return true;
        } else {
            input->seekg(chunk_size, std::ios::cur);
        }

        if(chunk_size & 1U) input->seekg(1, std::ios::cur);
    }

    *error = "WAVE data chunk was not found";
    return false;
}

}  // namespace

int main(int argc, char **argv)
{
    Options options;
    if(!parse_options(argc, argv, &options)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    snowboy_kws::Detector detector(options.resource.c_str(), options.model.c_str(),
                                   options.sensitivity.c_str(), options.frontend,
                                   options.gain);
    if(!detector.valid()) {
        std::cerr << "Cannot initialize detector: " << detector.error() << '\n';
        return EXIT_FAILURE;
    }

    std::ifstream input(options.input.c_str(), std::ios::binary);
    if(!input) {
        std::cerr << "Cannot open input: " << options.input << '\n';
        return EXIT_FAILURE;
    }

    uint32_t remaining_bytes = 0;
    if(!options.raw) {
        std::string error;
        if(!seek_wave_data(&input, &remaining_bytes, &error)) {
            std::cerr << error << '\n';
            return EXIT_FAILURE;
        }
    }

    std::vector<int16_t> samples(options.chunk_samples);
    uint64_t processed_samples = 0;
    unsigned int detections = 0;
    while(input) {
        size_t wanted_bytes = samples.size() * sizeof(samples[0]);
        if(!options.raw && wanted_bytes > remaining_bytes) wanted_bytes = remaining_bytes;
        if(wanted_bytes == 0) break;

        input.read(reinterpret_cast<char *>(samples.data()), wanted_bytes);
        const std::streamsize bytes_read = input.gcount();
        const size_t sample_count = static_cast<size_t>(bytes_read) / sizeof(samples[0]);
        if(sample_count == 0) break;

        const snowboy_kws::Detection result = detector.accept(samples.data(), sample_count);
        if(result.result == -1) {
            std::cerr << "Snowboy rejected an input chunk\n";
            return EXIT_FAILURE;
        }
        processed_samples += sample_count;
        if(result.detected) {
            ++detections;
            const double seconds = static_cast<double>(processed_samples) / 16000.0;
            std::cout << "detected hotword_index=" << result.result
                      << " time_seconds=" << std::fixed << std::setprecision(3)
                      << seconds << '\n';
        }

        if(!options.raw) {
            remaining_bytes -= static_cast<uint32_t>(sample_count * sizeof(samples[0]));
        }
    }

    std::cout << "processed_samples=" << processed_samples
              << " detections=" << detections << '\n';
    return EXIT_SUCCESS;
}
