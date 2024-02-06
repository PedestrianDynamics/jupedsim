# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
    from py_jupedsim import *
except ImportError:
    from .. import py_jupedsim as py_jps
    from ..py_jupedsim import *

__all__ = [name for name in dir(py_jps) if not name.startswith("__")]
