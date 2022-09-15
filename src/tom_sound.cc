namespace tom
{

fn void init_WASAPI(i32 samples_per_second, i32 buffer_size_in_samples)
{
#if 0
    if (FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) {
        INVALID_CODE_PATH;
    }

    IMMDeviceEnumerator *enumerator = {};
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&enumerator)))) {
        INVALID_CODE_PATH;
    }

    IMMDevice *device = {};
    if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        INVALID_CODE_PATH;
    }

    if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
                                (LPVOID *)&g_audio_client))) {
        INVALID_CODE_PATH;
    }

    WAVEFORMATEXTENSIBLE wave_format  = {};
    wave_format.Format.cbSize         = sizeof(wave_format);
    wave_format.Format.wFormatTag     = WAVE_FORMAT_EXTENSIBLE;
    wave_format.Format.wBitsPerSample = 16;
    wave_format.Format.nChannels      = 2;
    wave_format.Format.nSamplesPerSec = scast(DWORD, samples_per_second);
    wave_format.Format.nBlockAlign =
    (WORD)(wave_format.Format.nChannels * wave_format.Format.wBitsPerSample / 8);
    wave_format.Format.nAvgBytesPerSec =
        wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;
    wave_format.Samples.wValidBitsPerSample = 16;
    wave_format.dwChannelMask               = KSAUDIO_SPEAKER_STEREO;
    wave_format.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

    // buffer size in 100 nanoseconds
    REFERENCE_TIME buffer_duration =
        scast(REFERENCE_TIME, 10000000ULL * buffer_size_in_samples / samples_per_second);

    if (FAILED(g_audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
                                          buffer_duration, 0, &wave_format.Format, nullptr))) {
        INVALID_CODE_PATH;
    }

    if (FAILED(g_audio_client->GetService(IID_PPV_ARGS(&g_audio_render_client)))) {
        INVALID_CODE_PATH;
    }

    UINT32 sound_frame_cnt;
    if (FAILED(g_audio_client->GetBufferSize(&sound_frame_cnt))) {
        INVALID_CODE_PATH;
    }

    if (FAILED(g_audio_client->GetService(IID_PPV_ARGS(&g_audio_clock)))) {
        INVALID_CODE_PATH;
    }

    // Check if we got what we requested (better would to pass this value back
    // as real buffer size)
    TOM_ASSERT(buffer_size_in_samples <= scast(i32, sound_frame_cnt));
#endif
}

fn void fill_sound_buffer(SoundOutput& sound_output, i32 samples_to_write,
                                SoundOutputBuffer& source_buffer)
{
#if 0
    {
        BYTE *sound_buf_dat;
        if (SUCCEEDED(g_audio_render_client->GetBuffer((UINT32)samples_to_write,
                                                       &sound_buf_dat))) {
            i16 *sourceSample = source_buffer.samples;
            i16 *destSample   = (i16 *) sound_buf_dat;
            for (szt i = 0; i < samples_to_write; ++i) {
                *destSample++ = *sourceSample++;
                *destSample++ = *sourceSample++;
                ++sound_output.running_sample_index;
            }

            g_audio_render_client->ReleaseBuffer((UINT32) samples_to_write, 0);
        }
    }
#endif
}
}
