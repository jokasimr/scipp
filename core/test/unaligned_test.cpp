// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/core/dataset.h"
#include "scipp/core/unaligned.h"

using namespace scipp;
using namespace scipp::core;

TEST(UnalignedTest, align) {
  const Dim dim = Dim::Position;
  const auto pos = makeVariable<Eigen::Vector3d>(
      Dims{dim}, Shape{4},
      Values{Eigen::Vector3d{1, 1, 1}, Eigen::Vector3d{1, 1, 2},
             Eigen::Vector3d{1, 2, 3}, Eigen::Vector3d{1, 2, 4}});
  const auto x = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 1, 1, 1});
  const auto y = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 1, 2, 2});
  const auto z = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 2, 3, 4});
  DataArray base(makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 2, 3, 4}),
                 {{dim, pos}, {Dim::X, x}, {Dim::Y, y}, {Dim::Z, z}});

  base = concatenate(base, base + base, Dim::Temperature);
  EXPECT_EQ(base.dims(), Dimensions({Dim::Temperature, Dim::Position}, {2, 4}));
  const auto temp = makeVariable<double>(Dims{Dim::Temperature}, Shape{2});
  base.coords().set(Dim::Temperature, temp);

  const auto xbins = makeVariable<double>(Dims{Dim::X}, Shape{2}, Values{0, 4});
  const auto ybins = makeVariable<double>(Dims{Dim::Y}, Shape{2}, Values{0, 4});
  const auto zbins = makeVariable<double>(Dims{Dim::Z}, Shape{2}, Values{0, 4});

  auto realigned = unaligned::realign(
      base, {{Dim::Z, zbins}, {Dim::Y, ybins}, {Dim::X, xbins}});

  EXPECT_FALSE(realigned.hasData());
  EXPECT_EQ(
      realigned.dims(),
      Dimensions({Dim::Temperature, Dim::Z, Dim::Y, Dim::X}, {2, 1, 1, 1}));
  EXPECT_TRUE(realigned.coords().contains(Dim::Temperature));
  EXPECT_TRUE(realigned.coords().contains(Dim::X));
  EXPECT_TRUE(realigned.coords().contains(Dim::Y));
  EXPECT_TRUE(realigned.coords().contains(Dim::Z));
  EXPECT_EQ(realigned.coords()[Dim::Temperature], temp);
  EXPECT_EQ(realigned.coords()[Dim::X], xbins);
  EXPECT_EQ(realigned.coords()[Dim::Y], ybins);
  EXPECT_EQ(realigned.coords()[Dim::Z], zbins);

  EXPECT_TRUE(realigned.unaligned().hasData());
  EXPECT_EQ(realigned.unaligned(), base);
}

namespace {
auto make_array() {
  const Dim dim = Dim::Position;
  const auto pos = makeVariable<Eigen::Vector3d>(
      Dims{dim}, Shape{4},
      Values{Eigen::Vector3d{1, 1, 1}, Eigen::Vector3d{1, 1, 2},
             Eigen::Vector3d{1, 2, 3}, Eigen::Vector3d{1, 2, 4}});
  const auto x = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 1, 1, 1});
  const auto y = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 1, 2, 2});
  const auto z = makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 2, 3, 4});
  DataArray a(makeVariable<double>(Dims{dim}, Shape{4}, Values{1, 2, 3, 4}),
              {{dim, pos}, {Dim::X, x}, {Dim::Y, y}, {Dim::Z, z}});

  a = concatenate(a, a + a, Dim::Temperature);
  EXPECT_EQ(a.dims(), Dimensions({Dim::Temperature, Dim::Position}, {2, 4}));
  const auto temp = makeVariable<double>(Dims{Dim::Temperature}, Shape{2});
  a.coords().set(Dim::Temperature, temp);
  return a;
}

auto make_realigned() {
  auto a = make_array();

  const auto xbins =
      makeVariable<double>(Dims{Dim::X}, Shape{3}, Values{0, 2, 4});
  const auto ybins =
      makeVariable<double>(Dims{Dim::Y}, Shape{3}, Values{0, 2, 4});
  const auto zbins =
      makeVariable<double>(Dims{Dim::Z}, Shape{3}, Values{0, 2, 4});

  return unaligned::realign(
      a, {{Dim::Z, zbins}, {Dim::Y, ybins}, {Dim::X, xbins}});
}

auto make_aligned() {
  const auto temp = makeVariable<double>(Dims{Dim::Temperature}, Shape{2});
  const auto xbins =
      makeVariable<double>(Dims{Dim::X}, Shape{3}, Values{0, 2, 4});
  const auto ybins =
      makeVariable<double>(Dims{Dim::Y}, Shape{3}, Values{0, 2, 4});
  const auto zbins =
      makeVariable<double>(Dims{Dim::Z}, Shape{3}, Values{0, 2, 4});

  // TODO set proper values
  return DataArray(
      makeVariable<double>(Dims{Dim::Temperature, Dim::Z, Dim::Y, Dim::X},
                           Shape{2, 2, 2, 2}),
      {{Dim::Temperature, temp},
       {Dim::Z, zbins},
       {Dim::Y, ybins},
       {Dim::X, xbins}});
}
} // namespace

TEST(UnalignedTest, slice) {
  const auto realigned = make_realigned();
  const auto aligned = make_aligned();

    for (const auto dim : {Dim::Temperature, Dim::X, Dim::Y, Dim::Z}) {
      for (const auto s : {Slice(dim, 0), Slice(dim, 1), Slice(dim, 0, 1),
                           Slice(dim, 0, 2), Slice(dim, 1, 2)}) {
        const auto slice = realigned.slice(s);
        const auto reference = aligned.slice(s);
        // Same result as when slicing normal array, except for missing data
        EXPECT_FALSE(slice.hasData());
        EXPECT_EQ(slice.dims(), reference.dims());
        EXPECT_EQ(slice.coords(), reference.coords());
    }
  }
}
