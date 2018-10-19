// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2018 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

//! @file

#pragma once

#include <DO/Sara/Core/MultiArray/MultiArrayView.hpp>


namespace DO { namespace Sara {

  template <typename T>
  class ConstantPadding
  {
  public:
    inline ConstantPadding(T value)
      : _value{value}
    {
    }

    template <int N, int O>
    inline auto at(const MultiArrayView<T, N, O>& f,
                   const Matrix<int, N, 1>& x) const -> T
    {
      if (x.minCoeff() < 0 || (x - f.sizes()).maxCoeff() >= 0)
        return _value;

      return f(x);
    }

  private:
    T _value;
  };

  template <typename T>
  inline auto make_constant_padding(T&& value) -> ConstantPadding<T>
  {
    return {value};
  }


  class RepeatPadding
  {
  public:
    inline RepeatPadding() = default;

    template <typename T, int N, int O>
    inline auto at(const MultiArrayView<T, N, O>& f,
                   const Matrix<int, N, 1>& x) const -> T
    {
      Matrix<int, N, 1> y = x.cwiseMax(Matrix<int, N, 1>::Zero())
                                .cwiseMin((f.sizes().array() - 1).matrix());
      return f(y);
    }
  };


  class PeriodicPadding
  {
  public:
    inline PeriodicPadding() = default;

    template <typename T, int N, int O>
    inline auto at(const MultiArrayView<T, N, O>& f,
                   const Matrix<int, N, 1>& x) const -> T
    {
      auto y = x;

      // First pass.
      // Find the equivalent coordinate between [-2 * li, 2 * li[.
      for (auto i = 0; i < N; ++i)
      {
        const auto li = f.size(i);
        static_assert(std::is_same<decltype(li), const int>::value, "");

        if (x[i] >= 0)
          y[i] = x[i] % (2 * li);
        else
          y[i] = -(-x[i] % (2 * li));
      }

      // Second pass.
      // Find the equivalent coordinate between [0, li[.
      for (auto i = 0; i < N; ++i)
      {
        const auto li = f.size(i);

        if (0 <= y[i] && y[i] < li)
          continue;

        else if (y[i] >= li)
          y[i] = 2 * li - y[i] - 1;

        else if (-li <= y[i] && y[i] < 0)
          y[i] = -y[i] - 1;

        else if (y[i] < -li)
          y[i] = y[i] + 2 * li;
      }

      return f(y);
    }
  };

} /* namespace Sara */
} /* namespace DO */
