// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
#pragma once

#include <string_view>

#include "pybind11.h"
#include <scipp/core/dtype.h>
#include <scipp/units/unit.h>

namespace pybind11 {
class dtype;
}

scipp::core::DType dtype_of(const pybind11::object &x);

scipp::core::DType cast_dtype(const pybind11::object &dtype);

void ensure_conversion_possible(scipp::core::DType from, scipp::core::DType to,
                                const std::string &data_name);

template <class T> decltype(auto) converting_cast(const pybind11::object &obj) {
  return obj.cast<T>();
}

template <>
inline decltype(auto) converting_cast<int64_t>(const pybind11::object &obj) {
  if (dtype_of(obj) == scipp::dtype<double>) {
    // This conversion is not implemented in pybind11 v2.6.2
    return obj.cast<pybind11::int_>().cast<int64_t>();
  } else {
    // All other conversions are either supported by pybind11 or not
    // desired anyway.
    return obj.cast<int64_t>();
  }
}

scipp::core::DType common_dtype(const pybind11::object &values,
                                const pybind11::object &variances,
                                scipp::core::DType dtype, bool plural);

scipp::core::DType scipp_dtype(const pybind11::object &type);

bool has_datetime_dtype(const pybind11::object &obj);

[[nodiscard]] scipp::units::Unit
parse_datetime_dtype(const std::string &dtype_name);
[[nodiscard]] scipp::units::Unit
parse_datetime_dtype(const pybind11::object &dtype);
