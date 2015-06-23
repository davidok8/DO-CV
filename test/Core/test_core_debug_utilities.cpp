// ========================================================================== //
// This file is part of DO-CV, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2015 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#include <string>

#include <gtest/gtest.h>

#include <DO/Sara/Core/DebugUtilities.hpp>


using namespace std;
using namespace DO::Sara;


struct CoutRedirect {
  CoutRedirect(std::streambuf * new_buffer)
    : old(std::cout.rdbuf(new_buffer))
  { }

  ~CoutRedirect() {
    std::cout.rdbuf(old);
  }

private:
  std::streambuf * old;
};


TEST(DO_Sara_Core_Test, test_print_stage)
{
  stringstream buffer;
  CoutRedirect cout_redirect(buffer.rdbuf());
  print_stage("Hello");
  auto text = buffer.str();

  EXPECT_NE(text.find("Hello"), string::npos);
}


int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
