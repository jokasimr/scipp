# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)

from .. import config, units
from .. import concat, values, scalar, histogram, full_like, Variable, DataArray
from .. import abs as abs_, flatten, ones
from ..typing import MetaDataMap

import numpy as np
from numpy.typing import ArrayLike
from typing import Any, Dict, Tuple


def to_bin_centers(x: Variable, dim: str) -> Variable:
    """
    Convert array edges to centers
    """
    return 0.5 * (x[dim, 1:] + x[dim, :-1])


def to_bin_edges(x: Variable, dim: str) -> Variable:
    """
    Convert array centers to edges
    """
    idim = x.dims.index(dim)
    if x.shape[idim] < 2:
        one = scalar(1.0, unit=x.unit)
        return concat([x[dim, 0:1] - one, x[dim, 0:1] + one], dim)
    else:
        center = to_bin_centers(x, dim)
        # Note: use range of 0:1 to keep dimension dim in the slice to avoid
        # switching round dimension order in concatenate step.
        left = center[dim, 0:1] - (x[dim, 1] - x[dim, 0])
        right = center[dim, -1] + (x[dim, -1] - x[dim, -2])
        return concat([left, center, right], dim)


def vars_to_err(v: ArrayLike) -> ArrayLike:
    """
    Convert variances to errors.
    """
    with np.errstate(invalid="ignore"):
        v = np.sqrt(v)
    np.nan_to_num(v, copy=False)
    return v


def find_log_limits(x: Variable) -> Tuple[Variable, ...]:
    """
    To find log scale limits, we histogram the data between 1.0-30
    and 1.0e+30 and include only bins that are non-zero.
    """
    volume = np.product(x.shape)
    pixel = flatten(values(x.astype('float64')), to='pixel')
    weights = ones(dims=['pixel'], shape=[volume], unit='counts')
    hist = histogram(DataArray(data=weights, coords={'order': pixel}),
                     bins=Variable(dims=['order'],
                                   values=np.geomspace(1e-30, 1e30, num=61),
                                   unit=x.unit))
    # Find the first and the last non-zero bins
    inds = np.nonzero((hist.data > scalar(0.0, unit=units.counts)).values)
    ar = np.arange(hist.data.shape[0])[inds]
    # Safety check in case there are no values in range 1.0e-30:1.0e+30:
    # fall back to the linear method and replace with arbitrary values if the
    # limits are negative.
    if len(ar) == 0:
        [vmin, vmax] = find_linear_limits(x)
        if vmin.value <= 0.0:
            if vmax.value <= 0.0:
                vmin = full_like(vmin, 0.1)
                vmax = full_like(vmax, 1.0)
            else:
                vmin = 1.0e-3 * vmax
    else:
        vmin = hist.coords['order']['order', ar.min()]
        vmax = hist.coords['order']['order', ar.max() + 1]
    return (vmin, vmax)


def find_linear_limits(x: Variable) -> Tuple[Variable, ...]:
    """
    Find variable finite min and max.
    TODO: If we implement finitemin and finitemax for Variable, we would no longer need
    to go via Numpy's isfinite.
    """
    v = x.values
    finite_vals = v[np.isfinite(v)]
    finite_min = np.amin(finite_vals)
    finite_max = np.amax(finite_vals)
    return (scalar(finite_min, unit=x.unit,
                   dtype='float64'), scalar(finite_max, unit=x.unit, dtype='float64'))


def find_limits(x: Variable,
                scale: str = None,
                flip: bool = False) -> Dict[str, Variable]:
    """
    Find sensible limits, depending on linear or log scale.
    """
    if scale is not None:
        if scale == "log":
            lims = {"log": find_log_limits(x)}
        else:
            lims = {"linear": find_linear_limits(x)}
    else:
        lims = {"log": find_log_limits(x), "linear": find_linear_limits(x)}
    if flip:
        for key in lims:
            lims[key] = np.flip(lims[key]).copy()
    return lims


def fix_empty_range(lims: Tuple[Variable, ...],
                    replacement: Variable = None) -> Tuple[Variable, ...]:
    """
    Range correction in case xmin == xmax
    """
    dx = scalar(0.0, unit=lims[0].unit)
    if lims[0].value == lims[1].value:
        if replacement is not None:
            dx = 0.5 * replacement
        elif lims[0].value == 0.0:
            dx = scalar(0.5, unit=lims[0].unit)
        else:
            dx = 0.5 * abs_(lims[0])
    return [lims[0] - dx, lims[1] + dx]


def to_dict(meta: MetaDataMap) -> Dict[str, Variable]:
    """
    Convert a coords, meta, attrs or masks object to a python dict.
    """
    return {name: var for name, var in meta.items()}
