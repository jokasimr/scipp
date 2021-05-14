// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once
#include "scipp/core/dimensions.h"
#include "scipp/core/element_array_view.h"
#include "scipp/core/except.h"
#include "scipp/units/unit.h"
#include "scipp/variable/data_model.h"
#include "scipp/variable/except.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/variable_concept.h"

namespace scipp::variable {

/// Implementation of VariableConcept that holds and array with element type T.
template <class T, class Elem, int... N>
class StructuredModel : public VariableConcept {
public:
  using value_type = T;
  using element_type = Elem;
  static constexpr auto axis_count = sizeof...(N);
  static constexpr auto element_count = (N * ...);

  StructuredModel(const scipp::index size, const units::Unit &unit,
                  element_array<Elem> model)
      : VariableConcept(units::one), // unit ignored
        m_elements(std::make_shared<DataModel<Elem>>(size * element_count, unit,
                                                     std::move(model))) {}

  static DType static_dtype() noexcept { return scipp::dtype<T>; }
  DType dtype() const noexcept override { return scipp::dtype<T>; }
  scipp::index size() const override {
    return m_elements->size() / element_count;
  }

  const units::Unit &unit() const override { return m_elements->unit(); }
  void setUnit(const units::Unit &unit) override { m_elements->setUnit(unit); }

  VariableConceptHandle
  makeDefaultFromParent(const scipp::index size) const override;

  VariableConceptHandle
  makeDefaultFromParent(const Variable &shape) const override {
    return makeDefaultFromParent(shape.dims().volume());
  }

  bool equals(const Variable &a, const Variable &b) const override;
  void copy(const Variable &src, Variable &dest) const override;
  void copy(const Variable &src, Variable &&dest) const override;
  void assign(const VariableConcept &other) override;

  bool hasVariances() const noexcept override { return false; }
  void setVariances(const Variable &) override {
    throw except::VariancesError("This data type cannot have variances.");
  }

  VariableConceptHandle clone() const override {
    return std::make_unique<StructuredModel<T, Elem, N...>>(*this);
  }

  auto values(const core::ElementArrayViewParams &base) const {
    return ElementArrayView(base, get_values());
  }
  auto values(const core::ElementArrayViewParams &base) {
    return ElementArrayView(base, get_values());
  }

  VariableConceptHandle elements() const { return m_elements; }

  scipp::index dtype_size() const override { return sizeof(T); }
  const VariableConceptHandle &bin_indices() const override {
    throw except::TypeError("This data type does not have bin indices.");
  }

  scipp::span<const T> values() const { return {get_values(), size()}; }
  scipp::span<T> values() { return {get_values(), size()}; }

private:
  const T *get_values() const {
    return reinterpret_cast<const T *>(
        requireT<const DataModel<Elem>>(*m_elements).values().data());
  }
  T *get_values() {
    return reinterpret_cast<T *>(
        requireT<DataModel<Elem>>(*m_elements).values().data());
  }
  VariableConceptHandle m_elements;
};

template <class T, class Elem, int... N>
VariableConceptHandle StructuredModel<T, Elem, N...>::makeDefaultFromParent(
    const scipp::index size) const {
  return std::make_unique<StructuredModel<T, Elem, N...>>(
      size, unit(), element_array<Elem>(size * element_count));
}

template <class T, class Elem, int... N>
bool StructuredModel<T, Elem, N...>::equals(const Variable &a,
                                            const Variable &b) const {
  return equals_impl(a.elements<T>().template values<Elem>(),
                     b.elements<T>().template values<Elem>());
}

template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::copy(const Variable &src,
                                          Variable &dest) const {
  transform_in_place<T>(dest, src, [](auto &a, const auto &b) { a = b; });
}
template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::copy(const Variable &src,
                                          Variable &&dest) const {
  copy(src, dest);
}

template <class T, class Elem, int... N>
void StructuredModel<T, Elem, N...>::assign(const VariableConcept &other) {
  *this = requireT<const StructuredModel<T, Elem, N...>>(other);
}

} // namespace scipp::variable
