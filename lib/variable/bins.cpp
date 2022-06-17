// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/core/eigen.h"
#include "scipp/core/element/arg_list.h"

#include "scipp/variable/bins.h"
#include "scipp/variable/comparison.h"
#include "scipp/variable/reduction.h"
#include "scipp/variable/subspan_view.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/util.h"

#include "operations_common.h"

namespace scipp::variable {

void copy_data(const Variable &src, Variable &dst) {
  transform_in_place<double, float, int64_t, int32_t, bool, std::string,
                     core::time_point, Eigen::Vector3d, Eigen::Matrix3d,
                     Eigen::Affine3d, core::Translation, core::Quaternion>(
      dst, src, [](auto &a, const auto &b) { a = b; }, "copy");
}

Variable bin_sizes(const Variable &var) {
  if (is_bins(var)) {
    const auto [begin, end] = unzip(var.bin_indices());
    return end - begin;
  }
  return makeVariable<scipp::index>(var.dims(), units::none);
}

void copy_slices(const Variable &src, Variable dst, const Dim dim,
                 const Variable &srcIndices, const Variable &dstIndices) {
  auto src_ = make_bins_no_validate(srcIndices, dim, src);
  auto dst_ = make_bins_no_validate(dstIndices, dim, dst);
  copy(src_, dst_);
}

Variable resize_default_init(const Variable &var, const Dim dim,
                             const scipp::index size) {
  auto dims = var.dims();
  if (dims.contains(dim))
    dims.resize(dim, size);
  // Using variableFactory instead of variable::resize for creating
  // _uninitialized_ variable.
  return variable::variableFactory().create(var.dtype(), dims, var.unit(),
                                            var.has_variances());
}

/// Construct a bin-variable over a variable.
///
/// Each bin is represented by a VariableView. `indices` defines the array of
/// bins as slices of `buffer` along `dim`.
Variable make_bins(Variable indices, const Dim dim, Variable buffer) {
  expect_valid_bin_indices(indices, dim, buffer.dims());
  return make_bins_no_validate(std::move(indices), dim, buffer);
}

/// Construct a bin-variable over a variable without index validation.
///
/// Must be used only when it is guaranteed that indices are valid or overlap of
/// bins is acceptable.
Variable make_bins_no_validate(Variable indices, const Dim dim,
                               Variable buffer) {
  return variable::make_bins_impl(std::move(indices), dim, std::move(buffer));
}

} // namespace scipp::variable
