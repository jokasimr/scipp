// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Neil Vaytet
#pragma once

#include "scipp-core_export.h"
#include "scipp/common/index_composition.h"
#include "scipp/core/dimensions.h"
#include "scipp/core/strides.h"

namespace scipp::core {

class SCIPP_CORE_EXPORT ViewIndex {
public:
  ViewIndex(const Dimensions &target_dimensions, const Strides &strides);

  constexpr void increment_outer() noexcept {
    scipp::index d = 0;
    while ((m_coord[d] == m_extent[d]) && (d < NDIM_MAX - 1)) {
      m_index += m_delta[d + 1];
      ++m_coord[d + 1];
      m_coord[d] = 0;
      ++d;
    }
  }
  constexpr void increment() noexcept {
    m_index += m_delta[0];
    ++m_coord[0];
    if (m_coord[0] == m_extent[0])
      increment_outer();
    ++m_fullIndex;
  }

  constexpr void setIndex(const scipp::index index) noexcept {
    m_fullIndex = index;
    extract_indices(index, m_dims, m_extent, m_coord);
    m_index = flat_index_from_strides(m_strides.begin(), m_strides.end(m_dims),
                                      m_coord.begin());
  }

  void set_to_end() noexcept {
    m_fullIndex = 0;
    for (scipp::index dim = 0; dim < m_dims - 1; ++dim) {
      m_fullIndex *= m_extent[dim];
    }
    std::fill(m_coord.begin(), m_coord.begin() + std::max(m_dims - 1, 0), 0);
    m_coord[m_dims] = m_extent[m_dims];
    m_index = m_coord[m_dims] * m_strides[m_dims];
  }

  [[nodiscard]] constexpr scipp::index get() const noexcept { return m_index; }
  [[nodiscard]] constexpr scipp::index index() const noexcept {
    return m_fullIndex;
  }

  constexpr bool operator==(const ViewIndex &other) const noexcept {
    return m_fullIndex == other.m_fullIndex;
  }
  constexpr bool operator!=(const ViewIndex &other) const noexcept {
    return m_fullIndex != other.m_fullIndex;
  }

private:
  // NOTE:
  // We investigated different containers for the m_delta, m_coord & m_extent
  // arrays, and their impact on performance when iterating over a variable
  // view.
  // Using std::array or C-style arrays give good performance (7.5 Gb/s) as long
  // as a range based loop is used:
  //
  //   for ( const auto x : view ) {
  //
  // If a loop which explicitly accesses the begin() and end() of the container
  // is used, e.g.
  //
  //   for ( auto it = view.begin(); it != view.end(); ++it ) {
  //
  // then the results differ widely.
  // - using std::array is 80x slower than above, at ~90 Mb/s
  // - using C-style arrays is 20x slower than above, at ~330 Mb/s
  //
  // We can recover the maximum performance by storing the view.end() in a
  // variable, e.g.
  //
  //   auto iend = view.end();
  //   for ( auto it = view.begin(); it != iend; ++it ) {
  //
  // for both std::array and C-style arrays.
  //
  // Finally, when using C-style arrays, we get a compilation warning from L37
  //
  //   m_delta[d] -= m_delta[d2];
  //
  // with the GCC compiler:
  //
  //  warning: array subscript is above array bounds [-Warray-bounds]
  //
  // which disappears when switching to std::array. This warning is not given
  // by the CLANG compiler, and is not fully understood as d2 is always less
  // than d and should never overflow the array bounds.
  // We decided to go with std::array as our final choice to avoid the warning,
  // as the performance is identical to C-style arrays, as long as range based
  // loops are used.

  scipp::index m_index{0};
  std::array<scipp::index, NDIM_MAX> m_delta = {};
  std::array<scipp::index, NDIM_MAX> m_coord = {};
  std::array<scipp::index, NDIM_MAX> m_extent = {};
  /// Strides in memory.
  Strides m_strides;
  /// Index in iteration dimensions.
  scipp::index m_fullIndex{0};
  /// Number of dimensions.
  int32_t m_dims;
};

} // namespace scipp::core
