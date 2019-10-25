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
inline void cpixmap<int16_t>::reverseEndian(void)
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
	//pixVec = reverse_endian(pixVec);
	pixVec = rotate_left(pixVec, 8);
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec8us pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	//pixVec = reverse_endian(pixVec);
	pixVec = rotate_left(pixVec, 8);
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      uint16x8_t pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec = vld1q_u16((const uint16_t *)&pixLine[x]);
	uint16x4_t lowVec = vget_high_u16(pixVec);
	uint16x4_t highVec = vget_low_u16(pixVec);
	pixVec = vcombine_u16(lowVec, highVec);
	vst1q_u16((uint16_t *)&pixLine[x], pixVec);
      }
# endif
    }
  }
}

template <>
inline void cpixmap<uint16_t>::reverseEndian(void)
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
	//pixVec = reverse_endian(pixVec);
	pixVec = rotate_left(pixVec, 8);
	pixVec.store(&pixLine[x]);
      }
#  elif INSTRSET >= 2 // SSE2 - 128bits
      Vec8us pixVec;
      for (size_t x = 0; x < m_width; x += 8) {
	pixVec.load(&pixLine[x]);
	//pixVec = reverse_endian(pixVec);
	pixVec = roate_left(pixVec, 8);
	pixVec.store(&pixLine[x]);
      }
#  endif
# elif defined(__ARM_NEON__)
      uint16x8_t pixVec;
      for (size_t x = 0; x < m_width; x += 2) {
	pixVec = vld1q_u16((const uint16_t *)&pixLine[x]);
	uint16x4_t lowVec = vget_high_u16(pixVec);
	uint16x4_t highVec = vget_low_u16(pixVec);
	pixVec = vcombine_u16(lowVec, highVec);
	vst1q_u16((uint16_t *)&pixLine[x], pixVec);
      }
# endif
    }
  }
}
