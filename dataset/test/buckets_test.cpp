// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/dataset/bucket.h"
#include "scipp/dataset/dataset.h"
#include "scipp/dataset/histogram.h"
#include "scipp/dataset/shape.h"
#include "scipp/variable/bucket_model.h"
#include "scipp/variable/operations.h"

using namespace scipp;
using namespace scipp::dataset;

class DataArrayBucketTest : public ::testing::Test {
protected:
  using Model = variable::DataModel<bucket<DataArray>>;
  Dimensions dims{Dim::Y, 2};
  Variable indices = makeVariable<std::pair<scipp::index, scipp::index>>(
      dims, Values{std::pair{0, 2}, std::pair{2, 4}});
  Variable data =
      makeVariable<double>(Dims{Dim::X}, Shape{4}, Values{1, 2, 3, 4});
  DataArray buffer = DataArray(data, {{Dim::X, data + data}});
  Variable var{std::make_unique<Model>(indices, Dim::X, buffer)};
};

TEST_F(DataArrayBucketTest, concatenate) {
  const auto result = buckets::concatenate(var, var * (3.0 * units::one));
  Variable out_indices = makeVariable<std::pair<scipp::index, scipp::index>>(
      dims, Values{std::pair{0, 4}, std::pair{4, 8}});
  Variable out_data = makeVariable<double>(Dims{Dim::X}, Shape{8},
                                           Values{1, 2, 3, 6, 3, 4, 9, 12});
  Variable out_x = makeVariable<double>(Dims{Dim::X}, Shape{8},
                                        Values{2, 4, 2, 4, 6, 8, 6, 8});
  DataArray out_buffer = DataArray(out_data, {{Dim::X, out_x}});
  EXPECT_EQ(result,
            Variable(std::make_unique<Model>(out_indices, Dim::X, out_buffer)));

  // "in-place" append gives same as concatenate
  buckets::append(var, var * (3.0 * units::one));
  EXPECT_EQ(result, var);
  buckets::append(var, -var);
}

TEST_F(DataArrayBucketTest, concatenate_with_broadcast) {
  auto var2 = var;
  var2.rename(Dim::Y, Dim::Z);
  var2 *= 3.0 * units::one;
  const auto result = buckets::concatenate(var, var2);
  Variable out_indices = makeVariable<std::pair<scipp::index, scipp::index>>(
      Dims{Dim::Y, Dim::Z}, Shape{2, 2},
      Values{std::pair{0, 4}, std::pair{4, 8}, std::pair{8, 12},
             std::pair{12, 16}});
  Variable out_data = makeVariable<double>(
      Dims{Dim::X}, Shape{16},
      Values{1, 2, 3, 6, 1, 2, 9, 12, 3, 4, 3, 6, 3, 4, 9, 12});
  Variable out_x = makeVariable<double>(
      Dims{Dim::X}, Shape{16},
      Values{2, 4, 2, 4, 2, 4, 6, 8, 6, 8, 2, 4, 6, 8, 6, 8});
  DataArray out_buffer = DataArray(out_data, {{Dim::X, out_x}});
  EXPECT_EQ(result,
            Variable(std::make_unique<Model>(out_indices, Dim::X, out_buffer)));

  // Broadcast not possible for in-place append
  EXPECT_THROW(buckets::append(var, var2), except::DimensionMismatchError);
}

TEST_F(DataArrayBucketTest, histogram) {
  Variable weights = makeVariable<double>(
      Dims{Dim::X}, Shape{4}, Values{1, 2, 3, 4}, Variances{1, 2, 3, 4});
  DataArray events = DataArray(weights, {{Dim::Z, data}});
  Variable buckets{std::make_unique<Model>(indices, Dim::X, events)};
  // `buckets` *does not* depend on the histogramming dimension
  const auto bin_edges =
      makeVariable<double>(Dims{Dim::Z}, Shape{4}, Values{0, 1, 2, 4});
  EXPECT_EQ(buckets::histogram(buckets, bin_edges),
            makeVariable<double>(Dims{Dim::Y, Dim::Z}, Shape{2, 3},
                                 Values{0, 1, 2, 0, 0, 3},
                                 Variances{0, 1, 2, 0, 0, 3}));
}

TEST_F(DataArrayBucketTest, histogram_existing_dim) {
  Variable weights = makeVariable<double>(
      Dims{Dim::X}, Shape{4}, Values{1, 2, 3, 4}, Variances{1, 2, 3, 4});
  DataArray events = DataArray(weights, {{Dim::Y, data}});
  Variable buckets{std::make_unique<Model>(indices, Dim::X, events)};
  // `buckets` *does* depend on the histogramming dimension
  const auto bin_edges =
      makeVariable<double>(Dims{Dim::Y}, Shape{4}, Values{0, 1, 2, 4});
  const auto expected = makeVariable<double>(
      Dims{Dim::Y}, Shape{3}, Values{0, 1, 5}, Variances{0, 1, 5});
  EXPECT_EQ(buckets::histogram(buckets, bin_edges), expected);

  // Histogram data array containing bucket variable
  DataArray a(buckets);
  EXPECT_EQ(histogram(a, bin_edges),
            DataArray(expected, {{Dim::Y, bin_edges}}));
  // Masked data array
  a.masks().set(
      "mask", makeVariable<bool>(Dims{Dim::Y}, Shape{2}, Values{false, true}));
  EXPECT_EQ(histogram(a, bin_edges),
            DataArray(makeVariable<double>(Dims{Dim::Y}, Shape{3},
                                           Values{0, 1, 2}, Variances{0, 1, 2}),
                      {{Dim::Y, bin_edges}}));
}

TEST_F(DataArrayBucketTest, sum) {
  EXPECT_EQ(buckets::sum(var),
            makeVariable<double>(indices.dims(), Values{3, 7}));
}

class DataArrayBucketMapTest : public ::testing::Test {
protected:
  using ModelVariable = variable::DataModel<bucket<Variable>>;
  using ModelDataArray = variable::DataModel<bucket<DataArray>>;
  Dimensions dims{Dim::Y, 2};
  Variable indices = makeVariable<std::pair<scipp::index, scipp::index>>(
      dims, Values{std::pair{0, 2}, std::pair{2, 4}});
  Variable data =
      makeVariable<double>(Dims{Dim::X}, Shape{4}, Values{1, 2, 3, 4});
  Variable weights = makeVariable<double>(
      Dims{Dim::X}, Shape{4}, Values{1, 2, 3, 4}, Variances{1, 2, 3, 4});
  DataArray events = DataArray(weights, {{Dim::Z, data}});
  Variable buckets{std::make_unique<ModelDataArray>(indices, Dim::X, events)};
  // `buckets` *does not* depend on the histogramming dimension
  Variable bin_edges =
      makeVariable<double>(Dims{Dim::Z}, Shape{4}, Values{0, 1, 2, 4});
  DataArray histogram = DataArray(Variable(bin_edges.slice({Dim::Z, 1, 4})),
                                  {{Dim::Z, bin_edges}});
};

TEST_F(DataArrayBucketMapTest, map) {
  const auto out = buckets::map(histogram, buckets, Dim::Z);
  // event coords 1,2,3,4
  // histogram:
  // | 1 | 2 | 4 |
  // 0   1   2   4
  const auto expected_scale =
      makeVariable<double>(Dims{Dim::X}, Shape{4}, Values{2, 4, 4, 0});
  EXPECT_EQ(out, Variable(std::make_unique<ModelVariable>(indices, Dim::X,
                                                          expected_scale)));

  // Mapping result can be used to scale
  auto scaled = buckets * out;
  const auto expected = Variable{std::make_unique<ModelDataArray>(
      indices, Dim::X, events * expected_scale)};
  EXPECT_EQ(scaled, expected);

  // Mapping and scaling also works for slices
  auto partial = buckets;
  for (auto s : {Slice(Dim::Y, 0), Slice(Dim::Y, 1)})
    partial.slice(s) *= buckets::map(histogram, buckets.slice(s), Dim::Z);
  EXPECT_EQ(partial, expected);
}

TEST_F(DataArrayBucketMapTest, map_masked) {
  histogram.masks().set(
      "mask", makeVariable<bool>(histogram.dims(), Values{false, true, false}));
  const auto out = buckets::map(histogram, buckets, Dim::Z);
  const auto expected_scale =
      makeVariable<double>(Dims{Dim::X}, Shape{4}, Values{0, 4, 4, 0});
  EXPECT_EQ(out, Variable(std::make_unique<ModelVariable>(indices, Dim::X,
                                                          expected_scale)));
}

class DatasetBucketTest : public ::testing::Test {
protected:
  using Model = variable::DataModel<bucket<Dataset>>;
  Dimensions dims{Dim::Y, 2};
  Variable indices = makeVariable<std::pair<scipp::index, scipp::index>>(
      dims, Values{std::pair{0, 2}, std::pair{2, 3}});
  Variable column =
      makeVariable<double>(Dims{Dim::X}, Shape{3}, Values{1, 2, 3});
  Dataset buffer0;
  Dataset buffer1;

  void check() {
    Variable var0{std::make_unique<Model>(indices, Dim::X, buffer0)};
    Variable var1{std::make_unique<Model>(indices, Dim::X, buffer1)};
    const auto result = buckets::concatenate(var0, var1);
    EXPECT_EQ(result.values<bucket<Dataset>>()[0],
              concatenate(buffer0.slice({Dim::X, 0, 2}),
                          buffer1.slice({Dim::X, 0, 2}), Dim::X));
    EXPECT_EQ(result.values<bucket<Dataset>>()[1],
              concatenate(buffer0.slice({Dim::X, 2, 3}),
                          buffer1.slice({Dim::X, 2, 3}), Dim::X));
  }
  void check_fail() {
    Variable var0{std::make_unique<Model>(indices, Dim::X, buffer0)};
    Variable var1{std::make_unique<Model>(indices, Dim::X, buffer1)};
    EXPECT_ANY_THROW(buckets::concatenate(var0, var1));
  }
};

TEST_F(DatasetBucketTest, concatenate) {
  buffer0.coords().set(Dim::X, column);
  buffer1.coords().set(Dim::X, column + column);
  check();
  buffer0.setData("a", column * column);
  check_fail();
  buffer1.setData("a", column);
  check();
  buffer0.setData("b", column * column);
  check_fail();
  buffer1.setData("b", column / column);
  check();
  buffer0["a"].masks().set("mask", column);
  check_fail();
  buffer1["a"].masks().set("mask", column);
  check();
  buffer0["b"].coords().set(Dim("attr"), column);
  check_fail();
  buffer1["b"].coords().set(Dim("attr"), column);
  check();
  buffer0.coords().set(Dim("scalar"), 1.0 * units::m);
  check_fail();
  buffer1.coords().set(Dim("scalar"), 1.0 * units::m);
  check();
  buffer1.coords().set(Dim("scalar2"), 1.0 * units::m);
  check_fail();
}
