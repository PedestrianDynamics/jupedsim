import argparse

import logging
import os
import pathlib
import subprocess
import sys

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)

def parse_args():
    parser = argparse.ArgumentParser('JuPedSim Performance Tests')
    parser.add_argument("-t", "--test", choices=["grosser_stern", "large_street_network", "all"], default="all")

    known, forwarded_args = parser.parse_known_args()
    if forwarded_args and forwarded_args[0] != "--":
        # TODO warning when remainder does not start with --
        pass
    else:
        forwarded_args = forwarded_args[1:]
    print(forwarded_args)

    return known, forwarded_args

def build_jupedsim():
    # Create build directory
    build_path = pathlib.Path("/build")
    build_path.mkdir()

    with subprocess.Popen(
            ["cmake", "-S", "/src", "-B", "/build", "-DCMAKE_PREFIX_PATH=/opt/deps"],
            # capture_output=True,
            # check=True
            stdout=subprocess.PIPE,
        ) as p:
        for line in p.stdout:
            print(line.decode("utf-8").rstrip())
        # pass
    with subprocess.Popen(
            ["cmake", "--build", "/build" ,"--", "-j", "--", "VERBOSE=1"],
            # capture_output=True,
            # check=True
            stdout=subprocess.PIPE,
        ) as p:
        for line in p.stdout:
            print(line.decode("utf-8").rstrip())
        # pass

def run_test(test, args, result_dir):
    """
    perf record --call-graph fp -e cycles:u /src/performancetest/grosser_stern.py --limit 10
perf script > grosser_stern.perf
/opt/FlameGraph/stackcollapse-perf.pl grosser_stern.perf > grosser_stern.folded
/opt/FlameGraph/flamegraph.pl --width 2000 grosser_stern.folded > grosser_stern.svg
    :return:
    """
    print(f"args: {args}")
    test_env = os.environ.copy()
    test_env["PYTHONPATH"] = f"/src/python_modules/jupedsim:/src/python_modules/visdbg:/build/lib"

    perf = subprocess.Popen(
            ["perf", "record", "--call-graph", "fp", "-e", "cycles:u", "-o", "perf.data", "python3", f"/src/performancetest/{test}.py", *args],
            # "perf record --call-graph fp -e cycles:u -o perf.data python3 /src/performancetest/large_street_network.py --limit 100",
            # shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=test_env,
            cwd="/build"
        )
    out, err = perf.communicate()
    print()
    print("recorded")

    with open(f"/build/{test}.perf", "w") as perf_file:
        print(perf_file.name)
        with subprocess.Popen(
                ["perf", "script"],
                stdout=perf_file,
                env=test_env,
                cwd="/build",
        ) as p:
            pass

    with open(f"/build/{test}.folded", "w") as perf_file:
        print(perf_file.name)
        with subprocess.Popen(
                ["/opt/FlameGraph/stackcollapse-perf.pl", f"{test}.perf"],
                stdout=perf_file,
                cwd="/build",
        ) as p:
            pass

    with open(result_dir/f"{test}.svg", "w") as perf_file:
        print(perf_file.name)
        with subprocess.Popen(
                ["/opt/FlameGraph/flamegraph.pl", f"{test}.folded"],
                stdout=perf_file,
                cwd="/build",
        ) as p:
            pass

    with subprocess.Popen(
            ["ls", "-lah", f"{str(result_dir.absolute())}"],
            stdout=subprocess.PIPE,
            env=test_env,
        ) as p:
        for line in p.stdout:
            print(line.decode("utf-8").rstrip())


def run_tests(test_selection: str, args):
    result_dir = pathlib.Path("/build/results")
    result_dir.mkdir()

    if test_selection in ["all", "large_street_network"]:
        print("large_street_network")
        if not args or test_selection == "all":
            args = ["--limit", "100"] #4000
        print(args)
        run_test("large_street_network", args, result_dir)

    if test_selection in ["all", "grosser_stern"]:
        print("grosser_stern")
        if not args or test_selection == "all":
            args = ["--limit", "10"] #100
        print(args)
        run_test("grosser_stern", args, result_dir)


def main():
    known, forwarded_args = parse_args()
    print(known)
    print(forwarded_args)

    build_jupedsim()
    run_tests(known.test, forwarded_args)

if __name__ == "__main__":
    main()




