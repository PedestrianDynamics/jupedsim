import time

import pytest
from jupedsim.models.collision_free_speed import CollisionFreeSpeedModel


def test_timer_integration_small_simulation(tmp_path):
    """
    Integration test: exercise the real C++ Timer API exposed via jupedsim.native.
    """
    # Build a Simulation with a real model to access the C++-backed Simulation object
    try:
        from jupedsim.simulation import Simulation
    except Exception:
        pytest.fail("jupedsim python bindings not importable or models missing")

    # Minimal geometry: small square
    geom = [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
    sim = Simulation(model=CollisionFreeSpeedModel(), geometry=geom, dt=0.01)

    # underlying C++ simulation object exposed as _obj
    timer = sim.timer
    timer.push_timer("integration_test")

    time.sleep(0.001)
    timer.pop_timer("integration_test")

    dur = timer.elapsed_time_us("integration_test")
    assert isinstance(dur, int)
    assert dur >= 0

    @timer.timer_event
    def sleep():
        time.sleep(0.001)

    sleep()

    with timer.timer_event("test_region"):
        time.sleep(0.001)

    s = str(timer)

    assert "integration_test" in s
    assert "Total Simulation Time" in s
    assert "test_region" in s
    assert "sleep" in s


def test_profiler_integration_with_cpp_extension(tmp_path):
    """
    Integration test: exercise the real C++ Trace/Profiler API.
    """
    try:
        import jupedsim.internal.tracing as tracing
    except Exception:
        pytest.fail("jupedsim.native extension not importable")

    # enable/disable shouldn't raise
    tracing.enable_tracing()
    tracing.disable_tracing()

    # push/pop probes
    tracing.start_trace_event("integration_probe")

    time.sleep(0.001)
    tracing.end_trace_event()

    @tracing.trace_event
    def sleep():
        time.sleep(0.001)

    sleep()

    with tracing.trace_event("test_region"):
        time.sleep(0.001)

    # dump to a temp file; some backends may not write immediately but should accept the call
    out_file = tmp_path / "trace_out.ptrace"
    tracing.dump_traces(str(out_file))
