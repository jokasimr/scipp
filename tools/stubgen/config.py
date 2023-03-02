import ast
from pathlib import Path
from typing import List

# The name of the C++ module that contains the classes we generate stubs for.
CPP_CORE_MODULE_NAME = 'scipp._scipp.core'

# Template for the generated stub.
TEMPLATE_FILE = Path(__file__).resolve().parent / 'stub_template.py.template'

# Place the generated stub file here unless overridden by a command line argument.
DEFAULT_TARGET = (
    Path(__file__).resolve().parent.parent.parent
    / 'src'
    / 'scipp'
    / 'core'
    / 'cpp_classes.pyi'
)

# Select whether docstrings are included in the stub.
INCLUDE_DOCS = False

# Add a comment to disable type checks for ``override`` for these functions.
DISABLE_TYPE_CHECK_OVERRIDE = ('__eq__', '__ne__')

# Comment to place at the top of the generated stub file.
HEADER = '''############################################
#               ATTENTION                  #
# This file was generated by tools/stubgen #
# Do not edit!                             #
############################################'''


def class_is_excluded(name: str) -> bool:
    """Return True if a class stub should be omitted."""
    return name.startswith('ElementArrayView') or name.startswith('_')


def _squash_binary_more_narrow_type(
    overloads: List[ast.FunctionDef],
) -> List[ast.FunctionDef]:
    # A lot of binary functions have overloads for int and float.
    # But the former is superseded by the latter because int is more narrow than float.
    # Drop all int overloads in this case.
    rhs_types = []
    for overload in overloads:
        ann = overload.args.args[1].annotation
        if isinstance(ann, ast.Name):
            rhs_types.append(ann.id)
        else:
            rhs_types.append(ann)

    if 'int' in rhs_types and 'float' in rhs_types:
        squashed = list(overloads)
        del squashed[rhs_types.index('int')]
        return squashed
    return overloads


def squash_overloads(overloads: List[ast.FunctionDef]) -> List[ast.FunctionDef]:
    """Combine overloads if a stub for only one is required."""
    if overloads[0].name in ('__eq__', '__ne__'):
        return overloads[:1]
    if len(overloads[0].args.args) == 2:
        return _squash_binary_more_narrow_type(overloads)
    return overloads
