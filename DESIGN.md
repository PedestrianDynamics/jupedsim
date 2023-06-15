# Design Documentation

## Artefact Overview

```txt
                                                                   ┌───────────┐
Executable                                                         │  jpsvis   │
                                                                   └───────────┘

           ──────────────────────────────────────────────────────────────────────
           ┌─────────────────┐                ┌─────────────────┐
Python     │   py_jupedsim   │     <uses>     │    jupedsim     │
           │  <native code>  │ ◄────────────  │  <pure python>  │
           └─────────┬───────┘                └─────────────────┘
                     │
           ──────────┼───────────────────────────────────────────────────────────
                     │
           ┌─────────▼───────┐
C-API      │   libjupedsim   │ (C wrapper around libsimulator)
           └─────────┬───────┘
                     │
           ──────────┼───────────────────────────────────────────────────────────
                     │
           ┌─────────▼───────┐
C++        │   libsimulator  │ (Actual simulation library)
           └─────────────────┘
```

### `jpsvis`

This is the "legacy" results viewer. This application consumes old XML based
geometry files and the old trajectory format.

Currently there are no tools available to export simulation geometry in this
old format. Since it is not 100% decided how to progress with this application
no concrete changes are planned.

### `libsimulator`

This C++ library implements the core simulation and models. This library is an
internal artefact and not supposed to be used directly. No guarantees are made
w.r.t API stability.

### `libjupedsim`

This is the public interface to the simulation library.

### `py_jupedsim`

This is the public interface to the simulation wrapped in python.

