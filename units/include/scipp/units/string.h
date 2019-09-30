#ifndef SCIPP_UNITS_STRING_H
#define SCIPP_UNITS_STRING_H
#include <string>

#include "scipp-units_export.h"
#include "scipp/units/unit.h"

namespace scipp::units {

SCIPP_UNITS_EXPORT std::string to_string(const units::Unit &unit);

} // namespace scipp::units

#endif // SCIPP_UNITS_STRING_H