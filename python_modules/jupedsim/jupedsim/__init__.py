try:
    import jupedsim.py_jupedsim as jps_native
except ModuleNotFoundError:
    import py_jupedsim as jps_native

print(jps_native)
import jupedsim.aabb
import jupedsim.distributions
import jupedsim.grid
import jupedsim.serialization
import jupedsim.trajectory_writer_sqlite
import jupedsim.util
