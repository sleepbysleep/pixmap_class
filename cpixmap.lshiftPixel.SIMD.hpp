/*
  Copyright (C) 2017 Hoyoung Lee

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <cassert>
#include <iostream>
#include <cstring>
#include <cstdint>

#include <cpixmap.hpp>

#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
# define MAX_VECTOR_SIZE 512
# include <vectorclass/vectorclass.h>
# if INSTRSET < 2
#  error "Unsupported x86-SIMD! Please comment USE_SIMD on!"
# endif
#elif defined(__GNUC__) && defined (__ARM_NEON__)
# include <arm_neon.h>
#else
# error "Undefined SIMD!"
#endif

template <>
inline void cpixmap<int8_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      int8_t *pixLine = (int8_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 8 // AVX2 - 256bits
      Vec32c pixVec;
      for (size_t x = 0; x < m_width; x += 32) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec16c pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      int8x16_t pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec = vld1q_s8((const int8_t *)(&pixLine[x]));
	pixVec = vshlq_n_s8(pixVec, bits);
	vst1q_s8((int8_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<uint8_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      uint8_t *pixLine = (uint8_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 8 // AVX2 - 256bits
      Vec32uc pixVec;
      for (size_t x = 0; x < m_width; x += 32) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec16uc pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      uint8x16_t pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec = vld1q_u8((const uint8_t *)(&pixLine[x]));
	pixVec = vshlq_n_u8(pixVec, bits);
	vst1q_u8((uint8_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<int16_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      int16_t *pixLine = (int16_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 8 // AVX2 - 256bits
      Vec16s pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec8s pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      int16x8_t pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec = vld1q_s16((const int16_t *)(&pixLine[x]));
	pixVec = vshlq_n_s16(pixVec, bits);
	vst1q_s16((int16_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }      
}

template <>
inline void cpixmap<uint16_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      uint16_t *pixLine = (uint16_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 8 // AVX2 - 256bits
      Vec16us pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec8us pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      uint16x8_t pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec = vld1q_u16((const uint16_t *)(&pixLine[x]));
	pixVec = vshlq_n_u16(pixVec, bits);
	vst1q_u16((uint16_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<int32_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      int32_t *pixLine = (int32_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 9 // AVX512 - 512bits
      Vec16i pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 8 // AVX2 - 256bits
      Vec8i pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec4i pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      int32x4_t pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec = vld1q_s32((const int32_t *)(&pixLine[x]));
	pixVec = vshlq_n_s32(pixVec, bits);
	vst1q_s32((int32_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<uint32_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      uint32_t *pixLine = (uint32_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 9 // AVX512 - 512bits
      Vec16ui pixVec;
      for (size_t x = 0; x < m_width; x += 16) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 8 // AVX2 - 256bits
      Vec8ui pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec4ui pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEO__)
      uint32x4_t pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec = vld1q_u32((const uint32_t *)(&pixLine[x]));
	pixVec = vshlq_n_u32(pixVec, bits);
	vst1q_u32((uint32_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<int64_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      int64_t *pixLine = (int64_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 9 // AVX512 - 512bits
      Vec8q pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 8 // AVX2 - 256bits
      Vec4q pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec2q pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      int64x2_t pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec = vld1q_s64((const int64_t *)(&pixLine[x]));
	pixVec = vshlq_n_s64(pixVec, bits);
	vst1q_s64((int64_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<uint64_t>::lshiftPixel(size_t bits)
{
  for (size_t z = 0; z < m_bands; ++z) {
#pragma omp parallel for
    for (size_t y = 0; y < m_height; ++y) {
      uint64_t *pixLine = (uint64_t *)(m_buffer + z*m_band_stride + y*m_height_stride);
# if defined(__x86_64__) || defined(__i386__)
#  if INSTRSET >= 9 // AVX512 - 512bits
      Vec8uq pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 8 // AVX2 - 256bits
      Vec4uq pixVec;
      for (size_t x = 0; x < m_width; x += 4) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec2uq pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec.load(&pixLine[x]);
	pixVec <<= bits;
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      uint64x2_t pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec = vld1q_u64((const uint64_t *)(&pixLine[x]));
	pixVec = vshlq_n_u64(pixVec, bits);
	vst1q_u64((uint64_t *)(&pixLine[x]), pixVec);
      }
# endif
    }
  }
}
