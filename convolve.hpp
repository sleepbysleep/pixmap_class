/*
  Copyright (C) 2014 Hoyoung Lee

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
#include <limits>
#include <iostream>
#include <float.h>

#include <cpixmap.hpp>

template <typename T>
void convolve(cpixmap<T>& dst, cpixmap<T>& src, cpixmap<int>& kernel, int rshift = 0, int offset = 0)
{
  assert(dst.isMatched(src));
  assert(std::numeric_limits<T>::is_integer);
  assert(std::numeric_limits<T>::digits < std::numeric_limits<int>::digits);

  int roff = (kernel.getWidth()>>1) + 1; // positive value
  int loff = roff - kernel.getWidth(); // negative value
  
  int doff = (kernel.getHeight()>>1) + 1; // positive value
  int uoff = doff - kernel.getHeight(); // negative value

  int minval = std::numeric_limits<T>::lowest();
  int maxval = std::numeric_limits<T>::max();

  bool do_scale = (rshift != 0) || (offset != 0);
  bool do_clip = (minval != std::numeric_limits<int>::lowest()) || (maxval != std::numeric_limits<int>::max());
  
  for (int z = 0; z < src.getBands(); z++) {
#pragma omp parallel for
    for (int y = 0; y < src.getHeight(); y++) {
      T *dstline = dst.getLine(y, z);
      //float *outline = output.getLine(y, z);
      for (int x = 0; x < src.getWidth(); x++) {
	int sum = 0;
	for (int j = std::max(uoff, -y); j < std::min(doff, src.getHeight()-y); j++) {
	  int *kline = kernel.getLine(j-uoff, 0);
	  T *srcline = src.getLine(y+j, z);
	  for (int i = std::max(loff, -x); i < std::min(roff, src.getWidth()-x); i++)
	    sum += *(kline + (i-loff)) * (int)(*(srcline + (x+i)));
	}
	//output.putPixel(sum, x, y, z);
	if (do_scale) sum = (sum>>rshift) + offset;
	if (do_clip) *(dstline + x) = std::min(std::max(sum, minval), maxval);
	else *(dstline + x) = sum;
      }
    }
  }
}

#if 0
//FIXME: cpixmap<float> to another class for float
template <typename T>
void convolve(cpixmap<T>& dst, cpixmap<T>& src, cpixmap<float>& kernel, float scale = 1.0, float offset = 0)
{
  assert(dst.isMatched(src));
  assert(std::numeric_limits<T>::is_integer);

  int roff = (kernel.getWidth()>>1) + 1; // positive value
  int loff = roff - kernel.getWidth(); // negative value

  int doff = (kernel.getHeight()>>1) + 1; // positive value
  int uoff = doff - kernel.getHeight(); // negative value

  float minval = std::numeric_limits<T>::lowest();
  float maxval = std::numeric_limits<T>::max();

  bool do_scale = (scale != 1.0f) || (offset != 0.0f);
  bool do_clip = (minval != std::numeric_limits<float>::lowest()) || (maxval != std::numeric_limits<float>::max());
  
  for (int z = 0; z < src.getBands(); z++) {
#pragma omp parallel for
    for (int y = 0; y < src.getHeight(); y++) {
      T *dstline = dst.getLine(y, z);
      for (int x = 0; x < src.getWidth(); x++) {
	float sum = 0.0f;
	for (int j = std::max(uoff, -y); j < std::min(doff, src.getHeight()-y); j++) {
	  T *srcline = src.getLine(y+j, z);
	  float *kline = kernel.getLine(j-uoff, 0);
	  for (int i = std::max(loff, -x); i < std::min(roff, src.getWidth()-x); i++)
	    sum += (float)*(srcline + (x+i)) * *(kline + (i-loff));
	}
	//output.putPixel(sum, x, y, z);
	if (do_scale) sum = sum * scale + offset;
	if (do_clip) *(dstline + x) = std::min(std::max(sum, minval), maxval);
	else *(dstline + x) = sum;
      }
    }
  }
}

#endif

template <typename T>
void convolveXYSeperately(cpixmap<T>& dst, cpixmap<T>& src,
			  cpixmap<int>& xkernel, cpixmap<int>& ykernel,
			  int rshift = 0, int offset = 0)
{
  assert(dst.isMatched(src));
  assert(xkernel.getWidth() > 1 && xkernel.getHeight() == 1);
  assert(ykernel.getWidth() > 1 && ykernel.getHeight() == 1);

  cpixmap<T> temp(src);
  cpixmap<int> vkernel(1, ykernel.getWidth(), 1);

  // transpose ykernel into vkernel.
  for (int i = 0; i < ykernel.getWidth(); i++)
    vkernel.putPixel(ykernel.getPixel(i, 0), 0, i);

  convolve(temp, src, xkernel, 0, 0);
  convolve(dst, temp, vkernel, rshift, offset);
}

/*
FIXME: cpixmap<float> to another class for supporting float
template <typename T>
void convolveXYSeperately(cpixmap<T>& dst, cpixmap<T>& src,
			  cpixmap<float>& xkernel, cpixmap<float>& ykernel,
			  float scale = 1.0, float offset = 0.0)
{
  assert(dst.isMatched(src));
  assert(xkernel.getWidth() > 1 && xkernel.getHeight() == 1);
  assert(ykernel.getWidth() > 1 && ykernel.getHeight() == 1);
  
  cpixmap<T> temp(src);
  cpixmap<float> vkernel(1, ykernel.getWidth(), 1);
  
  // transpose ykernel into vkernel.
  for (int i = 0; i < ykernel.getWidth(); i++)
    vkernel.putPixel(ykernel.getPixel(i, 0), 0, i);

  convolve(temp, src, xkernel, 1.0f, 0.0f);
  convolve(dst, temp, vkernel, scale, offset);
}
*/
