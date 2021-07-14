# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet
from functools import lru_cache
from .._scipp import core as sc
from .._shape import flatten
from .model1d import PlotModel1d
import numpy as np


def _planar_norm(a, b):
    return sc.sqrt(a * a + b * b)


class ScatterPointModel:
    """
    Model representing scattered data.
    """
    def __init__(self, *, positions, scipp_obj_dict, resolution):
        self._axes = ['z', 'y', 'x']
        scipp_obj_dict = {
            key: flatten(array,
                         dims=array.meta[positions].dims,
                         to='_'.join(array.meta[positions].dims))
            for key, array in scipp_obj_dict.items()
        }
        self._data_model = PlotModel1d(scipp_obj_dict=scipp_obj_dict,
                                       resolution=resolution)
        array = next(iter(scipp_obj_dict.values()))
        # TODO Get dim labels from field names
        self._scatter_dims = ['x', 'y', 'z']
        self._positions = array.meta[positions]
        self._components = {'x': self.x, 'y': self.y, 'z': self.z}

    @property
    def dims(self):
        return self._scatter_dims

    @property
    def positions(self):
        return self._positions

    @property
    def unit(self):
        return self._positions.unit

    @property
    @lru_cache(maxsize=None)
    def limits(self):
        """
        Extents of the box that contains all the positions.
        """
        extents = {}
        pos = self._positions.fields
        for xyz, x in zip(self._axes, [pos.z, pos.y, pos.x]):
            xmin = sc.min(x).value
            xmax = sc.max(x).value
            extents[xyz] = np.array([xmin, xmax])
        return extents

    @property
    @lru_cache(maxsize=None)
    def center(self):
        return np.array([0.5 * np.sum(self.limits[dim]) for dim in 'xyz'])

    @property
    @lru_cache(maxsize=None)
    def box_size(self):
        return np.array([
            self.limits['x'][1] - self.limits['x'][0],
            self.limits['y'][1] - self.limits['y'][0],
            self.limits['z'][1] - self.limits['z'][0]
        ])

    # TODO replace x,y,z? use dims
    @property
    @lru_cache(maxsize=None)
    def components(self):
        return self._components

    @property
    @lru_cache(maxsize=None)
    def x(self):
        return self._positions.fields.x

    @property
    @lru_cache(maxsize=None)
    def y(self):
        return self._positions.fields.y

    @property
    @lru_cache(maxsize=None)
    def z(self):
        return self._positions.fields.z

    @property
    @lru_cache(maxsize=None)
    def radius_x(self):
        return _planar_norm(self._positions.fields.y, self._positions.fields.z)

    @property
    @lru_cache(maxsize=None)
    def radius_y(self):
        return _planar_norm(self._positions.fields.x, self._positions.fields.z)

    @property
    @lru_cache(maxsize=None)
    def radius_z(self):
        return _planar_norm(self._positions.fields.x, self._positions.fields.y)

    @property
    @lru_cache(maxsize=None)
    def radius(self):
        return sc.norm(self._positions)

    def __getattr__(self, attr):
        """
        Forward some methods from internal PlotModel1d.
        """
        return getattr(self._data_model, attr)
