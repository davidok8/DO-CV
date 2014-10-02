// ========================================================================== //
// This file is part of DO++, a basic set of libraries in C++ for computer 
// vision.
//
// Copyright (C) 2013 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public 
// License v. 2.0. If a copy of the MPL was not distributed with this file, 
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

//! @file

#ifndef DO_IMAGEPROCESSING_INTERPOLATION_HPP
#define DO_IMAGEPROCESSING_INTERPOLATION_HPP


#include <stdexcept>

#include <DO/Core/Image.hpp>
#include <DO/Core/Pixel/PixelTraits.hpp>
#include <DO/Core/DebugUtilities.hpp>


namespace DO {

  /*!
    \ingroup ImageProcessing
    \defgroup Interpolation Interpolation
    @{
   */

  // ====================================================================== //
  // Interpolation
  //! \brief Interpolation function
  template <typename T, int N>
  typename PixelTraits<T>::template Cast<double>::pixel_type
  interpolate(const Image<T, N>& image, const Matrix<double, N, 1>& pos)
  {
    // Typedefs.
    typedef typename PixelTraits<T>::template Cast<double>::pixel_type
      pixel_type;
    typedef typename Image<T, N>::const_subarray_iterator
      const_subarray_iterator;

    // Find the smallest integral bounding box that encloses 'pos'.
    Matrix<int, N, 1> start, end;
    Matrix<double, N, 1> frac;
    for (int i = 0; i < N; ++i)
    {
      if (pos[i] < 0 || pos[i] >= image.size(i))
        throw std::range_error("Cannot interpolate: position is out of range");

      double ith_int_part;
      frac[i] = std::modf(pos[i], &ith_int_part);
      start[i] = static_cast<int>(ith_int_part);
    }
    end.array() = start.array() + 2;

    // Compute the weighted sum.
    const_subarray_iterator it(image.begin_subarray(start, end));
    pixel_type interpolated_value(color_min_value<pixel_type>());
    Matrix<int, N, 1> offset;
    for ( ; !it.end(); ++it)
    {
      double weight = 1.;
      for (int i = 0; i < N; ++i)
      {
        weight *= (it.position()[i] == start[i]) ? (1.-frac[i]) : frac[i];
        offset[i] = it.position()[i] < image.size(i) ? 0 : -1;
      }

      pixel_type color;
      convert_channel(it(offset), color);
      interpolated_value += weight*color;
    }
    return interpolated_value;
  }

}


#endif /* DO_IMAGEPROCESSING_INTERPOLATION_HPP */