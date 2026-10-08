# SPDX-License-Identifier: LGPL-3.0-or-later
try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps

# Re-export every non-dunder native symbol as a module attribute so wrapper code
# can reach internal transport types (e.g. py_jps._PythonModel, _CustomModelState).
_names = [name for name in dir(py_jps) if not name.startswith("__")]
globals().update({name: getattr(py_jps, name) for name in _names})
# Keep internal (single-underscore) names out of the public surface / star-imports.
__all__ = [name for name in _names if not name.startswith("_")]

# Static re-export of the public native API. Redundant at runtime with the
# globals() update above, but sphinx-autoapi analyses source without running
# it and needs this line to resolve `from jupedsim.native import X`.
from jupedsim.py_jupedsim import *  # noqa: E402, F403
