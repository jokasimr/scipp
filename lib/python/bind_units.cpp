// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include <sstream>

#include "scipp/core/dtype.h"
#include "scipp/core/tag_util.h"
#include "scipp/units/unit.h"

#include "pybind11.h"
#include "unit.h"

using namespace scipp;
namespace py = pybind11;

constexpr int UNIT_DICT_VERSION = 1;

namespace {

bool is_simple_unit(const units::Unit &unit) {
  const auto &&base_units = unit.underlying().base_units();
  return !base_units.is_per_unit() && !base_units.has_i_flag() &&
         !base_units.has_e_flag() && !base_units.is_equation() &&
         unit.underlying().commodity() == 0;
}

// We only support units where we are confident that we can encode them using
// a different unit library, in order to ensure that we can switch
// implementations in the future if necessary.
void assert_simple_unit_for_dict(const units::Unit &unit) {
  if (!is_simple_unit(unit)) {
    throw std::invalid_argument(
        "Unit cannot be converted to dict: '" + to_string(unit) +
        "' Only units expressed in terms of regular base units are supported.");
  }
}

py::dict to_dict(const units::Unit &unit) {
  assert_simple_unit_for_dict(unit);

  py::dict dict;
  dict["__version__"] = UNIT_DICT_VERSION;
  dict["multiplier"] = unit.underlying().multiplier();

  py::dict powers;
  unit.map_over_bases(
      [&powers](const char *const base, const auto power) mutable {
        if (power != 0) {
          powers[base] = power;
        }
      });
  if (!powers.empty())
    dict["powers"] = powers;

  return dict;
}

int get(const py::dict &dict, const char *const name) {
  if (dict.contains(name)) {
    return dict[name].cast<int>();
  }
  return 0;
}

units::Unit from_dict(const py::dict &dict) {
  if (const auto ver = dict["__version__"].cast<int>();
      ver != UNIT_DICT_VERSION) {
    throw std::invalid_argument(
        "Unit dict has version " + std::to_string(ver) +
        " but the current installation of scipp only supports version " +
        std::to_string(UNIT_DICT_VERSION));
  }

  const py::dict powers = dict.contains("powers") ? dict["powers"] : py::dict();
  return units::Unit(llnl::units::precise_unit(
      llnl::units::detail::unit_data{
          get(powers, "m"), get(powers, "kg"), get(powers, "s"),
          get(powers, "A"), get(powers, "K"), get(powers, "mol"),
          get(powers, "cd"), get(powers, "$"), get(powers, "counts"),
          get(powers, "rad"), 0, 0, 0, 0},
      dict["multiplier"].cast<double>()));
}

std::string repr(const units::Unit &unit) {
  if (!is_simple_unit(unit)) {
    return "<unsupported unit: " + to_string(unit) + '>';
  }

  std::ostringstream oss;
  oss << "Unit(" << unit.underlying().multiplier();
  unit.map_over_bases([&oss](const char *const base, const auto power) mutable {
    if (power != 0)
      oss << "*" << base << "**" << power;
  });
  oss << ')';
  return oss.str();
}

std::string repr_html(const units::Unit &unit) {
  // Regular string output is in a div with data-mime-type="text/plain"
  // But html output is in a div with data-mime-type="text/html"
  // Jupyter applies different padding to those, so hack the inner pre element
  // to match the padding of text/plain.
  return "<pre style=\"margin-bottom:0; padding-top:var(--jp-code-padding)\">" +
         unit.name() + "</pre>";
}

} // namespace

void init_units(py::module &m) {
  py::class_<DefaultUnit>(m, "DefaultUnit")
      .def("__repr__",
           [](const DefaultUnit &) { return "<automatically deduced unit>"; });
  py::class_<units::Unit>(m, "Unit", "A physical unit.", py::dynamic_attr())
      .def(py::init<const std::string &>())
      .def("__str__", [](const units::Unit &u) { return u.name(); })
      .def("__repr__", repr)
      .def("_repr_html_", repr_html)
      .def_property_readonly("name", &units::Unit::name,
                             "A read-only string describing the "
                             "type of unit.")
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self * py::self)
      // cppcheck-suppress duplicateExpression
      .def(py::self / py::self)
      .def("__pow__", [](const units::Unit &self,
                         const int64_t power) { return pow(self, power); })
      .def("__abs__", [](const units::Unit &self) { return abs(self); })
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(hash(py::self))
      .def("to_dict", to_dict,
           "Serialize a unit to a dict.\n\nThis function is meant to be used "
           "with :meth:`scipp.Unit.from_dict` to serialize units.\n\n"
           "Warning\n"
           "-------\n"
           "The structure of the returned dict is an implementation detail and "
           "may change without warning at any time! "
           "It should not be used to access the internal representation of "
           "``Unit``.")
      .def("from_dict", from_dict,
           "Deserialize a unit from a dict.\n\nThis function is meant to be "
           "used in combination with :meth:`scipp.Unit.to_dict`.");

  m.def("abs", [](const units::Unit &u) { return abs(u); });
  m.def("pow", [](const units::Unit &u, const int64_t power) {
    return pow(u, power);
  });
  m.def("pow",
        [](const units::Unit &u, const double power) { return pow(u, power); });
  m.def("reciprocal", [](const units::Unit &u) { return units::one / u; });
  m.def("sqrt", [](const units::Unit &u) { return sqrt(u); });

  py::implicitly_convertible<std::string, units::Unit>();

  auto units = m.def_submodule("units");
  units.attr("angstrom") = units::angstrom;
  units.attr("counts") = units::counts;
  units.attr("deg") = units::deg;
  units.attr("dimensionless") = units::dimensionless;
  units.attr("kg") = units::kg;
  units.attr("K") = units::K;
  units.attr("meV") = units::meV;
  units.attr("m") = units::m;
  // Note: No binding to units::none here, use None in Python!
  units.attr("one") = units::one;
  units.attr("rad") = units::rad;
  units.attr("s") = units::s;
  units.attr("us") = units::us;
  units.attr("ns") = units::ns;
  units.attr("mm") = units::mm;

  units.attr("default_unit") = DefaultUnit{};

  m.def("to_numpy_time_string",
        py::overload_cast<const ProtoUnit &>(to_numpy_time_string))
      .def(
          "units_identical",
          [](const units::Unit &a, const units::Unit &b) {
            return identical(a, b);
          },
          "Check if two units are numerically identical.\n\n"
          "The regular equality operator allows for small differences "
          "in the unit's floating point multiplier. ``units_identical`` "
          "checks for exact identity.")
      .def("add_unit_alias", scipp::units::add_unit_alias, py::kw_only(),
           py::arg("name"), py::arg("unit"))
      .def("clear_unit_aliases", scipp::units::clear_unit_aliases);
}
