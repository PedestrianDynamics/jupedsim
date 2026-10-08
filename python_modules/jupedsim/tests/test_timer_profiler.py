import time

import jupedsim as jps
import jupedsim.py_jupedsim as native
import pytest
from jupedsim.models.collision_free_speed import CollisionFreeSpeedModel

TRACING_FUNCTIONS = [
    "enable_tracing",
    "disable_tracing",
    "is_tracing_enabled",
    "start_trace_event",
    "end_trace_event",
    "dump_traces",
]


@pytest.mark.parametrize("name", TRACING_FUNCTIONS)
def test_tracing_functions_are_native(name):
    assert getattr(jps, name) is getattr(native, name)
    assert getattr(jps, name).__doc__
    assert name in jps.__all__


def test_is_tracing_enabled_follows_enable_and_disable():
    jps.enable_tracing()
    try:
        assert jps.is_tracing_enabled()
    finally:
        jps.disable_tracing()
    assert not jps.is_tracing_enabled()


def test_profiler_class_is_gone():
    assert not hasattr(native, "Profiler")


def test_trace_event_balanced_when_body_raises(monkeypatch, tmp_path):
    import jupedsim.internal.tracing as tracing

    calls = []
    start = tracing.py_jps.start_trace_event
    end = tracing.py_jps.end_trace_event

    def record_start(name):
        calls.append("start")
        start(name)

    def record_end():
        calls.append("end")
        end()

    monkeypatch.setattr(tracing.py_jps, "start_trace_event", record_start)
    monkeypatch.setattr(tracing.py_jps, "end_trace_event", record_end)

    @tracing.trace_event
    def fails():
        raise RuntimeError("decorated")

    jps.enable_tracing()
    try:
        with pytest.raises(RuntimeError, match="decorated"):
            fails()
        with pytest.raises(RuntimeError, match="region"):
            with tracing.trace_event("region"):
                raise RuntimeError("region")
        assert calls == ["start", "end", "start", "end"]
        assert jps.is_tracing_enabled()
        out_file = tmp_path / "trace_out.ptrace"
        jps.dump_traces(str(out_file))
    finally:
        jps.disable_tracing()
    assert out_file.stat().st_size > 0


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
        pytest.fail("jupedsim.internal.tracing not importable")

    # enable/disable shouldn't raise
    jps.enable_tracing()
    jps.disable_tracing()

    # push/pop probes
    jps.start_trace_event("integration_probe")

    time.sleep(0.001)
    jps.end_trace_event()

    @tracing.trace_event
    def sleep():
        time.sleep(0.001)

    sleep()

    with tracing.trace_event("test_region"):
        time.sleep(0.001)

    # dump to a temp file; some backends may not write immediately but should accept the call
    out_file = tmp_path / "trace_out.ptrace"
    jps.dump_traces(str(out_file))
