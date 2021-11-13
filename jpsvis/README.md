# JPSVis

[![GitHub license](https://img.shields.io/badge/license-GPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpsvis/master/LICENSE)

## Showcase and tutorials

To highlight some features of JuPedSim we have uploaded some videos on our
[YouTube channel](https://www.youtube.com/channel/UCKS8w8CUClHEeN4K1SUSMBA).

## Requirements

For the visualization module (`jpsvis`) the following libraries are required.

| Tool | Version |
|------|---------|
| Qt   | >= 5.12 |
| VTK  | >= 9.0  |
| GLM  | >=0.9.9 |

### Optional Requirements [-DWITH\_BENCHMARKS]

If you would like to create micro benchmarks during development you will
additionally need google-benchmark.

On MacOS this is available as `google-benchmark`, on Windows via vcpkg as
`benchmark` and on Linux it is recommended to compile from source. See:
<https://github.com/google/benchmark>

### Install requirements for Mac OS X (with Homebrew)

We recommend using brew to install `jpsvis`. See [here](https://github.com/JuPedSim/homebrew-jps).

Nevertheless, to build the dependencies for the code:

```bash
brew update
brew install vtk --with-qt --without-python --with-python3 --without-boost  --build-from-source
```

### Install requirements for Ubuntu (21.04)

```bash
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libvtk9-dev
```

### Install requirements for Windows

To install the required libraries, we recommend using windows package manager [vcpkg](https://github.com/Microsoft/vcpkg)

```bash
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe install qt5:x64-windows vtk:x64-windows
```

## Compiling from sources

You can compile the simulation core for your specific platform with the
supplied cmake script.  The only requirement is a compiler supporting the new
standard c++17.

### Windows

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
cmake --build . --target jpsvis
```

(change `path/to` in the cmake call above accordingly).

### Linux

```bash
mkdir build && cd build
cmake ..
cmake --build . --target jpsvis
```

### OSX

```bash
mkdir build && cd build
cmake ..
cmake --build . --target jpsvis
```

### Additional Build Options

For additional build options please see "Build Options" section in
[CMakeLists.txt](CMakeLists.txt)

## Contributing

If you contribute code you need to be aware that all code needs to be formatted
with clang-format-13. This is enforced by the CI.
