// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2020-present David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#include <drafts/Halide/Components/Differential.hpp>


namespace v2 {

  using namespace Halide;

  template <typename T>
  class ForwardDifference : public Generator<ForwardDifference<T>>
  {
  public:
    using Base = Generator<ForwardDifference<T>>;
    using Base::get_target;

    template <typename T2>
    using Input = typename Base::template Input<T2>;

    template <typename T2>
    using Output = typename Base::template Output<T2>;

    GeneratorParam<int> tile_x{"tile_x", 16};
    GeneratorParam<int> tile_y{"tile_y", 16};

    Input<Buffer<T>> input{"input", 4};
    Input<std::int32_t> axis{"axis"};
    Output<Buffer<T>> output{"output", 4};

    //! 't' as time (since 'c' as channel does not make much sense).
    Var x{"x"}, y{"y"}, t{"t"}, n{"n"};
    Var xo{"xo"}, yo{"yo"}, to{"co"};
    Var xi{"xi"}, yi{"yi"}, ti{"ci"};

    void generate()
    {
      using DO::Shakti::HalideBackend::gradient;
      using DO::Shakti::HalideBackend::norm;

      auto input_padded = Halide::BoundaryConditions::repeat_edge(input);

      const auto dx = select(axis == 0, 1, 0);
      const auto dy = select(axis == 1, 1, 0);
      const auto dt = select(axis == 2, 1, 0);
      const auto dn = select(axis == 3, 1, 0);

      output(x, y, t, n) = input_padded(x + dx, y + dy, t + dt, n + dn) -
                           input_padded(x, y, t, n);
    }

    void schedule()
    {
      // GPU schedule.
      if (get_target().has_gpu_feature())
      {
        output.gpu_tile(x, y, t, xo, yo, to, xi, yi, ti, tile_x, tile_y, 1,
                        TailStrategy::GuardWithIf);
      }

      // Hexagon schedule.
      else if (get_target().features_any_of({Target::HVX_64, Target::HVX_128}))
      {
        const auto vector_size =
            get_target().has_feature(Target::HVX_128) ? 128 : 64;

        output.hexagon()
            .split(y, yo, yi, 128)
            .parallel(yo)
            .vectorize(x, vector_size, TailStrategy::GuardWithIf);
      }

      // CPU schedule.
      else
      {
        output.split(y, yo, yi, 8)
            .parallel(yo)
            .vectorize(x, 8, TailStrategy::GuardWithIf);
      }
    }
  };

  template <typename T>
  class PolarGradient2D : public Generator<PolarGradient2D<T>>
  {
  public:
    using Base = Generator<PolarGradient2D<T>>;
    using Base::get_target;

    template <typename T2>
    using Input = typename Base::template Input<T2>;

    template <typename T2>
    using Output = typename Base::template Output<T2>;

    GeneratorParam<int> tile_x{"tile_x", 16};
    GeneratorParam<int> tile_y{"tile_y", 16};

    Input<Buffer<T>> input{"input", 4};
    Output<Buffer<>> output{"output", {Float(32), Float(32)}, 4};

    //! 't' as time (since 'c' as channel does not make much sense).
    Var x{"x"}, y{"y"}, t{"c"}, n{"n"};
    Var xo{"xo"}, yo{"yo"}, to{"co"};
    Var xi{"xi"}, yi{"yi"}, ti{"ci"};

    void generate()
    {
      using DO::Shakti::HalideBackend::gradient;
      using DO::Shakti::HalideBackend::norm;

      auto input_padded = Halide::BoundaryConditions::repeat_edge(input);
      auto g = gradient(input_padded, x, y, t, n);
      auto mag = norm(g);
      auto ori = Halide::atan2(g(1), g(0));
      output(x, y, t, n) = {mag, ori};
    }

    void schedule()
    {
      // GPU schedule.
      if (get_target().has_gpu_feature())
      {
        output.gpu_tile(x, y, t, xo, yo, to, xi, yi, ti, tile_x, tile_y, 1,
                        TailStrategy::GuardWithIf);
      }

      // Hexagon schedule.
      else if (get_target().features_any_of({Target::HVX_64, Target::HVX_128}))
      {
        const auto vector_size =
            get_target().has_feature(Target::HVX_128) ? 128 : 64;

        output.hexagon()
            .split(y, yo, yi, 128)
            .parallel(yo)
            .vectorize(x, vector_size, TailStrategy::GuardWithIf);
      }

      // CPU schedule.
      else
      {
        output.split(y, yo, yi, 8)
            .parallel(yo)
            .vectorize(x, 8, TailStrategy::GuardWithIf);
      }
    }
  };

}  // namespace v2


HALIDE_REGISTER_GENERATOR(v2::PolarGradient2D<float>,
                          shakti_polar_gradient_2d_32f_v2)
HALIDE_REGISTER_GENERATOR(v2::ForwardDifference<float>,
                          shakti_forward_difference_32f)
