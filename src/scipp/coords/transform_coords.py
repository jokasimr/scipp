# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Simon Heybrock, Jan-Lukas Wynen

from typing import Dict, Iterable, List, Mapping, Set, Union

from ..core import DataArray, Dataset, NotFoundError, VariableError, bins
from ..logging import get_logger
from .coord_table import Coord, CoordTable, Destination
from .graph import Graph, GraphDict, rule_sequence
from .options import Options
from .rule import ComputeRule, FetchRule, RenameRule, Rule, rule_output_names


def transform_coords(x: Union[DataArray, Dataset],
                     targets: Union[str, Iterable[str]],
                     graph: GraphDict,
                     *,
                     rename_dims: bool = True,
                     keep_aliases: bool = True,
                     keep_intermediate: bool = True,
                     keep_inputs: bool = True) -> Union[DataArray, Dataset]:
    """Compute new coords based on transformations of input coords.

    :param x: Input object with coords.
    :param targets: Name or list of names of desired output coords.
    :param graph: A graph defining how new coords can be computed from existing
                  coords. This may be done in multiple steps.
                  The graph is given by a ``dict`` where:

                  - Dict keys are ``str`` or ``tuple`` of ``str``, defining the
                    names of outputs generated by a dict value.
                  - Dict values are ``str`` or a callable (function). If ``str``,
                    this is a synonym for renaming a coord. If a callable,
                    it must either return a single variable or a dict of
                    variables. The argument names of callables must be coords
                    in ``x`` or be computable by other nodes in ``graph``.
    :param rename_dims: Rename dimensions if the corresponding dimension coords
                        are used as inputs. `Dimension` ``a`` is renamed to ``b``
                        if and only if

                        - `coord` ``a`` is used as input in exactly 1 node
                        - `coord` ``a`` is the only dimension coord in
                          that node's inputs
                        - `coord` ``b`` is the only output of that node.

                        Default is True.
    :param keep_aliases: If True, aliases for coords defined in graph are
                         included in the output. Default is True.
    :param keep_intermediate: Keep attributes created as intermediate results.
                              Default is True.
    :param keep_inputs: Keep consumed input coordinates or attributes.
                        Default is True.
    :return: New object with desired coords. Existing data and meta-data is
             shallow-copied.

    :seealso: The section in the user guide on
     `Coordinate transformations <../../user-guide/coordinate-transformations.rst>`_
    """
    options = Options(rename_dims=rename_dims,
                      keep_aliases=keep_aliases,
                      keep_intermediate=keep_intermediate,
                      keep_inputs=keep_inputs)
    targets = {targets} if isinstance(targets, str) else set(targets)
    if isinstance(x, DataArray):
        return _transform_data_array(x,
                                     targets=targets,
                                     graph=Graph(graph),
                                     options=options)
    else:
        return _transform_dataset(x,
                                  targets=targets,
                                  graph=Graph(graph),
                                  options=options)


def show_graph(graph: GraphDict, size: str = None, simplified: bool = False):
    """
    Show graphical representation of a graph as required by
    :py:func:`transform_coords`

    Requires `python-graphviz` package.

    :param graph: Transformation graph to show.
    :param size: Size forwarded to graphviz, must be a string, "width,height"
                 or "size". In the latter case, the same value is used for
                 both width and height.
    :param simplified: If ``True``, do not show the conversion functions,
                       only the potential input and output coordinates.
    """
    return Graph(graph).show(size=size, simplified=simplified)


def _transform_data_array(original: DataArray, targets: Set[str], graph: Graph,
                          options: Options) -> DataArray:
    rules = rule_sequence(graph.graph_for(original, targets))
    dim_name_changes = (_dim_name_changes(rules, original.dims)
                        if options.rename_dims else {})
    working_coords = CoordTable(rules, targets, options)
    _log_plan(rules, targets, dim_name_changes, working_coords)
    for rule in rules:
        for name, coord in rule(working_coords).items():
            working_coords.add(name, coord)

    res = _store_results(original, working_coords, targets)
    return res.rename_dims(dim_name_changes)


def _transform_dataset(original: Dataset, targets: Set[str], graph: Graph, *,
                       options: Options) -> Dataset:
    # Note the inefficiency here in datasets with multiple items: Coord
    # transform is repeated for every item rather than sharing what is
    # possible. Implementing this would be tricky and likely error-prone,
    # since different items may have different attributes. Unless we have
    # clear performance requirements we therefore go with the safe and
    # simple solution
    return Dataset(
        data={
            name: _transform_data_array(
                original[name], targets=targets, graph=graph, options=options)
            for name in original
        })


def _log_plan(rules: List[Rule], targets: Set[str], dim_name_changes: Mapping[str, str],
              coords: CoordTable) -> None:
    inputs = set(rule_output_names(rules, FetchRule))
    byproducts = {
        name
        for name in (set(rule_output_names(rules, RenameRule))
                     | set(rule_output_names(rules, ComputeRule))) - targets
        if coords.total_usages(name) < 0
    }

    message = f'Transforming coords ({", ".join(sorted(inputs))}) ' \
              f'-> ({", ".join(sorted(targets))})'
    if byproducts:
        message += f'\n  Byproducts:\n    {", ".join(sorted(byproducts))}'
    if dim_name_changes:
        dim_rename_steps = '\n'.join(f'    {t} <- {f}'
                                     for f, t in dim_name_changes.items())
        message += '\n  Rename dimensions:\n' + dim_rename_steps
    message += '\n  Steps:\n' + '\n'.join(
        f'    {rule}' for rule in rules if not isinstance(rule, FetchRule))

    get_logger().info(message)


def _store_coord(da: DataArray, name: str, coord: Coord) -> None:
    def out(x):
        return x.coords if coord.destination == Destination.coord else x.attrs

    def del_other(x):
        try:
            if coord.destination == Destination.coord:
                del x.attrs[name]
            else:
                del x.coords[name]
        except NotFoundError:
            pass

    if coord.has_dense:
        out(da)[name] = coord.dense
        del_other(da)
    if coord.has_event:
        try:
            out(da.bins)[name] = coord.event
        except VariableError:
            # Thrown on mismatching bin indices, e.g. slice
            da.data = da.data.copy()
            out(da.bins)[name] = coord.event
        del_other(da.bins)


def _store_results(da: DataArray, coords: CoordTable, targets: Set[str]) -> DataArray:
    da = da.copy(deep=False)
    if da.bins is not None:
        da.data = bins(**da.bins.constituents)
    for name, coord in coords.items():
        if name in targets:
            coord.destination = Destination.coord
        _store_coord(da, name, coord)
    return da


# A coord's dim can be renamed if its node
#  1. has one incoming dim coord
#  2. has only one outgoing connection
#
# This functions traversed the graph in depth-first order
# and builds a dict of old->new names according to the conditions above.
def _dim_name_changes(rules: List[Rule], dims: List[str]) -> Dict[str, str]:
    dim_coords = tuple(name for name in rule_output_names(rules, FetchRule)
                       if name in dims)
    pending = list(dim_coords)
    incoming_dim_coords = {name: [name] for name in pending}
    name_changes = {}
    while pending:
        name = pending.pop(0)
        if len(incoming_dim_coords[name]) != 1:
            continue  # Condition 1.
        dim = incoming_dim_coords[name][0]
        name_changes[dim] = name
        outgoing = _rules_with_dep(name, rules)
        if len(outgoing) == 1 and len(outgoing[0].out_names) == 1:
            # Potential candidate according to condition 2.
            pending.append(outgoing[0].out_names[0])
        for rule in filter(lambda r: len(r.out_names) == 1, outgoing):
            # Condition 2. is not satisfied for these children but we
            # still need to take the current node into account for 1.
            incoming_dim_coords.setdefault(rule.out_names[0], []).append(dim)
    return name_changes


def _rules_with_dep(dep, rules):
    return list(filter(lambda r: dep in r.dependencies, rules))
