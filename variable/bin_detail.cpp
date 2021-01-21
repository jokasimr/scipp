// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/core/element/bin_detail.h"
#include "scipp/core/element/util.h" // fill_zeros
#include "scipp/core/subbin_sizes.h"

#include "scipp/variable/bin_detail.h"
#include "scipp/variable/bin_util.h"
#include "scipp/variable/subspan_view.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/util.h"

namespace scipp::variable::bin_detail {

/// Index of the bin (given by `edges`) containing a coord value.
///
/// 0 if the coord is less than the first edge, nbin-1 if greater or equal last
/// edge. Assumes both `edges` and `coord` are sorted.
Variable begin_edge(const VariableConstView &coord,
                    const VariableConstView &edges) {
  auto indices = makeVariable<scipp::index>(coord.dims());
  const auto dim = edges.dims().inner();
  Variable bin(indices.slice({dim, 0}));
  accumulate_in_place(bin, indices, coord, subspan_view(edges, dim),
                      core::element::begin_edge);
  return indices;
}

/// End bin
///
/// 1 if the coord is
/// nbin if the coord is greater than the last edge. Assumes both `edges` and
/// `coord` are sorted.
Variable end_edge(const VariableConstView &coord,
                  const VariableConstView &edges) {
  auto indices = makeVariable<scipp::index>(coord.dims());
  const auto dim = edges.dims().inner();
  Variable bin(indices.slice({dim, 0}));
  accumulate_in_place(bin, indices, coord, subspan_view(edges, dim),
                      core::element::end_edge);
  return indices;
}

Variable cumsum_exclusive_subbin_sizes(const VariableConstView &var) {
  return transform<core::SubbinSizes>(
      var,
      overloaded{[](const units::Unit &u) { return u; },
                 [](const auto &sizes) { return sizes.cumsum_exclusive(); }});
}

Variable sum_subbin_sizes(const VariableConstView &var) {
  return transform<core::SubbinSizes>(
      var, overloaded{[](const units::Unit &u) { return u; },
                      [](const auto &sizes) { return sizes.sum(); }});
}

std::vector<scipp::index> flatten_subbin_sizes(const VariableConstView &var,
                                               const scipp::index length) {
  std::vector<scipp::index> flat;
  for (const auto &val : var.values<core::SubbinSizes>()) {
    flat.insert(flat.end(), val.sizes().begin(), val.sizes().end());
    for (scipp::index i = 0; i < length - scipp::size(val.sizes()); ++i)
      flat.push_back(0);
  }
  return flat;
}

Variable subbin_sizes_cumsum_exclusive(const VariableConstView &var,
                                       const Dim dim) {
  if (var.dims()[dim] == 0)
    return Variable{var};
  Variable cumulative(var.slice({dim, 0}));
  fill_zeros(cumulative);
  Variable out(var);
  accumulate_in_place(cumulative, out,
                      core::element::subbin_sizes_exclusive_scan);
  return out;
}

void subbin_sizes_add_intersection(const VariableView &a,
                                   const VariableConstView &b) {
  transform_in_place(a, b, core::element::subbin_sizes_add_intersection);
}

} // namespace scipp::variable::bin_detail
