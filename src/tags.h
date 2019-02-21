/// @file
/// SPDX-License-Identifier: GPL-3.0-or-later
/// @author Simon Heybrock
/// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
/// National Laboratory, and European Spallation Source ERIC.
#ifndef TAGS_H
#define TAGS_H

#include <memory>
#include <tuple>
#include <vector>

#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <gsl/gsl_util>

#include "bool.h"
#include "dimension.h"
#include "traits.h"
#include "unit.h"
#include "value_with_delta.h"

enum class DType {
  Unknown,
  Double,
  Float,
  Int32,
  Int64,
  String,
  Char,
  Bool,
  Dataset
};
template <class T> constexpr DType dtype = DType::Unknown;
template <> constexpr DType dtype<double> = DType::Double;
template <> constexpr DType dtype<float> = DType::Float;
template <> constexpr DType dtype<int32_t> = DType::Int32;
template <> constexpr DType dtype<int64_t> = DType::Int64;
template <> constexpr DType dtype<std::string> = DType::String;
template <> constexpr DType dtype<char> = DType::Char;
template <> constexpr DType dtype<bool> = DType::Bool;
template <> constexpr DType dtype<Bool> = DType::Bool;
template <> constexpr DType dtype<Dataset> = DType::Dataset;

// Adding new tags
// ===============
//
// 1. Add the tag class to either CoordDef, DataDef, or AttrDef in namespace
//    detail, and list the class in the corresponding`tags` tuple.
// 2. Add a `using` clause for the tag in Coord, Data, or Attr.
//
// Optionally:
// 3. If the underlying type is new, explicit instantiations in variable.cpp may
//    need to be added.
// 4. If the new tag corresponds to a dimension, `is_dimension_coordinate` and
//    `coordinate_dimension` need to be set correctly.
// 5. For support in Python, the tag needs to be exported in
//    `exports/dataset.cpp` and a fair number of methods in the same file need
//    to be modified to support the new tag.

class Tag {
public:
  constexpr Tag() : m_value(0) {}
  constexpr explicit Tag(uint16_t value) : m_value(value) {}
  constexpr uint16_t value() const { return m_value; }

  constexpr bool operator==(const Tag other) const {
    return m_value == other.m_value;
  }
  constexpr bool operator!=(const Tag other) const {
    return m_value != other.m_value;
  }

  constexpr bool operator<(const int32_t value) const {
    return m_value < value;
  }
  constexpr bool operator>=(const int32_t value) const {
    return m_value >= value;
  }

private:
  uint16_t m_value;
};

namespace detail {
struct ReturnByValuePolicy {};
// Returns by value for const access (allowing for deriving from other
// variables), but by reference if access is non-const (other code must ensure
// that no references to temporaries are returned). The main example for this is
// a spectrum position: It can be derived from detector positions (averaged
// using Coord::DetectorGrouping) or be provided directly. In the latter case
// the positions may be modified.
struct ReturnByValueIfConstPolicy {};
} // namespace detail

class Dataset;

namespace detail {
struct CoordDef {
  struct Monitor {
    using type = Dataset;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  // TODO Should we name this `Detectors` and `Components` instead, or find some
  // more generic terms?
  struct DetectorInfo {
    using type = Dataset;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct ComponentInfo {
    using type = Dataset;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct X {
    using type = double;
    static constexpr auto unit = Unit::Id::Length;
  };
  struct Y {
    using type = double;
    static constexpr auto unit = Unit::Id::Length;
  };
  struct Z {
    using type = double;
    static constexpr auto unit = Unit::Id::Length;
  };
  struct Tof {
    using type = double;
    static constexpr auto unit = Unit::Id::Tof;
  };
  struct Energy {
    using type = double;
    static constexpr auto unit = Unit::Id::Energy;
  };
  struct DeltaE {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Ei {
    using type = double;
    static constexpr auto unit = Unit::Id::Energy;
  };
  struct Ef {
    using type = double;
    static constexpr auto unit = Unit::Id::Energy;
  };
  struct DetectorId {
    using type = int32_t;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct SpectrumNumber {
    using type = int32_t;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct DetectorGrouping {
    // Dummy for now, or sufficient like this?
    using type = boost::container::small_vector<gsl::index, 1>;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct RowLabel {
    using type = std::string;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Polarization {
    // Dummy for now
    using type = std::string;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Temperature {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Time {
    using type = int64_t;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct TimeInterval {
    using type = std::pair<int64_t, int64_t>;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Mask {
    using type = bool;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct FuzzyTemperature {
    using type = ValueWithDelta<double>;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Position : public detail::ReturnByValueIfConstPolicy {
    using type = Eigen::Vector3d;
    static constexpr auto unit = Unit::Id::Length;
  };

  using tags = std::tuple<Monitor, DetectorInfo, ComponentInfo, X, Y, Z, Tof,
                          Energy, DeltaE, Ei, Ef, DetectorId, SpectrumNumber,
                          DetectorGrouping, RowLabel, Polarization, Temperature,
                          FuzzyTemperature, Time, TimeInterval, Mask, Position>;
};

struct DataDef {
  struct Tof {
    using type = double;
    static constexpr auto unit = Unit::Id::Tof;
  };
  struct PulseTime {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Value {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Variance {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct StdDev : public detail::ReturnByValuePolicy {
    using type = double;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Int {
    using type = int64_t;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct String {
    using type = std::string;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct Events {
    using type = Dataset;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };
  struct EventTofs {
    using type = boost::container::small_vector<double, 8>;
    static constexpr auto unit = Unit::Id::Tof;
  };
  struct EventPulseTimes {
    using type = boost::container::small_vector<double, 8>;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };

  using tags = std::tuple<Tof, PulseTime, Value, Variance, StdDev, Int, String,
                          Events, EventTofs, EventPulseTimes>;
};

struct AttrDef {
  struct ExperimentLog {
    using type = Dataset;
    static constexpr auto unit = Unit::Id::Dimensionless;
  };

  using tags = std::tuple<ExperimentLog>;
};

using Tags = decltype(std::tuple_cat(std::declval<detail::CoordDef::tags>(),
                                     std::declval<detail::DataDef::tags>(),
                                     std::declval<detail::AttrDef::tags>()));
template <class T>
static constexpr uint16_t tag_id =
    detail::index<std::remove_const_t<T>, Tags>::value;

template <class TagDefinition> struct TagImpl : public Tag, TagDefinition {
  constexpr TagImpl() : Tag(tag_id<TagDefinition>) {}
};

} // namespace detail

struct Coord {
  using Monitor_t = detail::TagImpl<detail::CoordDef::Monitor>;
  using DetectorInfo_t = detail::TagImpl<detail::CoordDef::DetectorInfo>;
  using ComponentInfo_t = detail::TagImpl<detail::CoordDef::ComponentInfo>;
  using X_t = detail::TagImpl<detail::CoordDef::X>;
  using Y_t = detail::TagImpl<detail::CoordDef::Y>;
  using Z_t = detail::TagImpl<detail::CoordDef::Z>;
  using Tof_t = detail::TagImpl<detail::CoordDef::Tof>;
  using Energy_t = detail::TagImpl<detail::CoordDef::Energy>;
  using DeltaE_t = detail::TagImpl<detail::CoordDef::DeltaE>;
  using Ei_t = detail::TagImpl<detail::CoordDef::Ei>;
  using Ef_t = detail::TagImpl<detail::CoordDef::Ef>;
  using DetectorId_t = detail::TagImpl<detail::CoordDef::DetectorId>;
  using SpectrumNumber_t = detail::TagImpl<detail::CoordDef::SpectrumNumber>;
  using DetectorGrouping_t =
      detail::TagImpl<detail::CoordDef::DetectorGrouping>;
  using RowLabel_t = detail::TagImpl<detail::CoordDef::RowLabel>;
  using Polarization_t = detail::TagImpl<detail::CoordDef::Polarization>;
  using Temperature_t = detail::TagImpl<detail::CoordDef::Temperature>;
  using FuzzyTemperature_t =
      detail::TagImpl<detail::CoordDef::FuzzyTemperature>;
  using Time_t = detail::TagImpl<detail::CoordDef::Time>;
  using TimeInterval_t = detail::TagImpl<detail::CoordDef::TimeInterval>;
  using Mask_t = detail::TagImpl<detail::CoordDef::Mask>;
  using Position_t = detail::TagImpl<detail::CoordDef::Position>;

  static constexpr Monitor_t Monitor{};
  static constexpr DetectorInfo_t DetectorInfo{};
  static constexpr ComponentInfo_t ComponentInfo{};
  static constexpr X_t X{};
  static constexpr Y_t Y{};
  static constexpr Z_t Z{};
  static constexpr Tof_t Tof{};
  static constexpr Energy_t Energy{};
  static constexpr DeltaE_t DeltaE{};
  static constexpr Ei_t Ei{};
  static constexpr Ef_t Ef{};
  static constexpr DetectorId_t DetectorId{};
  static constexpr SpectrumNumber_t SpectrumNumber{};
  static constexpr DetectorGrouping_t DetectorGrouping{};
  static constexpr RowLabel_t RowLabel{};
  static constexpr Polarization_t Polarization{};
  static constexpr Temperature_t Temperature{};
  static constexpr FuzzyTemperature_t FuzzyTemperature{};
  static constexpr Time_t Time{};
  static constexpr TimeInterval_t TimeInterval{};
  static constexpr Mask_t Mask{};
  static constexpr Position_t Position{};
};

struct Data {
  using Tof_t = detail::TagImpl<detail::DataDef::Tof>;
  using PulseTime_t = detail::TagImpl<detail::DataDef::PulseTime>;
  using Value_t = detail::TagImpl<detail::DataDef::Value>;
  using Variance_t = detail::TagImpl<detail::DataDef::Variance>;
  using StdDev_t = detail::TagImpl<detail::DataDef::StdDev>;
  // TODO Int and String is deprecated and should be removed, it is currently
  // only required to maintain tests using MDZipView before it is properly
  // refactored for multi-name support.
  using DeprecatedInt_t = detail::TagImpl<detail::DataDef::Int>;
  using DeprecatedString_t = detail::TagImpl<detail::DataDef::String>;
  using Events_t = detail::TagImpl<detail::DataDef::Events>;
  using EventTofs_t = detail::TagImpl<detail::DataDef::EventTofs>;
  using EventPulseTimes_t = detail::TagImpl<detail::DataDef::EventPulseTimes>;

  static constexpr Tof_t Tof{};
  static constexpr PulseTime_t PulseTime{};
  static constexpr Value_t Value{};
  static constexpr Variance_t Variance{};
  static constexpr StdDev_t StdDev{};
  static constexpr DeprecatedInt_t DeprecatedInt{};
  static constexpr DeprecatedString_t DeprecatedString{};
  static constexpr Events_t Events{};
  static constexpr EventTofs_t EventTofs{};
  static constexpr EventPulseTimes_t EventPulseTimes{};
};

struct Attr {
  using ExperimentLog_t = detail::TagImpl<detail::AttrDef::ExperimentLog>;

  static constexpr ExperimentLog_t ExperimentLog{};
};

template <class T>
static constexpr bool is_coord =
    T{} < std::tuple_size<detail::CoordDef::tags>::value;
template <class T>
static constexpr bool is_attr =
    T{} >= std::tuple_size<detail::CoordDef::tags>::value +
               std::tuple_size<detail::DataDef::tags>::value;
template <class T> static constexpr bool is_data = !is_coord<T> && !is_attr<T>;

// TODO Some things *may* be dimension coordinates, but they are not necessarily
// in all datasets. It depends on which dimensions are present. Does it even
// make sense to hard-code this? Maybe we require handling everything at
// runtime?
namespace detail {
template <class Tag> constexpr bool is_dimension_coordinate = false;
template <> constexpr bool is_dimension_coordinate<CoordDef::Tof> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::Energy> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::DeltaE> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::X> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::Y> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::Z> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::Position> = true;
template <>
constexpr bool is_dimension_coordinate<CoordDef::SpectrumNumber> = true;
template <> constexpr bool is_dimension_coordinate<CoordDef::RowLabel> = true;

template <class Tag> constexpr Dim coordinate_dimension = Dim::Invalid;
template <> constexpr Dim coordinate_dimension<CoordDef::Tof> = Dim::Tof;
template <> constexpr Dim coordinate_dimension<CoordDef::Energy> = Dim::Energy;
template <> constexpr Dim coordinate_dimension<CoordDef::DeltaE> = Dim::DeltaE;
template <> constexpr Dim coordinate_dimension<CoordDef::X> = Dim::X;
template <> constexpr Dim coordinate_dimension<CoordDef::Y> = Dim::Y;
template <> constexpr Dim coordinate_dimension<CoordDef::Z> = Dim::Z;
template <>
constexpr Dim coordinate_dimension<CoordDef::Position> = Dim::Position;
template <>
constexpr Dim coordinate_dimension<CoordDef::SpectrumNumber> = Dim::Spectrum;
template <> constexpr Dim coordinate_dimension<CoordDef::RowLabel> = Dim::Row;
} // namespace detail

template <class... Ts>
constexpr std::array<bool, std::tuple_size<detail::Tags>::value>
make_is_dimension_coordinate(const std::tuple<Ts...> &) {
  return {detail::is_dimension_coordinate<Ts>...};
}

template <class... Ts>
constexpr std::array<Dim, std::tuple_size<detail::Tags>::value>
make_coordinate_dimension(const std::tuple<Ts...> &) {
  return {detail::coordinate_dimension<Ts>...};
}

// Coordinates linked to a specific dimension, e.g., Coord::X to Dim::X. See
// also the xarray documentation regarding its distinction getween "dimension
// coordinates" and "non-dimension coordinates".
constexpr auto isDimensionCoord = make_is_dimension_coordinate(detail::Tags{});
constexpr auto coordDimension = make_coordinate_dimension(detail::Tags{});

inline Tag dimensionCoord(const Dim dim) {
  switch (dim) {
  case Dim::X:
    return Coord::X;
  case Dim::Y:
    return Coord::Y;
  case Dim::Z:
    return Coord::Z;
  case Dim::Tof:
    return Coord::Tof;
  case Dim::Energy:
    return Coord::Energy;
  case Dim::DeltaE:
    return Coord::DeltaE;
  default:
    throw std::runtime_error(
        "Coordinate for this dimension is not implemented");
  }
}

namespace detail {
template <class... Ts>
constexpr std::array<Unit::Id, std::tuple_size<detail::Tags>::value>
make_unit_table(const std::tuple<Ts...> &) {
  return {Ts::unit...};
}
template <class... Ts>
constexpr std::array<DType, std::tuple_size<detail::Tags>::value>
make_dtype_table(const std::tuple<Ts...> &) {
  return {dtype<typename Ts::type>...};
}
constexpr auto unit_table = make_unit_table(detail::Tags{});
constexpr auto dtype_table = make_dtype_table(detail::Tags{});
} // namespace detail

/// Return the default unit for a runtime tag.
constexpr auto defaultUnit(const Tag tag) {
  return detail::unit_table[tag.value()];
}
/// Return the default dtype for a runtime tag.
constexpr auto defaultDType(const Tag tag) {
  return detail::dtype_table[tag.value()];
}

class DataBin {
public:
  DataBin(const double left, const double right)
      : m_left(left), m_right(right) {}

  double center() const { return 0.5 * (m_left + m_right); }
  double width() const { return m_right - m_left; }
  double left() const { return m_left; }
  double right() const { return m_right; }

private:
  double m_left;
  double m_right;
};

template <class T> struct Bin { using type = DataBin; };

// std::vector<bool> may have a packed non-thread-safe implementation which we
// need to avoid. Therefore we use std::vector<Bool> instead.
template <class T> struct underlying_type { using type = T; };
template <> struct underlying_type<bool> { using type = Bool; };
template <class T> using underlying_type_t = typename underlying_type<T>::type;

template <class D, class Tag> struct element_return_type {
  using T = underlying_type_t<typename Tag::type>;
  using type = std::conditional_t<
      std::is_base_of<detail::ReturnByValuePolicy, Tag>::value, T,
      std::conditional_t<
          std::is_const<D>::value || std::is_const<Tag>::value,
          std::conditional_t<
              std::is_base_of<detail::ReturnByValueIfConstPolicy, Tag>::value,
              T, const T &>,
          T &>>;
};

template <class D, class Tags> struct element_return_type<D, Bin<Tags>> {
  using type = DataBin;
};

class EventListProxy;
template <class D> struct element_return_type<D, Data::Events_t> {
  using type = EventListProxy;
};

template <class D, class... Ts> class MDZipViewImpl;
template <class D, class... Tags>
struct element_return_type<D, MDZipViewImpl<D, Tags...>> {
  using type = MDZipViewImpl<D, Tags...>;
};

template <class D, class Tag>
using element_return_type_t = typename element_return_type<D, Tag>::type;

#endif // TAGS_H
