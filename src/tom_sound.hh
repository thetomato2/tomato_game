#ifndef TOM_SOUND_HH
#define TOM_SOUND_HH

#include "tom_core.hh"
namespace
{

struct SoundOutput
{
    i32 samples_per_sec;
    u32 running_sample_index;
    i32 bytes_per_sample;
    DWORD secondary_buf_size;
    i32 latency_sample_count;
};

struct SoundOutputBuffer
{
    i32 samples_per_second;
    i32 sample_count;
    i16* samples;
    i32 tone_hertz;
};

struct SoundState
{
    f32 frames_of_audio_latency;
    SoundOutput output;
    IAudioClient* audio_client;
    IAudioRenderClient* audio_render_client;
    IAudioClock* audio_clock;
};

}  // namespace

#endif
