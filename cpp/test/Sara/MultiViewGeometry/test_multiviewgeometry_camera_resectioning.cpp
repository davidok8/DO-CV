// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2018-2019 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#define BOOST_TEST_MODULE "MultiViewGeometry/Camera Resectioning"

#include <DO/Sara/Core/DebugUtilities.hpp>

#include <DO/Sara/MultiViewGeometry/Geometry/EssentialMatrix.hpp>
#include <DO/Sara/MultiViewGeometry/Geometry/PinholeCamera.hpp>
#include <DO/Sara/MultiViewGeometry/Resectioning/HartleyZisserman.hpp>
#include <DO/Sara/MultiViewGeometry/Utilities.hpp>

#include <boost/test/unit_test.hpp>


namespace sara = DO::Sara;


auto make_cube_vertices()
{
  auto cube = Eigen::MatrixXd{4, 8};
  cube.topRows(3) <<
      0, 1, 0, 1, 0, 1, 0, 1,  //
      0, 0, 1, 1, 0, 0, 1, 1,  //
      0, 0, 0, 0, 1, 1, 1, 1;  //
  cube.row(3).fill(1);

  // Recenter the cube.
  cube.topRows(3).colwise() += -0.5 * Eigen::Vector3d::Ones();

  return cube;
}

auto make_relative_motion() -> sara::Motion
{
  using namespace sara;

  const Eigen::Matrix3d R = rotation_z(0.2) * rotation_x(0.1) * rotation_y(0.3);
  // - The axes of the world coordinate system is has turned by the following
  //   rotational quantity.
  // - The columns of R are the vector coordinates of the world axes w.r.t.
  //   the camera coordinate system.

  const Eigen::Vector3d t{-2, -0.2, 10.};
  // - The vector t are the coordinates of the world center w.r.t. the camera
  //   coordinate system.

  return {R, t};
}

auto make_camera() -> sara::PinholeCamera
{
  const auto& [R, t] = make_relative_motion();
  return sara::normalized_camera(R, t);
}

auto to_camera_coordinates(const sara::PinholeCamera& C,
                          const Eigen::MatrixXd& X) -> Eigen::MatrixXd
{
  Eigen::MatrixXd X1 = (C.R * X.topRows(3)).colwise() + C.t;
  return X1.colwise().homogeneous();
}

auto project_to_film(const sara::PinholeCamera& C, const Eigen::MatrixXd& X)
    -> Eigen::MatrixXd
{
  auto x = Eigen::MatrixXd{3, X.cols()};
  x = C.matrix() * X;
  x = x.colwise().hnormalized();
  return x;
}

template <typename T, int M, int N>
inline auto tensor_view(const Eigen::Matrix<T, M, N>& m)
{
  return sara::TensorView_<T, 2>{const_cast<T*>(m.data()),
                                 {m.cols(), m.rows()}};
}


BOOST_AUTO_TEST_CASE(test_flipud)
{
  auto A = Eigen::Matrix3i{};
  A <<
    1, 2, 3,
    4, 5, 6,
    7, 8, 9;

  const auto A_flipped = sara::flipud(A);
  auto A_flipped_true = Eigen::Matrix3i{};
  A_flipped_true <<
    7, 8, 9,
    4, 5, 6,
    1, 2, 3;

  BOOST_CHECK(A_flipped_true == A_flipped);
}

BOOST_AUTO_TEST_CASE(test_fliplr)
{
  auto A = Eigen::Matrix3i{};
  A <<
    1, 2, 3,
    4, 5, 6,
    7, 8, 9;

  const auto A_flipped = sara::fliplr(A);
  auto A_flipped_true = Eigen::Matrix3i{};
  A_flipped_true <<
    3, 2, 1,
    6, 5, 4,
    9, 8, 7;
  BOOST_CHECK(A_flipped_true == A_flipped);
}

BOOST_AUTO_TEST_CASE(test_hartley_zisserman)
{
  auto Xw = make_cube_vertices();

  // Translate the cube further 10 meters away from the world center.
  Xw.row(2).array() += 10;

  std::cout << "* World Coordinates:" << std::endl;
  std::cout << "  Xw =\n" << Xw << std::endl;

  // Get the test camera matrix.
  const auto C = make_camera();

  auto Xc = to_camera_coordinates(C, Xw);
  std::cout << "* Camera Coordinates:" << std::endl;
  std::cout << "  Xc =\n" << Xc << std::endl;

  auto x = project_to_film(C, Xw);
  std::cout << "* Film Coordinates:" << std::endl;
  std::cout << "  x =\n" << x << std::endl;


  // Now check the resectioning method.
  const auto Xw1 = tensor_view(Xw);
  const auto x1 = tensor_view(x);
  const auto [K, R, t] = sara::resectioning_hartley_zisserman(Xw1, x1);
  const auto C1 = sara::PinholeCamera{K, R, t};

  std::cout << "R =\n" << C.R << std::endl;
  std::cout << "t =\n" << C.t << std::endl;

  std::cout << "K1 =\n" << K << std::endl;
  std::cout << "R1 =\n" << R << std::endl;
  std::cout << "t1 =\n" << t << std::endl;

  std::cout << "C1 - C =\n" << C1.matrix() - C.matrix() << std::endl;

  BOOST_CHECK_LE((C1.matrix() - C.matrix()).norm(), 1e-8);
}
