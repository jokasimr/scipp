# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
import numpy as np
import scipp as sc
from scipp.optimize import curve_fit

import pytest


def func(x, a, b):
    return a * sc.exp(-(b / x.unit) * x)


def array1d(*, a=1.2, b=1.3, noise_scale=0.1, size=50):
    x = sc.linspace(dim='xx', start=-0.1, stop=4.0, num=size, unit='m')
    y = func(x, a, b)
    rng = np.random.default_rng()
    # Noise is random but avoiding unbounded values to avoid flaky tests
    y.values += noise_scale * np.clip(rng.normal(size=size), -2.0, 2.0)
    return sc.DataArray(y, coords={'xx': x})


def test_should_not_raise_given_function_with_dimensionless_params_and_1d_array():
    curve_fit(func, array1d())


def test_should_raise_TypeError_when_xdata_given_as_param():
    with pytest.raises(TypeError):
        curve_fit(func, array1d(), xdata=np.arange(4))


def test_should_raise_TypeError_when_ydata_given_as_param():
    with pytest.raises(TypeError):
        curve_fit(func, array1d(), ydata=np.arange(4))


def test_should_raise_TypeError_when_sigma_given_as_param():
    with pytest.raises(TypeError):
        curve_fit(func, array1d(), sigma=np.arange(4))


def test_should_raise_NotFoundError_when_data_array_has_no_coord():
    da = array1d()
    del da.coords[da.dim]
    with pytest.raises(sc.NotFoundError):
        curve_fit(func, da)


def test_should_raise_BinEdgeError_when_data_array_is_histogram():
    da = array1d()
    hist = da[1:].copy()
    hist.coords[hist.dim] = da.coords[hist.dim]
    with pytest.raises(sc.BinEdgeError):
        curve_fit(func, hist)


def test_should_raise_DimensionError_when_data_array_is_not_1d():
    da = sc.concat([array1d(), array1d()], 'extra_dim')
    with pytest.raises(sc.DimensionError):
        curve_fit(func, da)


def test_masks_are_not_ignored():
    da = array1d(size=20)
    unmasked, _ = curve_fit(func, da)
    da.masks['mask'] = sc.zeros(sizes=da.sizes, dtype=bool)
    da.masks['mask'][-5:] = sc.scalar(True)
    masked, _ = curve_fit(func, da)
    assert all(masked != unmasked)


@pytest.mark.parametrize("noise_scale", [1e-1, 1e-2, 1e-3, 1e-6, 1e-9])
def test_optimized_params_approach_real_params_as_data_noise_decreases(noise_scale):
    popt, _ = curve_fit(func, array1d(a=1.7, b=1.5, noise_scale=noise_scale))
    np.testing.assert_allclose(popt, [1.7, 1.5], rtol=2.0 * noise_scale)


@pytest.mark.parametrize("mask_pos", [0, 1, -3])
@pytest.mark.parametrize("mask_size", [1, 2])
def test_masked_points_are_treated_as_if_they_were_removed(mask_pos, mask_size):
    da = array1d(size=10)
    da.masks['mask'] = sc.zeros(sizes=da.sizes, dtype=bool)
    da.masks['mask'][mask_pos:mask_pos + mask_size] = sc.scalar(True)
    masked, _ = curve_fit(func, da)
    removed, _ = curve_fit(
        func, sc.concat([da[:mask_pos], da[mask_pos + mask_size:]], da.dim))
    assert all(masked == removed)


@pytest.mark.parametrize("variance,expected", [(1e9, 1.0), (1, 2.0), (1 / 3, 3.0),
                                               (1e-9, 5.0)],
                         ids=['disabled', 'equal', 'high', 'dominant'])
def test_variances_determine_weights(variance, expected):
    x = sc.array(dims=['x'], values=[1, 2, 3, 4])
    y = sc.array(dims=['x'], values=[1., 5., 1., 1.], variances=[1., 1., 1., 1.])
    da = sc.DataArray(data=y, coords={'x': x})
    da.variances[1] = variance
    # Fit a constant to highlight influence of weights
    popt, _ = curve_fit(lambda x, a: sc.scalar(a), da)
    assert popt[0] == pytest.approx(expected)


def test_fit_function_with_dimensionful_params_raises_UnitError_when_no_p0_given():
    def f(x, a, b):
        return a * sc.exp(-b * x)

    with pytest.raises(sc.UnitError):
        curve_fit(f, array1d())


def test_fit_function_with_dimensionful_params_yields_outputs_with_units():
    def f(x, a, b):
        return a * sc.exp(-b * x)

    x = sc.linspace(dim='x', start=0.5, stop=2.0, num=10, unit='m')
    da = sc.DataArray(f(x, 1.2, 1.3 / sc.Unit('m')), coords={'x': x})
    p0 = [1.1, 1.2 / sc.Unit('m')]
    popt, pcov = curve_fit(f, da, p0=p0)
    assert not isinstance(popt[0], sc.Variable)
    assert popt[1].unit == sc.Unit('1/m')
    assert not isinstance(pcov[0][0], sc.Variable)
    assert pcov[0][1].unit == sc.Unit('1/m')
    assert pcov[1][0].unit == sc.Unit('1/m')
    assert pcov[1][1].unit == sc.Unit('1/m**2')