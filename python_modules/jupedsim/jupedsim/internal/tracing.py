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

    def timer_event(self, func=None, *, name=None):
        """Use as either a decorator or a context-manager factory for timers.

        Supported uses:
          @timer.timer_event
          def f(...): ...

          with timer.timer_event(name="custom-name"):
              ...

        This mirrors the behavior of `trace_event` for the profiler.
        """

        def decorator(fn):
            @functools.wraps(fn)
            def wrapper(*args, **kwargs):
                if name is None:
                    event_name = f"{fn.__name__}()"
                else:
                    event_name = f"{name}()"
                self.push_timer(event_name)
                try:
                    return fn(*args, **kwargs)
                finally:
                    self.pop_timer(event_name)

            return wrapper

        # Decorator used without arguments: @timer.timer_event
        if callable(func):
            return decorator(func)
        elif func is None:
            return decorator

        @contextmanager
        def timer_region():
            self.push_timer(name)
            try:
                yield
            finally:
                self.pop_timer(name)

        return timer_region()


def is_tracing_enabled() -> bool:
    """Check if the profiler is enabled."""
    return py_jps.Profiler().is_enabled


def enable_tracing() -> None:
    """Enable the profiler."""
    py_jps.Profiler().enable()


def disable_tracing() -> None:
    """Disable the profiler."""
    py_jps.Profiler().disable()


def start_trace_event(name: str) -> None:
    """Starts a named trace event."""
    py_jps.Profiler().start_trace_event(name)


def end_trace_event() -> None:
    """Ends the last started trace event."""
    py_jps.Profiler().end_trace_event()


def dump_traces(filename: str) -> None:
    """Dump traces to file."""
    py_jps.Profiler().dump_and_reset(filename)


def trace_event(func=None, *, name=None):
    """Use as either a decorator or a context-manager factory.

    Usages supported:
      @trace_event
      def f(...): ...

      @trace_event(name="custom-name")
      def f(...): ...

      with trace_event("initialisation"):
          ...
    """

    def decorator(fn):
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            if name is None:
                event_name = f"{fn.__name__}()"
            else:
                event_name = f"{name}()"
            start_trace_event(event_name)
            try:
                return fn(*args, **kwargs)
            finally:
                end_trace_event()

        return wrapper

    if callable(func):
        return decorator(func)
    elif func is None:
        return decorator

    @contextmanager
    def trace_region(name):
        start_trace_event(name)
        try:
            yield
        finally:
            end_trace_event()

    return trace_region(func)
