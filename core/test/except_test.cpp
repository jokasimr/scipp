/// @file
/// SPDX-License-Identifier: GPL-3.0-or-later
/// @author Simon Heybrock
/// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
/// National Laboratory, and European Spallation Source ERIC.
#include <gtest/gtest.h>

#include <type_traits>

#include "dataset.h"
#include "dimensions.h"
#include "except.h"

using namespace scipp::core;

TEST(DimensionMismatchError, what) {
  Dimensions dims{{Dim::X, 1}, {Dim::Y, 2}};
  except::DimensionMismatchError error(dims, Dimensions{});
  EXPECT_EQ(
      error.what(),
      std::string("Expected dimensions {{Dim::X, 1}, {Dim::Y, 2}}, got {}."));
}

TEST(DimensionNotFoundError, what) {
  Dimensions dims{{Dim::X, 1}, {Dim::Y, 2}};
  except::DimensionNotFoundError error(dims, Dim::Z);
  EXPECT_EQ(error.what(), std::string("Expected dimension to be in {{Dim::X, "
                                      "1}, {Dim::Y, 2}}, got Dim::Z."));
}

TEST(DimensionLengthError, what) {
  Dimensions dims{{Dim::X, 1}, {Dim::Y, 2}};
  except::DimensionLengthError error(dims, Dim::Y, 3);
  EXPECT_EQ(error.what(),
            std::string("Expected dimension to be in {{Dim::X, 1}, {Dim::Y, "
                        "2}}, got Dim::Y with mismatching length 3."));
}

TEST(Dimensions, to_string) {
  Dataset a;
  a.insert(Attr::ExperimentLog, "log", Dimensions{{Dim::X, 2}});
  a.insert(Data::Value, "values", Dimensions{{Dim::X, 2}}, {1, 2});
  a.insert(Coord::X, Dimensions{{Dim::X, 3}}, {1, 2, 3});
  // Create new dataset with same variables but different order
  Dataset b;
  b.insert(a[1]);
  b.insert(a[2]);
  b.insert(a[0]);
  // string representations should be the same
  EXPECT_EQ(to_string(a), to_string(b));
}
