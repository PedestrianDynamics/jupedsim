# SPDX-License-Identifier: LGPL-3.0-or-later
try:
    import py_jupedsim as py_jps
    from py_jupedsim import *  # noqa: F403
except ImportError:
    from .. import py_jupedsim as py_jps
    from ..py_jupedsim import *  # noqa: F403

__all__ = [name for name in dir(py_jps) if not name.startswith("__")]
