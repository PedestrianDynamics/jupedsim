try:
    from .py_jupedsim import *
except ModuleNotFoundError:
    from py_jupedsim import *

import jupedsim.aabb
import jupedsim.distributions
import jupedsim.grid
import jupedsim.serialization
import jupedsim.trajectory_writer_sqlite
import jupedsim.util
