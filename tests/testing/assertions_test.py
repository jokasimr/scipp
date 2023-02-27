# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2023 Scipp contributors (https://github.com/scipp)
# @author Jan-Lukas Wynen
from copy import deepcopy

import numpy as np
import pytest

import scipp as sc
from scipp.testing import assert_identical


@pytest.mark.parametrize('a', (3, -1.2, 'hjh wed', [], {4}))
def test_assert_identical_builtin_equal(a):
    assert_identical(deepcopy(a), deepcopy(a))


@pytest.mark.parametrize('a', (3, -1.2, 'hjh wed', [], {4}))
@pytest.mark.parametrize('b', (1, 0.2, 'll', [7], {}))
def test_assert_identical_builtin_mismatch(a, b):
    with pytest.raises(AssertionError):
        assert_identical(a, b)


@pytest.mark.parametrize(
    'a', (sc.scalar(3), sc.scalar(7.12, variance=0.33, unit='m'), sc.scalar('jja ow=-'),
          sc.arange('u', 9.5, 13.0, 0.4), sc.linspace('ppl', 3.7, -99, 10, unit='kg'),
          sc.array(dims=['ww', 'gas'], values=[[np.nan], [3]])))
def test_assert_identical_variables_equal(a):
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_variables_dim_mismatch():
    a = sc.arange('rst', 5, unit='m')
    b = sc.arange('llf', 5, unit='m')
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.arange('t', 12, unit='m').fold('t', {'x': 3, 'k': 4})
    b = sc.arange('t', 12, unit='m').fold('t', {'x': 3, 't': 4})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_shape_mismatch():
    a = sc.arange('x', 5, unit='m')
    b = sc.arange('x', 6, unit='m')
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.arange('i', 5, unit='m')
    b = sc.arange('t', 10, unit='m').fold('t', {'x': 5, 'k': 2})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_unit_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='kg')
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_values_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='m') * 0.1
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_values_mismatch_nan():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='m')
    b[2] = np.nan
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_variances_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    a.variances = a.values
    b = sc.arange('u', 6.1, unit='m')
    b.variances = b.values * 1.2
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_variances_mismatch_nan():
    a = sc.arange('u', 6.1, unit='m')
    a.variances = a.values
    b = sc.arange('u', 6.1, unit='m')
    b.variances = b.values
    b.variances[1] = np.nan
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variables_presence_of_variances():
    a = sc.scalar(1.1, variance=0.1)
    b = sc.scalar(1.1)
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'a', (sc.DataArray(sc.scalar(
        91, unit='F')), sc.DataArray(sc.scalar(6.4), coords={'g': sc.scalar(4)}),
          sc.DataArray(sc.scalar(6.4), attrs={'rat': sc.scalar(0.01)}),
          sc.DataArray(sc.scalar(6.4), masks={'m': sc.scalar(False)}),
          sc.DataArray(
              sc.arange('y', 6.1, unit='mm'),
              coords={
                  'y': sc.arange('y', 6.1, unit='s') * 3,
                  'e': sc.arange('y', 8)
              },
              attrs={
                  'w': sc.scalar('wws'),
                  'qkk': sc.array(dims=['t'], values=[1.0, 2.0], variances=[0.0, 0.1])
              },
              masks={'1': sc.arange('y', 7) < 4})))
def test_assert_identical_data_arrays(a):
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_data_array_data_mismatch():
    a = sc.DataArray(sc.scalar(23), coords={'i': sc.scalar(-1)})
    b = sc.DataArray(sc.scalar(6.2), coords={'i': sc.scalar(-1)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_coords_key_mismatch():
    a = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33)})
    b = sc.DataArray(sc.scalar(-8), coords={'n': sc.scalar(33)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33)})
    b = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33), 't': sc.scalar(3)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_coords_value_mismatch():
    a = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33)})
    b = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(12)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33), 't': sc.scalar('yrr')})
    b = sc.DataArray(sc.scalar(-8), coords={'a': sc.scalar(33), 't': sc.scalar('yra')})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_attrs_key_mismatch():
    a = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(33)})
    b = sc.DataArray(sc.scalar(-8), attrs={'n': sc.scalar(33)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(33)})
    b = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(33), 't': sc.scalar(3)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_attrs_value_mismatch():
    a = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(82.0)})
    b = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(82)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(33), 't': sc.scalar('yrr')})
    b = sc.DataArray(sc.scalar(-8), attrs={'a': sc.scalar(33), 't': sc.scalar('yra')})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_masks_key_mismatch():
    a = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True)})
    b = sc.DataArray(sc.scalar(-8), masks={'n': sc.scalar(True)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True)})
    b = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True), 't': sc.scalar(False)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_array_masks_value_mismatch():
    a = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True)})
    b = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(False)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True), 't': sc.scalar(True)})
    b = sc.DataArray(sc.scalar(-8), masks={'a': sc.scalar(True), 't': sc.scalar(False)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)
