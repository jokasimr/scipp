// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/variable/element_array_model.h"
#include "scipp/variable/variable.tcc"

namespace scipp::variable {

template <class T>
Variable make_default_init(const Dimensions &dims, const units::Unit &unit,
                           const bool variances) {
  if (variances && !core::canHaveVariances<T>())
    throw except::VariancesError("This data type cannot have variances.");
  const auto volume = dims.volume();
  VariableConceptHandle model;
  if constexpr (std::is_same_v<model_t<T>, ElementArrayModel<T>>) {
    if (variances)
      model = std::make_shared<model_t<T>>(
          volume, unit, element_array<T>(volume, core::default_init_elements),
          element_array<T>(volume, core::default_init_elements));
    else
      model = std::make_shared<model_t<T>>(
          volume, unit, element_array<T>(volume, core::default_init_elements));
  } else {
    using Elem = typename model_t<T>::element_type;
    model = std::make_shared<model_t<T>>(
        volume, unit,
        element_array<Elem>(model_t<T>::element_count * volume,
                            core::default_init_elements));
  }
  return Variable(dims, std::move(model));
}

template <class T> class VariableMaker : public AbstractVariableMaker {
  using AbstractVariableMaker::create;
  bool is_bins() const override { return false; }
  Variable create(const DType, const Dimensions &dims, const units::Unit &unit,
                  const bool variances, const parent_list &) const override {
    return make_default_init<T>(dims, unit, variances);
  }
  Dim elem_dim(const Variable &) const override { return Dim::Invalid; }
  DType elem_dtype(const Variable &var) const override { return var.dtype(); }
  units::Unit elem_unit(const Variable &var) const override {
    return var.unit();
  }
  void expect_can_set_elem_unit(const Variable &var,
                                const units::Unit &u) const override {
    var.expectCanSetUnit(u);
  }
  void set_elem_unit(Variable &var, const units::Unit &u) const override {
    var.setUnit(u);
  }
  bool hasVariances(const Variable &var) const override {
    return var.hasVariances();
  }
  Variable empty_like(const Variable &prototype,
                      const std::optional<Dimensions> &shape,
                      const Variable &sizes) const override {
    if (sizes.is_valid())
      throw except::TypeError(
          "Cannot specify sizes in `empty_like` for non-bin prototype.");
    return create(prototype.dtype(), shape ? *shape : prototype.dims(),
                  prototype.unit(), prototype.hasVariances(), {});
  }
  virtual bool contains(const Variable &container, const Variable &var) const override {
    if (!container.is_valid() || container.dtype() != dtype<Variable>) {
      return false;
    }
    for (const auto &nested : container.values<Variable>()) {
      if (&var == &nested) {
        return true;
      }
      if (variableFactory().contains(nested, var)) {
        return true;
      }
    }
    return false;
  }
};

template <class T>
ElementArrayModel<T>::ElementArrayModel(
    const scipp::index size, const units::Unit &unit, element_array<T> model,
    std::optional<element_array<T>> variances)
    : VariableConcept(unit),
      m_values(model ? std::move(model)
                     : element_array<T>(size, default_init<T>::value())),
      m_variances(std::move(variances)) {
  if (m_variances)
    core::expect::canHaveVariances<T>();
  if (size != scipp::size(m_values))
    throw except::DimensionError("Creating Variable: data size does not match "
                                 "volume given by dimension extents.");
  if (m_variances && !*m_variances)
    *m_variances = element_array<T>(size, default_init<T>::value());
}

template <class T> VariableConceptHandle ElementArrayModel<T>::clone() const {
  return std::make_shared<ElementArrayModel<T>>(*this);
}

template <class T>
VariableConceptHandle
ElementArrayModel<T>::makeDefaultFromParent(const scipp::index size) const {
  if (hasVariances())
    return std::make_shared<ElementArrayModel<T>>(
        size, unit(), element_array<T>(size), element_array<T>(size));
  else
    return std::make_shared<ElementArrayModel<T>>(size, unit(),
                                                  element_array<T>(size));
}

/// Helper for implementing Variable(View)::operator==.
///
/// This method is using virtual dispatch as a trick to obtain T, such that
/// values<T> and variances<T> can be compared.
template <class T>
bool ElementArrayModel<T>::equals(const Variable &a, const Variable &b) const {
  return equals_impl(a.values<T>(), b.values<T>()) &&
         (!a.hasVariances() || equals_impl(a.variances<T>(), b.variances<T>()));
}

template <class T>
void ElementArrayModel<T>::assign(const VariableConcept &other) {
  *this = requireT<const ElementArrayModel<T>>(other);
}

template <class T>
void ElementArrayModel<T>::setVariances(const Variable &variances) {
  if (!core::canHaveVariances<T>())
    throw except::VariancesError("This data type cannot have variances.");
  if (!variances.is_valid())
    return m_variances.reset();
  // TODO Could move if refcount is 1?
  if (variances.hasVariances())
    throw except::VariancesError(
        "Cannot set variances from variable with variances.");
  m_variances.emplace(
      requireT<const ElementArrayModel>(variances.data()).m_values);
}

/// Macro for instantiating classes and functions required for support a new
/// dtype in Variable.
#define INSTANTIATE_ELEMENT_ARRAY_VARIABLE(name, ...)                          \
  template class SCIPP_EXPORT ElementArrayModel<__VA_ARGS__>;                  \
  template SCIPP_EXPORT Variable variable::make_default_init<__VA_ARGS__>(     \
      const Dimensions &, const units::Unit &, const bool);                    \
  INSTANTIATE_VARIABLE_BASE(name, __VA_ARGS__)                                 \
  namespace {                                                                  \
  auto register_variable_maker_##name((                                        \
      variableFactory().emplace(                                               \
          dtype<__VA_ARGS__>, std::make_unique<VariableMaker<__VA_ARGS__>>()), \
      0));                                                                     \
  }                                                                            \
  template SCIPP_EXPORT Variable::Variable(                                    \
      const units::Unit, const Dimensions &, element_array<__VA_ARGS__>,       \
      std::optional<element_array<__VA_ARGS__>>);                              \
  template SCIPP_EXPORT ElementArrayView<const __VA_ARGS__>                    \
  Variable::variances() const;                                                 \
  template SCIPP_EXPORT ElementArrayView<__VA_ARGS__> Variable::variances();

} // namespace scipp::variable
