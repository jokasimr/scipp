# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)

from .. import config
from .. import broadcast, DataArray
from .figure2d import Figure2d
# from .toolbar import Toolbar2d
from .tools import find_limits, fix_empty_range
from ..utils import name_with_unit
from .view import View

from functools import reduce
from matplotlib.colors import Normalize, LogNorm
import matplotlib.pyplot as plt
from typing import Any, Tuple


class View2d(View):
    """
    Class for 2 dimensional plots.
    """
    def __init__(self, **kwargs):

        super().__init__(figure=Figure2d, **kwargs)

        self.toolbar.add_button(name="home_view",
                                callback=self.figure.home_view,
                                icon="home",
                                tooltip="Reset original view")
        self.toolbar.add_togglebutton(name="pan_view",
                                      callback=self.figure.pan_view,
                                      icon="arrows",
                                      tooltip="Pan")
        self.toolbar.add_togglebutton(name="zoom_view",
                                      callback=self.figure.zoom_view,
                                      icon="square-o",
                                      tooltip="Zoom")
        self.toolbar.add_button(name="rescale_to_data",
                                callback=self.figure.rescale_to_data,
                                icon="arrows-v",
                                tooltip="Rescale")
        self.toolbar.add_button(name="transpose",
                                callback=self.figure.transpose,
                                icon="retweet",
                                tooltip="Transpose")
        self.toolbar.add_togglebutton(name='toggle_xaxis_scale',
                                      callback=self.figure.toggle_xaxis_scale,
                                      description="logx")
        self.toolbar.add_togglebutton(name='toggle_yaxis_scale',
                                      callback=self.figure.toggle_yaxis_scale,
                                      description="logy")
        self.toolbar.add_togglebutton(name="toggle_norm",
                                      callback=self.figure.toggle_norm,
                                      description="log",
                                      tooltip="log(data)")
        self.toolbar.add_button(name="save_view",
                                callback=self.figure.save_view,
                                icon="save",
                                tooltip="Save")
