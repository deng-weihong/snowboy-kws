#ifndef SNOWBOY_KWS_DETECTOR_H
#define SNOWBOY_KWS_DETECTOR_H

#include <stddef.h>
#include <stdint.h>

namespace snowboy_kws {

struct Detection {
    bool detected;
    int result;
};

/*
 * Small ownership-safe adapter around SnowboyDetect. Audio capture and
 * threading stay with the caller; this class only consumes mono PCM chunks.
 */
class Detector {
public:
    Detector(const char *resource_path,
             const char *model_path,
             const char *sensitivity,
             bool apply_frontend,
             float audio_gain);
    ~Detector();

    Detector(const Detector &) = delete;
    Detector &operator=(const Detector &) = delete;

    bool valid() const;
    const char *error() const;
    void reset();
    Detection accept(const int16_t *samples, size_t sample_count);

    int sample_rate() const;
    int channels() const;
    int bits_per_sample() const;

private:
    void *detector_;
    bool valid_;
    char error_[192];
};

}  // namespace snowboy_kws

#endif
