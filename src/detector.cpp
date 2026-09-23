#include "snowboy_kws/detector.h"

#include "snowboy-detect.h"

#include <stdio.h>

#include <exception>
#include <string>

namespace snowboy_kws {

Detector::Detector(const char *resource_path,
                   const char *model_path,
                   const char *sensitivity,
                   bool apply_frontend,
                   float audio_gain)
    : detector_(NULL), valid_(false)
{
    error_[0] = '\0';
    if(!resource_path || !resource_path[0] || !model_path || !model_path[0]) {
        snprintf(error_, sizeof(error_), "resource and model paths are required");
        return;
    }

    try {
        snowboy::SnowboyDetect *detector =
            new snowboy::SnowboyDetect(std::string(resource_path),
                                       std::string(model_path));
        detector_ = detector;
        if(sensitivity && sensitivity[0]) detector->SetSensitivity(sensitivity);
        detector->SetAudioGain(audio_gain);
        detector->ApplyFrontend(apply_frontend);
        if(detector->SampleRate() != 16000 || detector->NumChannels() != 1 ||
           detector->BitsPerSample() != 16) {
            snprintf(error_, sizeof(error_),
                     "unsupported Snowboy audio contract: rate=%d channels=%d bits=%d",
                     detector->SampleRate(), detector->NumChannels(),
                     detector->BitsPerSample());
            delete detector;
            detector_ = NULL;
            return;
        }
        valid_ = true;
    } catch(const std::exception &exception) {
        snprintf(error_, sizeof(error_), "Snowboy initialization failed: %s",
                 exception.what());
    } catch(...) {
        snprintf(error_, sizeof(error_), "Snowboy initialization failed");
    }
}

Detector::~Detector()
{
    delete static_cast<snowboy::SnowboyDetect *>(detector_);
    detector_ = NULL;
}

bool Detector::valid() const { return valid_; }

const char *Detector::error() const
{
    return error_[0] ? error_ : "";
}

void Detector::reset()
{
    if(detector_) static_cast<snowboy::SnowboyDetect *>(detector_)->Reset();
}

Detection Detector::accept(const int16_t *samples, size_t sample_count)
{
    Detection detection = {false, 0};
    if(!valid_ || !samples || sample_count == 0 || sample_count > 4096) {
        detection.result = -1;
        return detection;
    }
    const int result =
        static_cast<snowboy::SnowboyDetect *>(detector_)->RunDetection(
            samples, static_cast<int>(sample_count), false);
    detection.result = result;
    detection.detected = result > 0;
    return detection;
}

int Detector::sample_rate() const
{
    return detector_ ? static_cast<snowboy::SnowboyDetect *>(detector_)->SampleRate() : 0;
}

int Detector::channels() const
{
    return detector_ ? static_cast<snowboy::SnowboyDetect *>(detector_)->NumChannels() : 0;
}

int Detector::bits_per_sample() const
{
    return detector_ ? static_cast<snowboy::SnowboyDetect *>(detector_)->BitsPerSample() : 0;
}

}  // namespace snowboy_kws
