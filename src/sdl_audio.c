#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fixer.h"
#include "3dc.h"
#include "platform.h"
#include "inline.h"
#include "psndplat.h"
#include "gamedef.h"
#include "avpview.h"
#include "ffstdio.h"
#include "dynamics.h"
#include "dynblock.h"
#include "stratdef.h"

SOUNDSAMPLEDATA GameSounds[SID_MAXIMUM];
ACTIVESOUNDSAMPLE ActiveSounds[SOUND_MAXACTIVE];
SOUNDSAMPLEDATA BlankGameSound = {0, 0, 0, 0, 0, NULL, 0, 0, NULL, 0};
ACTIVESOUNDSAMPLE BlankActiveSound = {SID_NOSOUND, ASP_Minimum, 0, 0, NULL, 0, 0, 0, 0, 0, {{0, 0, 0}, 0, 0}, 0, 0, {0, 0, 0}, {0, 0, 0}, NULL, NULL, NULL};

#define SDL_AUDIO_RATE 22050
#define SDL_AUDIO_CHANNELS 2
#define SDL_AUDIO_BYTES_PER_SAMPLE 2
#define SDL_AUDIO_REFERENCE_DISTANCE 5200.0f
#define SDL_AUDIO_ROLLOFF 2.0f
#define SDL_PI 3.14159265358979323846f

typedef struct SDLSample
{
    int channels;
    int rate;
    size_t frames;
    int16_t *pcm;
    int refs;
    int retired;
} SDLSample;

typedef struct SDLVoice
{
    SDLSample *sample;
    double position;
    int playing;
    int loop;
    int threedee;
    int paused;
    float gain;
    VECTORCH source_position;
    int serial;
} SDLVoice;

typedef struct SDLVoiceSnapshot
{
    SDLSample *sample;
    double position;
    int playing;
    int loop;
    int threedee;
    float gain;
    VECTORCH source_position;
    int serial;
} SDLVoiceSnapshot;

typedef struct SDLListenerState
{
    float position[3];
    float forward[3];
    float up[3];
} SDLListenerState;

extern int WantSound;
extern DISPLAYBLOCK *Player;

static SDL_AudioStream *AudioStream;
static SDL_AudioDeviceID AudioDevice;
static SDL_Mutex *AudioMutex;
static int SoundActivated;
static int AudioShuttingDown;
static SDLListenerState Listener = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}};
static SDLVoice Voices[SOUND_MAXACTIVE];

static void FreeSample(SDLSample *sample)
{
    if (!sample)
        return;
    free(sample->pcm);
    free(sample);
}

static void ReleaseSampleLocked(SDLSample *sample)
{
    if (!sample)
        return;
    sample->refs--;
    if (sample->refs == 0 && sample->retired)
        FreeSample(sample);
}

static void RetireSampleLocked(SDLSample *sample)
{
    if (!sample)
        return;
    sample->retired = 1;
    ReleaseSampleLocked(sample);
}

static void SetVoiceSampleLocked(SDLVoice *voice, SDLSample *sample)
{
    if (voice->sample == sample)
        return;
    ReleaseSampleLocked(voice->sample);
    voice->sample = sample;
    if (sample)
        sample->refs++;
}

static int ParseWAV(const unsigned char *data, size_t data_size,
                    const unsigned char **fmt_ptr, const unsigned char **data_ptr,
                    uint32_t *fmt_len, uint32_t *data_len)
{
    size_t p = 12;
    size_t riff_length;
    size_t riff_end;
    const unsigned char *fmt = NULL;
    const unsigned char *pcm = NULL;
    uint32_t fl = 0;
    uint32_t dl = 0;

    if (!data || data_size < 12 || memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0)
        return 0;

    riff_length = (size_t)data[4] |
                  ((size_t)data[5] << 8) |
                  ((size_t)data[6] << 16) |
                  ((size_t)data[7] << 24);
    riff_end = 8u + riff_length;
    if (riff_end > data_size)
        riff_end = data_size;

    while (p + 8 <= riff_end)
    {
        uint32_t len = (uint32_t)data[p + 4] |
                       ((uint32_t)data[p + 5] << 8) |
                       ((uint32_t)data[p + 6] << 16) |
                       ((uint32_t)data[p + 7] << 24);
        size_t payload = p + 8;

        if (len > riff_end - payload)
            break;
        if (!fmt && memcmp(data + p, "fmt ", 4) == 0)
        {
            fmt = data + p;
            fl = len;
        }
        else if (!pcm && memcmp(data + p, "data", 4) == 0)
        {
            pcm = data + payload;
            dl = len;
        }

        p = payload + len + (len & 1u);
        if (p > riff_end)
            break;
    }

    if (!fmt || !pcm || fl < 16 || dl == 0)
        return 0;

    *fmt_ptr = fmt;
    *data_ptr = pcm;
    *fmt_len = fl;
    *data_len = dl;
    return 1;
}

static SDLSample *DecodeWAV(const unsigned char *data, size_t data_size)
{
    const unsigned char *fmt;
    const unsigned char *pcm;
    uint32_t fmt_len;
    uint32_t pcm_len;
    uint16_t tag;
    uint16_t channels;
    uint32_t rate;
    uint16_t bits;
    uint16_t block_align;
    int extensible_pcm = 0;
    size_t bytes_per_sample;
    size_t bytes_per_frame;
    size_t frames;
    size_t i;
    SDLSample *sample;

    if (!ParseWAV(data, data_size, &fmt, &pcm, &fmt_len, &pcm_len))
        return NULL;

    tag = (uint16_t)(fmt[8] | (fmt[9] << 8));
    channels = (uint16_t)(fmt[10] | (fmt[11] << 8));
    rate = (uint32_t)fmt[12] |
           ((uint32_t)fmt[13] << 8) |
           ((uint32_t)fmt[14] << 16) |
           ((uint32_t)fmt[15] << 24);
    block_align = (uint16_t)(fmt[20] | (fmt[21] << 8));
    bits = (uint16_t)(fmt[22] | (fmt[23] << 8));

    if (tag == 0xFFFE && fmt_len >= 40)
    {
        static const unsigned char pcm_guid[16] =
            {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};
        if (memcmp(fmt + 24, pcm_guid, sizeof(pcm_guid)) == 0)
            extensible_pcm = 1;
    }

    if (tag != 1 && !extensible_pcm)
        return NULL;
    if ((channels != 1 && channels != 2) || rate == 0)
        return NULL;
    if (bits != 8 && bits != 16 && bits != 24 && bits != 32)
        return NULL;

    bytes_per_sample = (size_t)bits / 8;
    bytes_per_frame = (size_t)block_align;
    if (bytes_per_frame == 0)
        bytes_per_frame = (size_t)channels * bytes_per_sample;
    if (bytes_per_frame == 0 || pcm_len % bytes_per_frame)
        return NULL;

    frames = pcm_len / bytes_per_frame;
    sample = (SDLSample *)calloc(1, sizeof(*sample));
    if (!sample)
        return NULL;

    sample->channels = channels;
    sample->rate = (int)rate;
    sample->frames = frames;
    sample->refs = 1;
    sample->pcm = (int16_t *)malloc(frames * channels * sizeof(int16_t));
    if (!sample->pcm)
    {
        free(sample);
        return NULL;
    }

    if (bits == 8)
    {
        for (i = 0; i < frames * channels; ++i)
            sample->pcm[i] = (int16_t)(((int)pcm[i] - 128) << 8);
    }
    else if (bits == 16)
    {
        for (i = 0; i < frames * channels; ++i)
            sample->pcm[i] = (int16_t)((uint16_t)pcm[i * 2] | ((uint16_t)pcm[i * 2 + 1] << 8));
    }
    else if (bits == 24)
    {
        for (i = 0; i < frames * channels; ++i)
        {
            int32_t value = (int32_t)((uint32_t)pcm[i * 3] |
                                      ((uint32_t)pcm[i * 3 + 1] << 8) |
                                      ((uint32_t)pcm[i * 3 + 2] << 16));
            if (value & 0x00800000)
                value |= (int32_t)0xFF000000;
            sample->pcm[i] = (int16_t)(value >> 8);
        }
    }
    else
    {
        for (i = 0; i < frames * channels; ++i)
        {
            uint32_t bits32 = (uint32_t)pcm[i * 4] |
                              ((uint32_t)pcm[i * 4 + 1] << 8) |
                              ((uint32_t)pcm[i * 4 + 2] << 16) |
                              ((uint32_t)pcm[i * 4 + 3] << 24);
            float value;
            memcpy(&value, &bits32, sizeof(value));
            if (value > 1.0f)
                value = 1.0f;
            if (value < -1.0f)
                value = -1.0f;
            sample->pcm[i] = (int16_t)lrintf(value * 32767.0f);
        }
    }

    return sample;
}

static void ResetVoiceLocked(int index)
{
    SDLVoice *voice = &Voices[index];
    SetVoiceSampleLocked(voice, NULL);
    voice->position = 0.0;
    voice->playing = 0;
    voice->loop = 0;
    voice->threedee = 0;
    voice->paused = 0;
    voice->gain = 1.0f;
    voice->source_position.vx = 0;
    voice->source_position.vy = 0;
    voice->source_position.vz = 0;
    voice->serial++;
}

static float DistanceGain(float distance)
{
    if (distance <= SDL_AUDIO_REFERENCE_DISTANCE)
        return 1.0f;
    return SDL_AUDIO_REFERENCE_DISTANCE /
           (SDL_AUDIO_REFERENCE_DISTANCE + SDL_AUDIO_ROLLOFF * (distance - SDL_AUDIO_REFERENCE_DISTANCE));
}

static void ListenerBasis(float *right)
{
    right[0] = Listener.up[1] * Listener.forward[2] - Listener.up[2] * Listener.forward[1];
    right[1] = Listener.up[2] * Listener.forward[0] - Listener.up[0] * Listener.forward[2];
    right[2] = Listener.up[0] * Listener.forward[1] - Listener.up[1] * Listener.forward[0];

    {
        float length = sqrtf(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
        if (length > 0.00001f)
        {
            right[0] /= length;
            right[1] /= length;
            right[2] /= length;
        }
        else
        {
            right[0] = 1.0f;
            right[1] = 0.0f;
            right[2] = 0.0f;
        }
    }
}

static void VoiceGains(const SDLVoiceSnapshot *voice, float *left, float *right)
{
    if (!voice->threedee)
    {
        *left = 0.7f * voice->gain;
        *right = 0.7f * voice->gain;
        return;
    }

    {
        float rel[3];
        float distance;
        float attenuation;
        float direction[3];
        float speaker_right[3];
        float pan;
        float angle;

        rel[0] = (float)voice->source_position.vx - Listener.position[0];
        rel[1] = (float)voice->source_position.vy - Listener.position[1];
        rel[2] = (float)voice->source_position.vz - Listener.position[2];
        distance = sqrtf(rel[0] * rel[0] + rel[1] * rel[1] + rel[2] * rel[2]);
        attenuation = DistanceGain(distance);

        if (distance > 0.00001f)
        {
            direction[0] = rel[0] / distance;
            direction[1] = rel[1] / distance;
            direction[2] = rel[2] / distance;
        }
        else
        {
            direction[0] = 0.0f;
            direction[1] = 0.0f;
            direction[2] = 0.0f;
        }

        ListenerBasis(speaker_right);
        pan = direction[0] * speaker_right[0] +
              direction[1] * speaker_right[1] +
              direction[2] * speaker_right[2];
        if (pan < -1.0f)
            pan = -1.0f;
        if (pan > 1.0f)
            pan = 1.0f;

        angle = (pan + 1.0f) * (SDL_PI * 0.25f);
        *left = cosf(angle) * attenuation * voice->gain;
        *right = sinf(angle) * attenuation * voice->gain;
    }
}

static void MixFrames(int16_t *out, size_t frames)
{
    SDLVoiceSnapshot snapshots[SOUND_MAXACTIVE];
    size_t i;
    size_t f;

    memset(out, 0, frames * SDL_AUDIO_CHANNELS * SDL_AUDIO_BYTES_PER_SAMPLE);
    memset(snapshots, 0, sizeof(snapshots));

    SDL_LockMutex(AudioMutex);
    for (i = 0; i < SOUND_MAXACTIVE; ++i)
    {
        SDLVoice *voice = &Voices[i];
        if (!voice->playing || !voice->sample || voice->sample->frames == 0)
            continue;

        snapshots[i].sample = voice->sample;
        snapshots[i].position = voice->position;
        snapshots[i].playing = voice->playing;
        snapshots[i].loop = voice->loop;
        snapshots[i].threedee = voice->threedee;
        snapshots[i].gain = voice->gain;
        snapshots[i].source_position = voice->source_position;
        snapshots[i].serial = voice->serial;
        voice->sample->refs++;
    }
    SDL_UnlockMutex(AudioMutex);

    for (i = 0; i < SOUND_MAXACTIVE; ++i)
    {
        SDLVoiceSnapshot *voice = &snapshots[i];
        SDLSample *sample = voice->sample;
        if (!voice->playing || !sample)
            continue;

        {
            float left_gain;
            float right_gain;
            double step = (double)sample->rate / (double)SDL_AUDIO_RATE;

            VoiceGains(voice, &left_gain, &right_gain);

            for (f = 0; f < frames; ++f)
            {
                size_t a;
                size_t b;
                float frac;
                float left_sample;
                float right_sample;
                float mixed_left;
                float mixed_right;

                if (voice->position >= (double)sample->frames)
                {
                    if (voice->loop)
                        voice->position = fmod(voice->position, (double)sample->frames);
                    else
                    {
                        voice->playing = 0;
                        break;
                    }
                }

                a = (size_t)voice->position;
                b = (a + 1 < sample->frames) ? a + 1 : a;
                frac = (float)(voice->position - (double)a);

                if (sample->channels == 1)
                {
                    float a_sample = (float)sample->pcm[a];
                    float b_sample = (float)sample->pcm[b];
                    left_sample = a_sample + (b_sample - a_sample) * frac;
                    right_sample = left_sample;
                }
                else
                {
                    size_t ai = a * 2;
                    size_t bi = b * 2;
                    float a_left = (float)sample->pcm[ai];
                    float b_left = (float)sample->pcm[bi];
                    float a_right = (float)sample->pcm[ai + 1];
                    float b_right = (float)sample->pcm[bi + 1];
                    left_sample = a_left + (b_left - a_left) * frac;
                    right_sample = a_right + (b_right - a_right) * frac;
                }

                mixed_left = (float)out[f * 2] + left_sample * left_gain;
                mixed_right = (float)out[f * 2 + 1] + right_sample * right_gain;
                if (mixed_left > 32767.0f)
                    mixed_left = 32767.0f;
                if (mixed_left < -32768.0f)
                    mixed_left = -32768.0f;
                if (mixed_right > 32767.0f)
                    mixed_right = 32767.0f;
                if (mixed_right < -32768.0f)
                    mixed_right = -32768.0f;
                out[f * 2] = (int16_t)mixed_left;
                out[f * 2 + 1] = (int16_t)mixed_right;
                voice->position += step;
            }
        }
    }

    SDL_LockMutex(AudioMutex);
    for (i = 0; i < SOUND_MAXACTIVE; ++i)
    {
        SDLVoiceSnapshot *snapshot = &snapshots[i];
        SDLVoice *voice = &Voices[i];
        if (!snapshot->sample)
            continue;

        if (voice->serial == snapshot->serial && voice->sample == snapshot->sample)
        {
            voice->position = snapshot->position;
            voice->playing = snapshot->playing;
        }

        ReleaseSampleLocked(snapshot->sample);
    }
    SDL_UnlockMutex(AudioMutex);
}

static void AudioStreamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    size_t frame_bytes = SDL_AUDIO_CHANNELS * SDL_AUDIO_BYTES_PER_SAMPLE;
    size_t frames;
    int16_t *mix;

    (void)userdata;
    (void)total_amount;

    if (additional_amount <= 0)
        return;
    if (frame_bytes == 0)
        return;

    frames = ((size_t)additional_amount + frame_bytes - 1) / frame_bytes;
    mix = (int16_t *)malloc(frames * SDL_AUDIO_CHANNELS * sizeof(int16_t));
    if (!mix)
        return;

    MixFrames(mix, frames);
    SDL_PutAudioStreamData(stream, mix, (int)(frames * frame_bytes));
    free(mix);
}

static int LoadWAVBytes(int soundNum, const char *wavName, const unsigned char *data, size_t len)
{
    SDLSample *sample = DecodeWAV(data, len);
    SDLSample *oldSample;
    char *nameCopy;

    if (!sample)
        return 0;

    nameCopy = (char *)AllocateMem(strlen(wavName) + 1);
    if (!nameCopy)
    {
        FreeSample(sample);
        return 0;
    }
    strcpy(nameCopy, wavName);

    SDL_LockMutex(AudioMutex);
    oldSample = (SDLSample *)GameSounds[soundNum].buffer;
    if (oldSample)
        RetireSampleLocked(oldSample);
    if (GameSounds[soundNum].wavName)
        DeallocateMem(GameSounds[soundNum].wavName);
    GameSounds[soundNum].buffer = sample;
    GameSounds[soundNum].dsFrequency = sample->rate;
    GameSounds[soundNum].length = (int)(((uint64_t)sample->frames << 16) / (sample->rate ? sample->rate : 1));
    GameSounds[soundNum].flags = SAMPLE_IN_HW;
    GameSounds[soundNum].wavName = nameCopy;
    GameSounds[soundNum].pitch = PITCH_DEFAULTPLAT;
    SDL_UnlockMutex(AudioMutex);

    return 1;
}

int PlatStartSoundSys(void)
{
    SDL_AudioSpec spec;
    int i;

    if (SoundActivated)
        return 1;
    if (WantSound == 0)
        return 0;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && !SDL_InitSubSystem(SDL_INIT_AUDIO))
        return 0;

    AudioMutex = SDL_CreateMutex();
    if (!AudioMutex)
        return 0;

    memset(Voices, 0, sizeof(Voices));
    for (i = 0; i < SOUND_MAXACTIVE; ++i)
        Voices[i].serial = 1;

    Listener.position[0] = 0.0f;
    Listener.position[1] = 0.0f;
    Listener.position[2] = 0.0f;
    Listener.forward[0] = 0.0f;
    Listener.forward[1] = 0.0f;
    Listener.forward[2] = 1.0f;
    Listener.up[0] = 0.0f;
    Listener.up[1] = 1.0f;
    Listener.up[2] = 0.0f;

    SDL_zero(spec);
    spec.format = SDL_AUDIO_S16;
    spec.channels = SDL_AUDIO_CHANNELS;
    spec.freq = SDL_AUDIO_RATE;

    AudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioStreamCallback, NULL);
    if (!AudioStream)
    {
        SDL_DestroyMutex(AudioMutex);
        AudioMutex = NULL;
        return 0;
    }

    AudioDevice = SDL_GetAudioStreamDevice(AudioStream);
    SoundActivated = 1;
    AudioShuttingDown = 0;

    if (!SDL_ResumeAudioStreamDevice(AudioStream))
    {
        SoundActivated = 0;
        AudioShuttingDown = 1;
        SDL_DestroyAudioStream(AudioStream);
        AudioStream = NULL;
        AudioDevice = 0;
        SDL_DestroyMutex(AudioMutex);
        AudioMutex = NULL;
        return 0;
    }

    return 1;
}

void PlatEndSoundSys(void)
{
    int i;

    if (AudioStream)
    {
        AudioShuttingDown = 1;
        SoundActivated = 0;
        SDL_DestroyAudioStream(AudioStream);
        AudioStream = NULL;
        AudioDevice = 0;
    }

    if (AudioMutex)
    {
        SDL_LockMutex(AudioMutex);
        for (i = 0; i < SOUND_MAXACTIVE; ++i)
            ResetVoiceLocked(i);
        for (i = 0; i < SID_MAXIMUM; ++i)
        {
            SDLSample *sample = (SDLSample *)GameSounds[i].buffer;
            if (sample)
            {
                GameSounds[i].buffer = NULL;
                RetireSampleLocked(sample);
            }
        }
        SDL_UnlockMutex(AudioMutex);
        SDL_DestroyMutex(AudioMutex);
        AudioMutex = NULL;
    }

    AudioShuttingDown = 0;
}

int PlatPlaySound(int activeIndex)
{
    int soundIndex;
    SDLSample *sample;
    SDLVoice *voice;

    if (!SoundActivated || !AudioMutex || activeIndex < 0 || activeIndex >= SOUND_MAXACTIVE)
        return 0;

    soundIndex = ActiveSounds[activeIndex].soundIndex;
    if (soundIndex < 0 || soundIndex >= SID_MAXIMUM)
        return 0;

    SDL_LockMutex(AudioMutex);
    sample = (SDLSample *)GameSounds[soundIndex].buffer;
    if (!GameSounds[soundIndex].loaded || !sample || sample->frames == 0)
    {
        SDL_UnlockMutex(AudioMutex);
        return 0;
    }

    voice = &Voices[activeIndex];
    SetVoiceSampleLocked(voice, sample);
    voice->position = 0.0;
    voice->playing = !ActiveSounds[activeIndex].paused;
    voice->loop = ActiveSounds[activeIndex].loop ? 1 : 0;
    voice->threedee = ActiveSounds[activeIndex].threedee ? 1 : 0;
    voice->paused = ActiveSounds[activeIndex].paused ? 1 : 0;
    voice->gain = 1.0f;
    voice->source_position = ActiveSounds[activeIndex].threedeedata.position;
    voice->serial++;
    SDL_UnlockMutex(AudioMutex);

    return 1;
}

void PlatStopSound(int activeIndex)
{
    if (!AudioMutex || activeIndex < 0 || activeIndex >= SOUND_MAXACTIVE)
        return;

    SDL_LockMutex(AudioMutex);
    Voices[activeIndex].playing = 0;
    Voices[activeIndex].paused = 0;
    Voices[activeIndex].position = 0.0;
    Voices[activeIndex].serial++;
    SDL_UnlockMutex(AudioMutex);
}

int PlatChangeGlobalVolume(int volume)
{
    (void)volume;
    return 1;
}

int PlatChangeSoundVolume(int activeIndex, int volume)
{
    (void)activeIndex;
    (void)volume;
    return 1;
}

int PlatChangeSoundPitch(int activeIndex, int pitch)
{
    if (activeIndex < 0 || activeIndex >= SOUND_MAXACTIVE)
        return 0;
    if (pitch < PITCH_MIN || pitch >= PITCH_MAX)
        return 0;
    ActiveSounds[activeIndex].pitch = pitch;
    return 1;
}

int PlatSoundHasStopped(int activeIndex)
{
    int stopped;

    if (!AudioMutex || activeIndex < 0 || activeIndex >= SOUND_MAXACTIVE)
        return 1;

    SDL_LockMutex(AudioMutex);
    stopped = Voices[activeIndex].playing ? 0 : 1;
    SDL_UnlockMutex(AudioMutex);
    return stopped;
}

int PlatDo3dSound(int activeIndex)
{
    VECTORCH relativePosn;
    int distance;
    int outerRange;
    int innerRange;

    if (!AudioMutex || activeIndex < 0 || activeIndex >= SOUND_MAXACTIVE || !Global_VDB_Ptr)
        return 0;
    if (!ActiveSounds[activeIndex].threedee)
        return 1;

    relativePosn.vx = ActiveSounds[activeIndex].threedeedata.position.vx - Global_VDB_Ptr->VDB_World.vx;
    relativePosn.vy = ActiveSounds[activeIndex].threedeedata.position.vy - Global_VDB_Ptr->VDB_World.vy;
    relativePosn.vz = ActiveSounds[activeIndex].threedeedata.position.vz - Global_VDB_Ptr->VDB_World.vz;
    distance = Magnitude(&relativePosn);
    innerRange = ActiveSounds[activeIndex].threedeedata.inner_range;
    outerRange = ActiveSounds[activeIndex].threedeedata.outer_range;

    SDL_LockMutex(AudioMutex);
    Voices[activeIndex].source_position = ActiveSounds[activeIndex].threedeedata.position;
    Voices[activeIndex].threedee = 1;

    if (ActiveSounds[activeIndex].paused)
    {
        if (distance < outerRange + SOUND_DEACTIVATERANGE)
        {
            ActiveSounds[activeIndex].paused = 0;
            Voices[activeIndex].paused = 0;
            Voices[activeIndex].position = 0.0;
            Voices[activeIndex].playing = 1;
            Voices[activeIndex].serial++;
        }
        else
        {
            SDL_UnlockMutex(AudioMutex);
            return 1;
        }
    }

    if (distance >= outerRange && ActiveSounds[activeIndex].loop)
    {
        Voices[activeIndex].playing = 0;
        Voices[activeIndex].paused = 1;
        Voices[activeIndex].position = 0.0;
        Voices[activeIndex].serial++;
        ActiveSounds[activeIndex].paused = 1;
    }
    else
    {
        Voices[activeIndex].playing = 1;
    }

    (void)innerRange;
    SDL_UnlockMutex(AudioMutex);
    return 1;
}

void PlatUpdatePlayer(void)
{
    if (!AudioMutex || !Global_VDB_Ptr)
        return;

    SDL_LockMutex(AudioMutex);
    Listener.position[0] = (float)Global_VDB_Ptr->VDB_World.vx;
    Listener.position[1] = (float)Global_VDB_Ptr->VDB_World.vy;
    Listener.position[2] = (float)Global_VDB_Ptr->VDB_World.vz;

    if (AvP.PlayerType != I_Alien)
    {
        Listener.forward[0] = (float)Global_VDB_Ptr->VDB_Mat.mat13 / 65536.0f;
        Listener.forward[1] = 0.0f;
        Listener.forward[2] = (float)Global_VDB_Ptr->VDB_Mat.mat33 / 65536.0f;
        Listener.up[0] = 0.0f;
        Listener.up[1] = 1.0f;
        Listener.up[2] = 0.0f;
    }
    else
    {
        Listener.forward[0] = (float)Global_VDB_Ptr->VDB_Mat.mat13 / 65536.0f;
        Listener.forward[1] = (float)Global_VDB_Ptr->VDB_Mat.mat23 / 65536.0f;
        Listener.forward[2] = (float)Global_VDB_Ptr->VDB_Mat.mat33 / 65536.0f;
        Listener.up[0] = (float)Global_VDB_Ptr->VDB_Mat.mat12 / 65536.0f;
        Listener.up[1] = (float)Global_VDB_Ptr->VDB_Mat.mat22 / 65536.0f;
        Listener.up[2] = (float)Global_VDB_Ptr->VDB_Mat.mat32 / 65536.0f;
    }
    SDL_UnlockMutex(AudioMutex);
}

void PlatSetEnviroment(unsigned int env_index, float reverb_mix)
{
    (void)env_index;
    (void)reverb_mix;
}

unsigned int PlatMaxHWSounds(void)
{
    return 256;
}

int PlatUse3DSoundHW(void)
{
    return 2;
}

int PlatDontUse3DSoundHW(void)
{
    return 2;
}

void PlatEndGameSound(SOUNDINDEX index)
{
    int i;
    SDLSample *sample;

    if (index < 0 || index >= SID_MAXIMUM || !AudioMutex)
        return;

    SDL_LockMutex(AudioMutex);
    sample = (SDLSample *)GameSounds[index].buffer;
    for (i = 0; i < SOUND_MAXACTIVE; ++i)
    {
        if (Voices[i].sample == sample)
        {
            Voices[i].playing = 0;
            Voices[i].paused = 0;
            Voices[i].position = 0.0;
            Voices[i].serial++;
            SetVoiceSampleLocked(&Voices[i], NULL);
        }
    }

    GameSounds[index].buffer = NULL;
    if (sample)
        RetireSampleLocked(sample);
    GameSounds[index].loaded = 0;
    GameSounds[index].dsFrequency = 0;
    if (GameSounds[index].wavName)
    {
        DeallocateMem(GameSounds[index].wavName);
        GameSounds[index].wavName = NULL;
    }
    SDL_UnlockMutex(AudioMutex);
}

void InitialiseBaseFrequency(SOUNDINDEX soundNum)
{
    if (GameSounds[soundNum].pitch > PITCH_MAXPLAT)
        GameSounds[soundNum].pitch = PITCH_MAXPLAT;
    if (GameSounds[soundNum].pitch < PITCH_MINPLAT)
        GameSounds[soundNum].pitch = PITCH_MINPLAT;
}

void UpdateSoundFrequencies(void)
{
}

int LoadWavFile(int soundNum, char *wavFileName)
{
    FILE *fp;
    long size;
    unsigned char *data;
    size_t got;
    int result;

    if (!SoundActivated || soundNum < 0 || soundNum >= SID_MAXIMUM || !wavFileName)
        return 0;

    fp = OpenGameFile(wavFileName, FILEMODE_READONLY, FILETYPE_PERM);
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    if (size <= 0)
    {
        fclose(fp);
        return 0;
    }

    data = (unsigned char *)malloc((size_t)size);
    if (!data)
    {
        fclose(fp);
        return 0;
    }

    got = fread(data, 1, (size_t)size, fp);
    fclose(fp);

    result = got == (size_t)size && LoadWAVBytes(soundNum, wavFileName, data, (size_t)size);
    free(data);
    return result;
}

unsigned char *ExtractWavFile(int soundIndex, unsigned char *bufferPtr)
{
    char *wavName;
    unsigned char *wav;
    uint32_t riffSize;
    size_t total;

    if (!SoundActivated || soundIndex < 0 || soundIndex >= SID_MAXIMUM || !bufferPtr)
        return NULL;

    wavName = (char *)bufferPtr;
    wav = bufferPtr + strlen(wavName) + 1;
    if (memcmp(wav, "RIFF", 4) != 0 || memcmp(wav + 8, "WAVE", 4) != 0)
        return NULL;

    riffSize = (uint32_t)wav[4] |
               ((uint32_t)wav[5] << 8) |
               ((uint32_t)wav[6] << 16) |
               ((uint32_t)wav[7] << 24);
    total = 8u + (size_t)riffSize;
    if (total > SIZE_MAX - (size_t)(wav - bufferPtr))
        return NULL;

    if (!LoadWAVBytes(soundIndex, wavName, wav, total))
        return NULL;
    return wav + total;
}

int LoadWavFromFastFile(int soundNum, char *wavFileName)
{
    FFILE *fp;
    size_t len;
    size_t nameLen;
    unsigned char *buf;
    int result;

    fp = ffopen(wavFileName, "rb");
    if (!fp)
        return 0;

    ffseek(fp, 0, SEEK_END);
    len = (size_t)fftell(fp);
    ffseek(fp, 0, SEEK_SET);

    nameLen = strlen(wavFileName) + 1;
    buf = (unsigned char *)malloc(nameLen + len);
    if (!buf)
    {
        ffclose(fp);
        return 0;
    }

    memcpy(buf, wavFileName, nameLen);
    result = ffread(buf + nameLen, len, 1, fp) == len;
    ffclose(fp);

    if (result)
        result = ExtractWavFile(soundNum, buf) != NULL;

    free(buf);
    return result;
}
