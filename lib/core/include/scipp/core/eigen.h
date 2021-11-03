// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "scipp/core/dtype.h"

// using AffineTransform = Eigen::Transform<double, 3, Eigen::Affine, Eigen::DontAlign>;

namespace scipp::core {

template <> inline constexpr DType dtype<Eigen::Vector3d>{4000};
template <> inline constexpr DType dtype<Eigen::Matrix3d>{4001};
template <> inline constexpr DType dtype<Eigen::Affine3d>{4002};
template <>
inline constexpr DType dtype<std::span<const Eigen::Vector3d>>{4100};
template <> inline constexpr DType dtype<std::span<Eigen::Vector3d>>{4200};

} // namespace scipp::core
