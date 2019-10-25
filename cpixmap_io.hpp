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

#include <iostream>
#include <cassert>
#include <limits>
#include <fstream>
#include <string>

#include <Magick++.h>

#include <cpixmap.hpp>
#include <chistogram.hpp>

template <typename T>
void readRawImage(std::string filename, cpixmap<T>& img, size_t z = 0)
{
  std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

  assert(file.is_open());
  //size_t imagesize = file.tellg();
  file.seekg(0, std::ios::beg);
  for (size_t i = 0; i < img.getHeight(); ++i) {
    file.read(reinterpret_cast<char *>(img.getLine(i, z)), img.getWidth()*sizeof(T));
  }
  file.close();
}

template <typename T>
void writeRawImage(cpixmap<T>& img, size_t z, std::string filename)
{
  std::ofstream file(filename.c_str(), std::ofstream::binary);

  assert(file.is_open());
  for (size_t i = 0; i < img.getHeight(); ++i) {
    file.write(reinterpret_cast<char *>(img.getLine(i, z)), img.getWidth()*sizeof(T));
  }
  file.close();
}

template <typename T>
void readImage(std::string filename, cpixmap<T>& img, size_t z = 0)
{
  Magick::Image image;
  
  image.read(filename.c_str());
  //image.display();

  img.setResolution(image.columns(), image.rows());
  
  Magick::PixelPacket *pixels;

  pixels = image.getPixels(0, 0, image.columns(), image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < image.rows(); ++i) {
    for (ssize_t j = 0; j < image.columns(); ++j) {
      Magick::ColorGray gray;
      gray = pixels[i*image.columns() + j];
      img.putPixel((T)(gray.shade() * ((double)std::numeric_limits<T>::max() - (double)std::numeric_limits<T>::min())) + std::numeric_limits<T>::min(), j, i, z);
    }
  }
}

template <typename T>
void readRGBImage(std::string filename, cpixmap<T>& img)
{
  Magick::Image image;
  
  image.read(filename.c_str());
  //image.display();

  img.setResolution(image.columns(), image.rows(), 3);
  
  Magick::PixelPacket *pixels;

  pixels = image.getPixels(0, 0, image.columns(), image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < image.rows(); ++i) {
    for (ssize_t j = 0; j < image.columns(); ++j) {
      Magick::ColorRGB color;
      color = pixels[i*image.columns() + j];
      img.putPixel((T)(color.red() * ((double)std::numeric_limits<T>::max() - (double)std::numeric_limits<T>::min()) + std::numeric_limits<T>::min()), j, i, cpixmap<T>::RED_BAND);
      img.putPixel((T)(color.green() * ((double)std::numeric_limits<T>::max() - (double)std::numeric_limits<T>::min()) + std::numeric_limits<T>::min()), j, i, cpixmap<T>::GREEN_BAND);
      img.putPixel((T)(color.blue() * ((double)std::numeric_limits<T>::max() - (double)std::numeric_limits<T>::min()) + std::numeric_limits<T>::min()), j, i, cpixmap<T>::BLUE_BAND);
      //img.putPixel((T)(gray.shade() * ((double)std::numeric_limits<T>::max() - (double)std::numeric_limits<T>::min())) + std::numeric_limits<T>::min(), j, i, z);
    }
  }
}

template <typename T>
void displayRGBPixmap(cpixmap<T>& img, bool do_scale = false)
{
  T minval, maxval, val;

  assert(img.getBands() >= cpixmap<T>::RGB_BANDS);

  if (do_scale) {
    minval = std::numeric_limits<T>::max();
    maxval = std::numeric_limits<T>::min();
    for (size_t i = 0; i < img.getHeight(); ++i) {
      for (size_t j = 0; j < img.getWidth(); ++j) {
	val = img.getPixel(j, i, cpixmap<T>::RED_BAND); // Red
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
	
	val = img.getPixel(j, i, cpixmap<T>::GREEN_BAND); // Green
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
	
	val = img.getPixel(j, i, cpixmap<T>::BLUE_BAND); // Blue
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
      }
    }
  } else {
    minval = std::numeric_limits<T>::min();
    maxval = std::numeric_limits<T>::max();
  }

  Magick::Image disp_image(Magick::Geometry(img.getWidth(), img.getHeight()), "black");
  disp_image.classType(Magick::DirectClass);
  disp_image.modifyImage();
  
  Magick::Pixels view(disp_image);
  Magick::PixelPacket *pixels;

  pixels = view.get(0, 0, disp_image.columns(), disp_image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < disp_image.rows(); ++i) {
    for (ssize_t j = 0; j < disp_image.columns(); ++j) {

      Magick::ColorRGB color((double)((int)img.getPixel(j, i, cpixmap<T>::RED_BAND) - (int)minval) /
			     (double)((int)maxval - (int)minval),
			     (double)((int)img.getPixel(j, i, cpixmap<T>::GREEN_BAND) - (int)minval) /
			     (double)((int)maxval - (int)minval),
			     (double)((int)img.getPixel(j, i, cpixmap<T>::BLUE_BAND) - (int)minval) /
			     (double)((int)maxval - (int)minval));
      /*
      Magick::ColorRGB color((double)img.getPixel(j, i, 0)/std::numeric_limits<T>::max(),
			     (double)img.getPixel(j, i, 1)/std::numeric_limits<T>::max(),
			     (double)img.getPixel(j, i, 2)/std::numeric_limits<T>::max());
      */
      pixels[i*disp_image.columns() + j] = color;
    }
  }
  view.sync();
  //disp_image.write("test_g.pgm");
  disp_image.display();
}

template <typename T>
void displayRGBPixmap(cpixmap<T>& rimg, cpixmap<T>& gimg, cpixmap<T>& bimg, bool do_scale = false)
{
  assert(rimg.isMatched(gimg));
  assert(gimg.isMatched(bimg));

  T minval, maxval, val;

  if (do_scale) {
    minval = std::numeric_limits<T>::max();
    maxval = std::numeric_limits<T>::min();
    for (size_t i = 0; i < rimg.getHeight(); ++i) {
      for (size_t j = 0; j < rimg.getWidth(); ++j) {
	val = rimg.getPixel(j, i);
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
	
	val = gimg.getPixel(j, i);
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
	
	val = bimg.getPixel(j, i);
	if (val < minval) minval = val;
	else if (val > maxval) maxval = val;
      }
    }
  } else {
    minval = std::numeric_limits<T>::min();
    maxval = std::numeric_limits<T>::max();
  }
  
  Magick::Image disp_image(Magick::Geometry(rimg.getWidth(), rimg.getHeight()), "black");
  disp_image.classType(Magick::DirectClass);
  disp_image.modifyImage();
  
  Magick::Pixels view(disp_image);
  Magick::PixelPacket *pixels;

  pixels = view.get(0, 0, disp_image.columns(), disp_image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < disp_image.rows(); ++i) {
    for (ssize_t j = 0; j < disp_image.columns(); ++j) {
      Magick::ColorRGB color((double)((int)rimg.getPixel(j, i, 0) - (int)minval) /
			     (double)((int)maxval - (int)minval),
			     (double)((int)gimg.getPixel(j, i, 0) - (int)minval) /
			     (double)((int)maxval - (int)minval),
			     (double)((int)bimg.getPixel(j, i, 0) - (int)minval) /
			     (double)((int)maxval - (int)minval));
      pixels[i*disp_image.columns() + j] = color;
    }
  }
  view.sync();
  //disp_image.write("test_g.pgm");
  disp_image.display();
}

template <typename T>
void displayPixmap(cpixmap<T>& img, size_t band = 0, bool do_scale = false)
{
  Magick::Image disp_image(Magick::Geometry(img.getWidth(), img.getHeight()), "black");
  disp_image.classType(Magick::DirectClass);
  disp_image.modifyImage();
  
  Magick::Pixels view(disp_image);
  
  Magick::PixelPacket *pixels;

  double minval, maxval;
  if (do_scale) {
    minval = maxval = static_cast<double>(img.getPixel(0, 0, band));
    for (size_t i = 0; i < img.getHeight(); ++i) {
      for (size_t j = 0; j < img.getWidth(); ++j) {
	if (img.getPixel(j, i, band) < static_cast<T>(minval))
	  minval = static_cast<double>(img.getPixel(j, i, band));
	if (img.getPixel(j, i, band) > static_cast<T>(maxval))
	  maxval = static_cast<double>(img.getPixel(j, i, band));
      }
    }
  } else {
    minval = (double)std::numeric_limits<T>::min();
    maxval = (double)std::numeric_limits<T>::max();
  }
  
  pixels = view.get(0, 0, disp_image.columns(), disp_image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < disp_image.rows(); ++i) {
    for (ssize_t j = 0; j < disp_image.columns(); ++j) {
      double value = (double)img.getPixel(j, i, band);
      Magick::ColorGray gray;
      gray.shade((value - minval) / (maxval - minval));
      pixels[i*disp_image.columns() + j] = gray;
    }
  }
  view.sync();
  //disp_image.write("test_g.pgm");
  disp_image.display();
}

template <typename T>
void writePixmap(cpixmap<T>& img, int band, std::string filename)
{
  Magick::Image write_image(Magick::Geometry(img.getWidth(), img.getHeight()), "black");
  write_image.classType(Magick::DirectClass);
  write_image.modifyImage();
  
  Magick::Pixels view(write_image);
  
  Magick::PixelPacket *pixels;
  //Magick::ColorGray gray;

  double minval = (double)std::numeric_limits<T>::min();
  double maxval = (double)std::numeric_limits<T>::max();
  
  pixels = view.get(0, 0, write_image.columns(), write_image.rows());
#pragma omp parallel for
  for (ssize_t i = 0; i < write_image.rows(); ++i) {
    for (ssize_t j = 0; j < write_image.columns(); ++j) {
      Magick::ColorGray gray(((double)img.getPixel(j, i, band) - minval) / (maxval - minval));
      pixels[i*write_image.columns() + j] = gray;
    }
  }
  view.sync();
  write_image.write(filename.c_str());
  //write_image.display();
}

template <typename T>
void copyPixmap(cpixmap<T>& dst, size_t xoff, size_t yoff, cpixmap<T>& src, size_t z = 0)
{
  size_t height = std::min(src.getHeight(), dst.getHeight()+yoff);
  size_t width = std::min(src.getWidth(), dst.getWidth()+xoff);

#pragma omp parallel for
  for (size_t y = 0; y < height; ++y) {
    T *dstline = dst.getLine(y+yoff, z);
    T *srcline = src.getLine(y, z);
    std::memcpy(dstline+xoff, srcline, width*sizeof(T));
  }
}

template <typename T>
void condensePixmap(cpixmap<T>& img, T minvalue, T maxvalue)
{
  for (size_t k = 0; k < img.getBands(); ++k) {
    for (size_t j = 0; j < img.getHeight(); ++j) {
      for (size_t i = 0; i < img.getWidth(); ++i) {
	T value = img.getPixel(i, j, k);
	value = (T)((float)value * ((float)maxvalue - (float)minvalue) / (float)std::numeric_limits<T>::max() + (float)minvalue);
	img.putPixel(value, i, j, k);
      }
    }
  }
}

template <typename T>
void comparePixmap(cpixmap<T>& img1, cpixmap<T>& img2)
{
  double manhattan_norm = 0.0l;
  double zero_norm = 0.0l;
  cpixmap<unsigned int> diffimg(img1);
  //chistogram_bins hbins;

  //hbins.setRange(-100, 100);
  //hbins.clearBins();
  for (size_t i = 0; i < img1.getHeight(); ++i) {
    for (size_t j = 0; j < img1.getWidth(); ++j) {
      //int32_t diff = (int32_t)img1(i, j) - (int32_t)img2(i, j);
      int diff = (int)img1(i, j) - (int)img2(i, j);
      diffimg(i, j) = static_cast<unsigned int>(std::abs(diff));
      //std::cout << diff;
      //hbins.accumulate(std::abs(diff));
      manhattan_norm += std::abs(diff);
      if (diff != 0) zero_norm += 1.0l;
    }
  }
  
  //hbins.dump();
  /*
  std::cout << "the number of kinds:" << hbins.getCardinality() << std::endl;
  std::cout << "Min diff:" << hbins.getMinArg() << ", Max diff:" << hbins.getMaxArg() << std::endl;
  std::cout << "Median diff:" << hbins.getMedianArg() << ", Mean diff:" << hbins.getMeanArg() << std::endl;
  std::cout << "Peak diff:" << hbins.getPeakArg() << std::endl;
  */
  std::cout << "L1 norm(Manhattan norm):" << manhattan_norm << ", per pixel:" << manhattan_norm/(img1.getHeight()*img1.getWidth()) << std::endl;
  std::cout << "L0 norm(Zero norm):" << zero_norm << ", per pixel:" << zero_norm/(img1.getHeight()*img1.getWidth()) << std::endl;

  displayPixmap(diffimg, 0, true);
}

template <typename T>
void statisticPixmap(cpixmap<T>& img)
{
  chistogram_bins<T> hbins;

  hbins.setRange(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
  hbins.clearBins();

  for (size_t i = 0; i < img.getHeight(); ++i)
    for (size_t j = 0; j < img.getWidth(); ++j)
      hbins.accumulate(img(i, j));

  hbins.dump();
}

template <typename T>
void trimPixmap(cpixmap<T>& img, int min_limit, int max_limit)
{
#pragma omp parallel for
  for (size_t i = 0; i < img.getHeight(); ++i)
    for (size_t j = 0; j < img.getWidth(); ++j)
      if ((int)img(i, j) > max_limit) img(i, j) = static_cast<T>(max_limit);
      else if ((int)img(i, j) < min_limit) img(i, j) = static_cast<T>(min_limit);
}
