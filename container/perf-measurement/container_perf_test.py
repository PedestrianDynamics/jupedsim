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
        logging.warning(f"found unknown arguments: '{' '.join(forwarded_args)}' will be ignored. If you want to pass "
                        f"them to the tests, separate them with '--', e.g., \n"
                        f"$ python run_perf_test.py -- -t large_street_network -- --limit 10000")
    else:
        forwarded_args = forwarded_args[1:]

    return known, forwarded_args


def build_jupedsim():
    # Create build directory
    build_path = pathlib.Path("/build")
    build_path.mkdir()

    with subprocess.Popen(
            ["cmake", "-S", "/src", "-B", "/build", "-DCMAKE_PREFIX_PATH=/opt/deps", "-DCMAKE_BUILD_TYPE=RelWithDebInfo"],
            stdout=subprocess.PIPE,
    ) as p:
        for line in p.stdout:
            print(line.decode("utf-8").rstrip())

    with subprocess.Popen(
            ["cmake", "--build", "/build", "--", "-j", "--", "VERBOSE=1"],
            stdout=subprocess.PIPE,
    ) as p:
        for line in p.stdout:
            print(line.decode("utf-8").rstrip())


def run_test(test, args, build_dir, result_dir):
    test_env = os.environ.copy()
    test_env["PYTHONPATH"] = f"/src/python_modules/jupedsim:/src/python_modules/visdbg:/build/lib"

    perf_data_file_name = f"{test}.perf.data"
    perf_file_name = f"{test}.perf"
    perf_folded_file_name = f"{test}.folded"
    perf_svg_file_name = f"{test}.svg"

    subprocess.run(
        ["perf", "record", "--call-graph", "fp", "-e", "cycles:u", "-o", perf_data_file_name, "python3",
         f"/src/performancetest/{test}.py", *args],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=test_env,
        cwd="/build",
        check=True,
    )

    with open(build_dir / perf_file_name, "w") as perf_file:
        subprocess.run(
            ["perf", "script", "-i", perf_data_file_name],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )

    with open(build_dir / perf_folded_file_name, "w") as perf_file:
        subprocess.run(
            ["/opt/FlameGraph/stackcollapse-perf.pl", perf_file_name],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )

    with open(result_dir / perf_svg_file_name, "w") as perf_file:
        subprocess.run(
            ["/opt/FlameGraph/flamegraph.pl", perf_folded_file_name],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )

    logging.info(f"created flamegraph for {test} in: {str(result_dir / perf_svg_file_name)}")


def run_tests(test_selection: str, args):
    build_dir = pathlib.Path("/build")
    result_dir = build_dir / "results"
    result_dir.mkdir()

    if test_selection in ["all", "large_street_network"]:
        logging.info("run large_street_network performance test")
        if not args or test_selection == "all":
            args = ["--limit", "4000"]
        run_test("large_street_network", args, build_dir, result_dir)

    if test_selection in ["all", "grosser_stern"]:
        logging.info("run grosser_stern performance test")
        if not args or test_selection == "all":
            args = ["--limit", "100"]
        run_test("grosser_stern", args, build_dir, result_dir)


def main():
    known, forwarded_args = parse_args()

    build_jupedsim()
    run_tests(known.test, forwarded_args)


if __name__ == "__main__":
    main()
