# SPDX-License-Identifier: LGPL-3.0-or-later

import functools
from contextlib import contextmanager

import jupedsim.native as py_jps


class Timer:
    """
    Timer can be used to measure the time spent in a code block. It is used
    internally to measure the time spent in different stages of the simulation.
    """

    def __init__(self, sim_object: py_jps.Simulation, timer_log_level: int = 0):
        self._obj = sim_object
        self._obj.set_timer_log_level(timer_log_level)
        self.push_timer("Total Simulation Time")
        self._timer_dict = None
        self._prev_iteration_time = 0
        self._prev_op_dec_time = 0

    def __repr__(self) -> str:
        """Prints the timer entries to the console. Only the contributions with more than 0.01% are printed."""
        # even if the timer has been stopped this should not change things
        self._obj.pop_timer("Total Simulation Time")
        timer_dict = self._obj.get_durations()
        if timer_dict.get("Total Simulation Time") is not None:
            ref = timer_dict["Total Simulation Time"]
            timer_dict.pop("Total Simulation Time")
        else:
            ref = timer_dict.get("Total Iteration")
            timer_dict.pop("Total Iteration")
        out = f"JuPedSim Timings:\n{'-' * 53}\n"
        for name, duration in timer_dict.items():
            if duration / ref * 100 < 0.01:
                continue
            out += f"{name:<30}| {duration / 1000000:8.2f} s ({duration / ref * 100:5.2f}%)\n"

        out += "-----------------------------------------------------\n"
        out += f"{('Total Simulation Time'):<30}| {ref / 1000000:8.2f} s\n"
        out += ""
        return out

    def elapsed_time_us(self, key: str) -> int:
        """
        Returns:
            Elapsed time in microseconds.
        """
        return self._obj.get_duration(key)

    def push_timer(self, name: str, probe_log_level: int = 0) -> None:
        """
        Pushes a timer with the given name. The timer will be stopped when the corresponding pop_timer is called.

        Args:
            name: Name of the timer to be pushed.
            probe_log_level: Log level for the probe.
        """
        self._obj.push_timer(name, probe_log_level)

    def pop_timer(self, name: str) -> None:
        """
        Pops a timer with the given name. The timer will be stopped and the elapsed time will be recorded.

        Args:
            name: Name of the timer to be popped.
        """
        self._obj.pop_timer(name)

    @property
    def iteration_duration_us(self) -> int:
        """
        Returns:
            Elapsed time of the iteration in microseconds.
        """
        current_iteration_time = self._obj.get_duration("Total Iteration")
        iteration_duration = current_iteration_time - self._prev_iteration_time
        self._prev_iteration_time = current_iteration_time
        return iteration_duration

    @property
    def operational_level_duration_us(self) -> int:
        """
        Returns:
            Elapsed time per iteration of the operational level in microseconds.
        """
        current_op_dec_time = self._obj.get_duration(
            "Operational Decision System"
        )
        op_dec_duration = current_op_dec_time - self._prev_op_dec_time
        self._prev_op_dec_time = current_op_dec_time
        return op_dec_duration

    def set_timer_instance(self, timer_object: py_jps.Trace) -> None:
        """
        Sets the timer object to be used for tracing. This can be used to set a custom timer object that is not the default one.

        Args:
            timer_object: Timer object to be used for tracing.
        """
        self._obj = timer_object


class _ProfilerProxy:
    """Thin proxy around the C++ Profiler singleton.

    We expose a single module-level instance `profiler` so code can just
    `from jupedsim.internal.tracing import profiler` and call methods on it.
    """

    def __init__(self):
        # Prefer the module-level `trace` object if available to avoid the
        # extra C++ call. Fall back to Trace.instance() for older builds.
        self._profiler = py_jps.Trace.instance()


# Single module-level profiler instance. Importers can use this directly:
# from jupedsim.internal.tracing import profiler
profiler = _ProfilerProxy()


def enable_tracing() -> None:
    """Enable the profiler."""
    profiler._profiler.enable()


def disable_tracing() -> None:
    """Disable the profiler."""
    profiler._profiler.disable()


def push_probe(name: str) -> None:
    """Push a named probe."""
    if not profiler._profiler.is_enabled():
        profiler._profiler.enable()  # auto-enable if not already enabled
    profiler._profiler.push_probe(name)


def pop_probe() -> None:
    """Pop the last pushed probe."""
    profiler._profiler.pop_probe()


def dump_traces(filename: str) -> None:
    """Dump traces to file."""
    profiler._profiler.dump_and_reset(filename)


def trace_event(arg=None):
    """Use as either a decorator or a context-manager factory.

    Usages supported:
      @trace_event
      def f(...): ...

      @trace_event("custom-name")
      def f(...): ...

      with trace_event("initialisation"):
          ...
    """

    # Decorator used without arguments: @trace_event
    if callable(arg):
        fn = arg

        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            push_probe(f"{fn.__name__}()")
            try:
                return fn(*args, **kwargs)
            finally:
                pop_probe()

        return wrapper

    # If arg is a string, we either return a context-manager instance when
    # called (so `with trace_event("name"):` works), or return a decorator
    # if this function is later used as `@trace_event("name")`.
    name = arg

    def decorator(fn):
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            push_probe(name or f"{fn.__name__}()")
            try:
                return fn(*args, **kwargs)
            finally:
                pop_probe()

        return wrapper

    if isinstance(name, str):
        # Return a context-manager instance for `with trace_event("name"):`
        @contextmanager
        def _cm():
            push_probe(name)
            try:
                yield
            finally:
                pop_probe()

        return _cm()

    # No argument provided but parentheses used: @trace_event() -> return decorator
    return decorator
