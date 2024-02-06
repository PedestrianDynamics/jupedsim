# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import glob
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

min_cpp_standard = 20
min_cmake_version = "3.19"

# Read version number from CMakeLists.txt
with open("CMakeLists.txt", "r", encoding="utf-8") as cmakelist:
    cmake_input = cmakelist.read()
    version_line = re.findall(r"project\(JuPedSim.*", cmake_input)[0]
    start_index = version_line.rfind("VERSION") + len("VERSION")
    end_index = version_line.find("LANGUAGES")
    version = version_line[start_index:end_index].strip()

with open("pypi-readme.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


def check_cmake():
    try:
        result = subprocess.run(
            ["cmake", "--version"], check=True, capture_output=True, text=True
        )
        # Check CMake version
        found_cmake_version = re.search(
            r"(\d+).(\d+).(\d+)", str(result.stdout)
        )
        for min_version, found_version in zip(
            min_cmake_version.split("."), found_cmake_version.groups()
        ):
            if found_version < min_version:
                return False
    except:
        return False
    return True


def check_cpp_compiler():
    with tempfile.TemporaryDirectory() as tmp_dir:
        simple_main = textwrap.dedent(
            """
            int main(){ 
                return 0;
            }
            """
        )
        cpp_file = tmp_dir + "/main.cpp"
        with open(cpp_file, "w") as cp_file:
            cp_file.write(simple_main)

        simple_cmake = textwrap.dedent(
            f"""
            cmake_minimum_required(VERSION 3.19)
            project(SimpleTest)
            set(CMAKE_CXX_STANDARD {min_cpp_standard})
            set(CMAKE_CXX_STANDARD_REQUIRED ON)
            add_executable(simple_test main.cpp)
            """
        )
        cmake_file = tmp_dir + "/CMakeLists.txt"
        with open(cmake_file, "w") as cm_file:
            cm_file.write(simple_cmake)

        tmp_dir_build = pathlib.Path(tmp_dir) / "build"
        tmp_dir_build.mkdir()

        try:
            subprocess.run(
                ["cmake", "-S", str(tmp_dir), "-B", str(tmp_dir_build)],
                check=True,
            )
        except:
            return False
    return True


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        if not check_cmake():
            raise ModuleNotFoundError(
                f"No CMake or no CMake >= {min_cmake_version} installation "
                f"found on the system, please install "
                f"CMake >= {min_cmake_version} to install JuPedSim."
            )

        if not check_cpp_compiler():
            raise ModuleNotFoundError(
                "Could not compile a simple C++ program, "
                f"please install a C++ compiler with C++{min_cpp_standard} "
                f"support to install JuPedSim."
            )

        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        # Using this requires trailing slash for auto-detection & inclusion of
        # auxiliary "native" libs
        debug = (
            int(os.environ.get("DEBUG", 0))
            if self.debug is None
            else self.debug
        )
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
        # from Python.
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
            "-DCMAKE_UNITY_BUILD=ON",
            f"-DPython_EXECUTABLE={sys.executable}",
        ]

        # Pile all .so in one place and use $ORIGIN as RPATH
        cmake_args += ["-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE"]
        cmake_args += ["-DCMAKE_INSTALL_RPATH={}".format("$ORIGIN")]

        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [
                item for item in os.environ["CMAKE_ARGS"].split(" ") if item
            ]

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator or cmake_generator == "Ninja":
                try:
                    import ninja

                    ninja_executable_path = Path(ninja.BIN_DIR) / "ninja"
                    cmake_args += [
                        "-GNinja",
                        f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja_executable_path}",
                    ]
                except ImportError:
                    pass

        else:
            # Single config generators are handled "normally"
            single_config = any(
                x in cmake_generator for x in {"NMake", "Ninja"}
            )

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += [
                    "-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))
                ]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        build_args += ["-j"]

        subprocess.run(
            ["cmake", ext.sourcedir, *cmake_args], cwd=build_temp, check=True
        )
        subprocess.run(
            ["cmake", "--build", ".", *build_args], cwd=build_temp, check=True
        )

        # Copy library files to root build folder
        files = glob.glob(str(build_temp) + "/lib/py_jupedsim*.so")
        files.extend(glob.glob(str(build_temp) + "/lib/py_jupedsim*.dylib"))
        files.extend(glob.glob(str(build_temp) + "/lib/py_jupedsim*.pyd"))

        for lib_file in files:
            shutil.copy(dst=extdir / "jupedsim", src=lib_file)


# The information here can also be placed in setup.cfg - better separation of
# logic and declaration, and simpler if you include description/version in a file.
setup(
    name="jupedsim",
    version=version,
    maintainer="JuPedSim Development Core Team",
    maintainer_email="dev@jupedsim.org",
    description="JuPedSim is an open source pedestrian dynamics simulator",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license_files=("LICENSE",),
    ext_modules=[CMakeExtension("python_bindings_jupedsim")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.10,<3.13",
    packages=[
        "jupedsim",
        "jupedsim.internal",
        "jupedsim.native",
        "jupedsim_visualizer",
    ],
    package_dir={
        "jupedsim": "python_modules/jupedsim/jupedsim",
        "jupedsim.internal": "python_modules/jupedsim/jupedsim/internal",
        "jupedsim.native": "python_modules/jupedsim/jupedsim/native",
        "jupedsim_visualizer": "python_modules/jupedsim_visualizer/jupedsim_visualizer",
    },
    install_requires=[
        "numpy~=1.25",
        "shapely~=2.0",
        "pyside6~=6.5",
        "vtk~=9.3",
    ],
    scripts=["python_modules/jupedsim_visualizer/bin/jupedsim-visualizer"],
    url="https://www.jupedsim.org",
    project_urls={
        "Documentation": "https://www.jupedsim.org",
        "Source": "https://github.com/PedestrianDynamics/jupedsim",
        "Tracker": "https://github.com/PedestrianDynamics/jupedsim/issues",
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: Unix",
        "Operating System :: MacOS",
        "Natural Language :: English",
        "Programming Language :: C++",
        "Programming Language :: C",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
    ],
)
