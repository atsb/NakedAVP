#ifndef UNALIGNED_H
#define UNALIGNED_H

// Anything using these types is not alignment and endian clean.

#if EMSCRIPTEN
#include <emscripten.h>

typedef emscripten_align1_short unaligned_s16;
typedef emscripten_align1_int unaligned_s32;
typedef emscripten_align1_short unaligned_u16;
typedef emscripten_align1_int unaligned_u32;
typedef emscripten_align1_float unaligned_f32;
typedef emscripten_align1_double unaligned_f64;

#else

#include <stdint.h>

typedef int16_t unaligned_s16;
typedef int32_t unaligned_s32;
typedef uint16_t unaligned_u16;
typedef uint32_t unaligned_u32;
typedef float  unaligned_f32;
typedef double unaligned_f64;

#endif

#endif
