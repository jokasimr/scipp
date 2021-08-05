// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/core/element/math.h"
#include "scipp/core/value_and_variance.h"
#include "scipp/units/unit.h"
#include "test_macros.h"

using namespace scipp;
using namespace scipp::core;

TEST(ElementAbsTest, unit) {
  units::Unit m(units::m);
  EXPECT_EQ(element::abs(m), units::abs(m));
}

TEST(ElementAbsTest, value) {
  EXPECT_EQ(element::abs(-1.23), std::abs(-1.23));
  EXPECT_EQ(element::abs(-1.23456789f), std::abs(-1.23456789f));
}

TEST(ElementAbsTest, value_and_variance) {
  const ValueAndVariance x(-2.0, 1.0);
  EXPECT_EQ(element::abs(x), abs(x));
}

TEST(ElementAbsTest, supported_types) {
  auto supported = decltype(element::abs)::types{};
  static_cast<void>(std::get<double>(supported));
  static_cast<void>(std::get<float>(supported));
}

TEST(ElementNormTest, unit) {
  const units::Unit s(units::s);
  const units::Unit m2(units::m * units::m);
  const units::Unit dimless(units::dimensionless);
  EXPECT_EQ(element::norm(m2), m2);
  EXPECT_EQ(element::norm(s), s);
  EXPECT_EQ(element::norm(dimless), dimless);
}

TEST(ElementNormTest, value) {
  Eigen::Vector3d v1(0, 3, 4);
  Eigen::Vector3d v2(3, 0, -4);
  EXPECT_EQ(element::norm(v1), 5);
  EXPECT_EQ(element::norm(v2), 5);
}

TEST(ElementPowTest, unit) {
  // element::pow cannot handle units itself as that requires the unit of the
  // base but the *value* of the exponent. This does not fit into the usual
  // transform framework.
  EXPECT_EQ(element::pow(units::one, units::one), units::one);
  EXPECT_THROW_DISCARD(element::pow(units::one, units::m), except::UnitError);
  EXPECT_THROW_DISCARD(element::pow(units::s, units::one), except::UnitError);
  EXPECT_THROW_DISCARD(element::pow(units::K, units::kg), except::UnitError);
}

TEST(ElementPowTest, value) {
  EXPECT_NEAR(element::pow(3.0, 2.0), 9.0, 1e-15);
  EXPECT_NEAR(element::pow(int64_t{3}, 2.0), 9.0, 1e-15);
  EXPECT_NEAR(element::pow(3.0, int64_t{2}), 9.0, 1e-15);
  EXPECT_EQ(element::pow(int64_t{3}, int64_t{2}), 9);
}

TEST(ElementPowTest, value_float_base_integer_exponent) {
  for (double base : {-5, -3, -2, -1, 1, 2, 5, 10}) {
    EXPECT_NEAR(element::pow(base, int64_t{0}), int64_t{1}, 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{1}), base, 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{2}), base * base, 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{3}), base * base * base, 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{-1}), 1.0 / base, 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{-2}), 1.0 / (base * base), 1e-12);
    EXPECT_NEAR(element::pow(base, int64_t{-3}), 1.0 / (base * base * base),
                1e-12);
  }
  EXPECT_NEAR(element::pow(0.0, int64_t{0}), 1.0, 1e-16);
  EXPECT_NEAR(element::pow(0.0, int64_t{1}), 0.0, 1e-16);
  EXPECT_NEAR(element::pow(0.0, int64_t{6}), 0.0, 1e-16);
  EXPECT_TRUE(std::isinf(element::pow(0.0, int64_t{-1})));
  EXPECT_NEAR(element::pow(4.125, int64_t{13}), 100117820.6814957, 1e-12);
  EXPECT_NEAR(element::pow(9.247, int64_t{26}), 1.3062379536886155e+25, 1e11);
}

TEST(ElementSqrtTest, unit) {
  const units::Unit m2(units::m * units::m);
  EXPECT_EQ(element::sqrt(m2), units::sqrt(m2));
}

TEST(ElementSqrtTest, value) {
  EXPECT_EQ(element::sqrt(1.23), std::sqrt(1.23));
  EXPECT_EQ(element::sqrt(1.23456789f), std::sqrt(1.23456789f));
}

TEST(ElementSqrtTest, value_and_variance) {
  const ValueAndVariance x(2.0, 1.0);
  EXPECT_EQ(element::sqrt(x), sqrt(x));
}

TEST(ElementSqrtTest, supported_types) {
  auto supported = decltype(element::sqrt)::types{};
  static_cast<void>(std::get<double>(supported));
  static_cast<void>(std::get<float>(supported));
}

TEST(ElementDotTest, unit) {
  const units::Unit m(units::m);
  const units::Unit m2(units::m * units::m);
  const units::Unit dimless(units::dimensionless);
  EXPECT_EQ(element::dot(m, m), m2);
  EXPECT_EQ(element::dot(dimless, dimless), dimless);
}

TEST(ElementDotTest, value) {
  Eigen::Vector3d v1(0, 3, -4);
  Eigen::Vector3d v2(1, 1, -1);
  EXPECT_EQ(element::dot(v1, v1), 25);
  EXPECT_EQ(element::dot(v2, v2), 3);
}

TEST(ElementReciprocalTest, unit) {
  const units::Unit one_over_m(units::one / units::m);
  EXPECT_EQ(element::reciprocal(one_over_m), units::m);
  const units::Unit one_over_s(units::one / units::s);
  EXPECT_EQ(element::reciprocal(units::s), one_over_s);
}

TEST(ElementReciprocalTest, value) {
  EXPECT_EQ(element::reciprocal(1.23), 1 / 1.23);
  EXPECT_EQ(element::reciprocal(1.23456789f), 1 / 1.23456789f);
}

TEST(ElementReciprocalTest, value_and_variance) {
  const ValueAndVariance x(2.0, 1.0);
  EXPECT_EQ(element::reciprocal(x), 1 / x);
}

TEST(ElementExpTest, value) {
  EXPECT_EQ(element::exp(1.23), std::exp(1.23));
  EXPECT_EQ(element::exp(1.23456789f), std::exp(1.23456789f));
}

TEST(ElementExpTest, unit) {
  EXPECT_EQ(element::exp(units::dimensionless), units::dimensionless);
}

TEST(ElementExpTest, bad_unit) { EXPECT_ANY_THROW(element::exp(units::m)); }

TEST(ElementLogTest, value) {
  EXPECT_EQ(element::log(1.23), std::log(1.23));
  EXPECT_EQ(element::log(1.23456789f), std::log(1.23456789f));
}

TEST(ElementLogTest, unit) {
  EXPECT_EQ(element::log(units::dimensionless), units::dimensionless);
}

TEST(ElementLogTest, bad_unit) { EXPECT_ANY_THROW(element::log(units::m)); }

TEST(ElementLog10Test, value) {
  EXPECT_EQ(element::log10(1.23), std::log10(1.23));
  EXPECT_EQ(element::log10(1.23456789f), std::log10(1.23456789f));
}

TEST(ElementLog10Test, unit) {
  EXPECT_EQ(element::log10(units::dimensionless), units::dimensionless);
}

TEST(ElementLog10Test, bad_unit) { EXPECT_ANY_THROW(element::log10(units::m)); }
