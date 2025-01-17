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
    'a',
    (
        sc.scalar(3),
        sc.scalar(7.12, variance=0.33, unit='m'),
        sc.scalar('jja ow=-'),
        sc.arange('u', 9.5, 13.0, 0.4),
        sc.linspace('ppl', 3.7, -99, 10, unit='kg'),
        sc.array(dims=['ww', 'gas'], values=[[np.nan], [3]]),
    ),
)
def test_assert_identical_variable_equal(a):
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_variable_dim_mismatch():
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


def test_assert_identical_variable_shape_mismatch():
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


def test_assert_identical_variable_unit_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='kg')
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variable_values_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='m') * 0.1
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variable_values_mismatch_nan():
    a = sc.arange('u', 6.1, unit='m')
    b = sc.arange('u', 6.1, unit='m')
    b[2] = np.nan
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variable_variances_mismatch():
    a = sc.arange('u', 6.1, unit='m')
    a.variances = a.values
    b = sc.arange('u', 6.1, unit='m')
    b.variances = b.values * 1.2
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variable_variances_mismatch_nan():
    a = sc.arange('u', 6.1, unit='m')
    a.variances = a.values
    b = sc.arange('u', 6.1, unit='m')
    b.variances = b.values
    b.variances[1] = np.nan
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_variable_presence_of_variances():
    a = sc.scalar(1.1, variance=0.1)
    b = sc.scalar(1.1)
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'a',
    (
        sc.DataArray(sc.scalar(91, unit='F')),
        sc.DataArray(sc.scalar(6.4), coords={'g': sc.scalar(4)}),
        sc.DataArray(sc.scalar(6.4), attrs={'rat': sc.scalar(0.01)}),
        sc.DataArray(sc.scalar(6.4), masks={'m': sc.scalar(False)}),
        sc.DataArray(
            sc.arange('y', 6.1, unit='mm'),
            coords={'y': sc.arange('y', 6.1, unit='s') * 3, 'e': sc.arange('y', 8)},
            attrs={
                'w': sc.scalar('wws'),
                'qkk': sc.array(dims=['t'], values=[1.0, 2.0], variances=[0.0, 0.1]),
            },
            masks={'1': sc.arange('y', 7) < 4},
        ),
    ),
)
def test_assert_identical_data_arrays_equal(a):
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


@pytest.mark.parametrize(
    'a',
    (
        sc.Dataset(),
        sc.Dataset({'f': sc.scalar(91, unit='F')}),
        sc.Dataset({'r': sc.scalar(6.4)}, coords={'g': sc.scalar(4)}),
        sc.Dataset({'2': sc.arange('u', 5), '3': sc.arange('b', 7)}),
        sc.Dataset(
            {
                'yy': sc.DataArray(
                    sc.arange('yy', 5),
                    attrs={'a': sc.scalar([2, 3])},
                    masks={'m': sc.arange('yy', 5) < 2},
                ),
                'i': sc.arange('i', 3),
            },
            coords={'i': sc.arange('w', 15).fold('w', {'i': 3, 'yy': 5})},
        ),
    ),
)
def test_assert_identical_datasets_equal(a):
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_dataset_data_key_mismatch():
    a = sc.Dataset({'a': sc.arange('t', 3)})
    b = sc.Dataset({'b': sc.arange('t', 3)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_dataset_data_value_mismatch():
    a = sc.Dataset({'a': sc.arange('t', 3)})
    b = sc.Dataset({'a': -sc.arange('t', 3)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_dataset_coords_key_mismatch():
    a = sc.Dataset({'a': sc.arange('t', 3)}, coords={'t': sc.scalar(2)})
    b = sc.Dataset({'a': sc.arange('t', 3)}, coords={'j': sc.scalar(2)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)

    a = sc.Dataset(
        {'a': sc.arange('t', 3), 'b': sc.arange('c', 5)},
        coords={'t': sc.scalar(2), 'c': sc.arange('c', 5)},
    )
    b = sc.Dataset(
        {'a': sc.arange('t', 3), 'b': sc.arange('c', 5)},
        coords={'t': sc.scalar(2), 'c': sc.arange('c', 6)},
    )
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'a',
    (
        sc.DataGroup(),
        sc.DataGroup({'hg': 4}),
        sc.DataGroup({'ii': sc.scalar(3.01, unit='s')}),
        sc.DataGroup({'Å': [5, 2], 'pl': sc.arange(';', 6)}),
        sc.DataGroup({'q': sc.DataGroup({'ik': sc.arange('ik', 2)})}),
    ),
)
def test_assert_identical_data_group_equal(a):
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_data_group_keys_mismatch():
    a = sc.DataGroup({'a': sc.arange('ook', 9)})
    b = sc.DataGroup({'b': sc.arange('ook', 9)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_group_values_mismatch_variable():
    a = sc.DataGroup({'a': sc.arange('ook', 9)})
    b = sc.DataGroup({'a': sc.arange('wak', 9)})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_group_values_mismatch_builtin():
    a = sc.DataGroup({'a': sc.arange('ook', 9), 'gg': 'well'})
    b = sc.DataGroup({'a': sc.arange('ook', 9), 'gg': 'done'})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_data_group_values_mismatch_nested():
    a = sc.DataGroup({'a': sc.DataGroup({'rz': sc.arange('rz', 4)})})
    b = sc.DataGroup({'a': sc.DataGroup({'rz': -sc.arange('rz', 4)})})
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'buffer',
    (
        sc.ones(sizes={'e': 10}, unit='counts'),
        sc.arange('.', 13, unit='m'),
        sc.DataArray(sc.zeros(sizes={'bm': 10}), coords={'bm': sc.arange('bm', 10)}),
    ),
)
@pytest.mark.parametrize(
    'indices',
    (
        sc.array(dims=['lm'], values=[0, 3, 8, 10], dtype='int64', unit=None),
        sc.array(dims=['h'], values=[2, 4, 10], dtype='int64', unit=None),
        sc.array(dims=['lka'], values=[0, 6], dtype='int64', unit=None),
        sc.array(dims=['l we'], values=[], dtype='int64', unit=None),
    ),
)
def test_assert_identical_binned_variable_equal(buffer, indices):
    begin, end = indices[:-1], indices[1:]
    a = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    assert_identical(deepcopy(a), deepcopy(a))


def test_assert_identical_binned_data_array_equal():
    da = sc.DataArray(sc.ones(sizes={'vv': 9}), coords={'vv': sc.arange('vv', 9)}).bin(
        vv=3
    )
    assert_identical(deepcopy(da), deepcopy(da))


@pytest.mark.parametrize(
    'buffer',
    (
        sc.ones(sizes={'e': 10}, unit='counts'),
        sc.DataArray(sc.zeros(sizes={'bm': 10}), coords={'bm': sc.arange('bm', 10)}),
    ),
)
@pytest.mark.parametrize(
    'indices',
    (
        sc.array(dims=['lm'], values=[0, 3, 8, 10], dtype='int64', unit=None),
        sc.array(dims=['l we'], values=[], dtype='int64', unit=None),
    ),
)
def test_assert_identical_binned_variable_unit_mismatch(buffer, indices):
    begin, end = indices[:-1], indices[1:]
    a = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    buffer = deepcopy(buffer)
    buffer.unit = 'm'
    b = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'buffer',
    (
        sc.ones(sizes={'e': 10}, unit='counts'),
        sc.DataArray(sc.zeros(sizes={'bm': 10}), coords={'bm': sc.arange('bm', 10)}),
    ),
)
@pytest.mark.parametrize(
    'indices',
    (
        sc.array(dims=['lm'], values=[0, 3, 8, 10], dtype='int64', unit=None),
        sc.array(dims=['l we'], values=[2, 6], dtype='int64', unit=None),
    ),
)
def test_assert_identical_binned_variable_value_mismatch(buffer, indices):
    begin, end = indices[:-1], indices[1:]
    a = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    buffer = deepcopy(buffer)
    buffer[3] = -1
    b = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


@pytest.mark.parametrize(
    'indices',
    (
        sc.array(dims=['lm'], values=[0, 3, 8, 10], dtype='int64', unit=None),
        sc.array(dims=['l we'], values=[1, 6], dtype='int64', unit=None),
    ),
)
def test_assert_identical_binned_variable_coord_mismatch(indices):
    buffer = sc.DataArray(
        sc.zeros(sizes={'bm': 10}), coords={'bm': sc.arange('bm', 10)}
    )
    begin, end = indices[:-1], indices[1:]
    a = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    buffer = deepcopy(buffer)
    buffer.coords['bm'][1] = -10
    b = sc.bins(data=buffer, dim=buffer.dim, begin=begin, end=end)
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_binned_data_array_value_mismatch():
    a = sc.DataArray(
        sc.ones(sizes={'rv': 9}), coords={'rv': sc.arange('rv', 9), 'j': sc.scalar(3.1)}
    ).bin(rv=3)
    b = deepcopy(a)
    b.bins.constituents['data'][7] = -1.2
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_binned_data_array_event_coord_mismatch():
    a = sc.DataArray(
        sc.ones(sizes={'rv': 9}), coords={'rv': sc.arange('rv', 9), 'j': sc.scalar(3.1)}
    ).bin(rv=3)
    b = deepcopy(a)
    b.bins.constituents['data'].coords['rv'][5] = 0.2
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)


def test_assert_identical_binned_data_array_bin_coord_mismatch():
    a = sc.DataArray(
        sc.ones(sizes={'rv': 9}), coords={'rv': sc.arange('rv', 9), 'j': sc.scalar(3.1)}
    ).bin(rv=3)
    b = deepcopy(a)
    b.coords['j'].unit = 'm'
    with pytest.raises(AssertionError):
        assert_identical(a, b)
    with pytest.raises(AssertionError):
        assert_identical(b, a)
