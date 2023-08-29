# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import io
import logging
import os
import pathlib
import subprocess
import pandas as pd

import shapely
import sqlite3
import matplotlib.pyplot as plt

import jinja2

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)


def parse_args():
    parser = argparse.ArgumentParser("JuPedSim Performance Tests")
    parser.add_argument(
        "-t",
        "--test",
        choices=["grosser_stern", "large_street_network", "all"],
        default="all",
    )

    known, forwarded_args = parser.parse_known_args()
    if forwarded_args and forwarded_args[0] != "--":
        logging.warning(
            f"found unknown arguments: '{' '.join(forwarded_args)}' will be ignored. If you want to pass "
            f"them to the tests, separate them with '--', e.g., \n"
            f"$ python run_perf_test.py -- -t large_street_network -- --limit 10000"
        )
    else:
        forwarded_args = forwarded_args[1:]

    return known, forwarded_args


def build_jupedsim():
    # Create build directory
    build_path = pathlib.Path("/build")
    build_path.mkdir(exist_ok=True)

    with subprocess.Popen(
            [
                "cmake",
                "-S",
                "/src",
                "-B",
                "/build",
                "-DCMAKE_PREFIX_PATH=/opt/deps",
                "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
    ) as proc:
        if proc.stdout:
            for line in proc.stdout:
                print(line.decode("utf-8").rstrip())

    with subprocess.Popen(
            ["cmake", "--build", "/build", "--", "-j", "--", "VERBOSE=1"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
    ) as proc:
        if proc.stdout:
            for line in proc.stdout:
                print(line.decode("utf-8").rstrip())


def run_test(test, args, build_dir, result_dir):
    test_env = os.environ.copy()
    test_env[
        "PYTHONPATH"
    ] = f"/src/python_modules/jupedsim:/src/python_modules/jupedsim_visualizer:/build/lib"

    perf_data_file_name = f"{test}.perf.data"
    perf_file_name = f"{test}.perf"
    perf_folded_file_name = f"{test}.folded"
    perf_svg_file_name = f"{test}.svg"
    perf_geo_svg_file_name = f"{test}_geo.svg"
    perf_it_time_svg_file_name = f"{test}_it_time.svg"
    perf_op_lvl_svg_file_name = f"{test}_op_lvl.svg"
    perf_tt_svg_file_name = f"{test}_tt.svg"

    with subprocess.Popen(
            [
                "perf",
                "record",
                "--call-graph",
                "fp",
                "-e",
                "cycles:u",
                "-o",
                perf_data_file_name,
                "python3",
                f"/src/performancetest/{test}.py",
                *args,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            env=test_env,
            cwd="/build",
    ) as proc:
        if proc.stdout:
            for line in proc.stdout:
                print(line.decode("utf-8").rstrip())

    with open(build_dir / perf_file_name, "w") as perf_file:
        subprocess.run(
            ["perf", "script", "-i", perf_data_file_name],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )

    with open(build_dir / perf_folded_file_name, "w") as perf_file:
        subprocess.run(
            [
                "/opt/FlameGraph/stackcollapse-perf.pl",
                perf_file_name,
            ],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )

    with open(result_dir / perf_svg_file_name, "w") as perf_file:
        subprocess.run(
            [
                "/opt/FlameGraph/flamegraph.pl",
                "--title",
                test,
                "--inverted",
                "--colors",
                "mem",
                perf_folded_file_name,
            ],
            stdout=perf_file,
            cwd="/build",
            check=True,
        )
    sql_files = []
    path = os.getcwd()
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".sqlite") and test in file:
                sql_files.append(os.path.join(root, file))

    db = sqlite3.connect(sql_files[0])

    perf_stats = pd.read_sql_query("SELECT * FROM perf_statistics", db)
    metadata = pd.read_sql_query("SELECT * FROM metadata", db)
    geometry_as_wkt = (
        db.cursor().execute("SELECT * from geometry LIMIT 1").fetchone()[0]
    )
    geometry = shapely.from_wkt(geometry_as_wkt)

    plt.figure()
    plt.axis('equal')
    for geo in geometry.geoms:
        xe, ye = geo.exterior.xy

        for inner in geo.interiors:
            xi, yi = zip(*inner.coords[:])
            plt.plot(xi, yi, color="blue")

        plt.plot(xe, ye, color="blue")
    plt.savefig(result_dir / perf_geo_svg_file_name)
    plt.figure()

    # Total iteration time / agent count per iteration
    perf_stats["iteration_loop_us"].plot(
        figsize=(12.5, 6.25), xlabel="iteration", ylabel="time[µs]", legend=True
    )
    perf_stats["agent_count"].plot(secondary_y=True, ylabel="agents", legend=True)
    plt.savefig(result_dir / perf_it_time_svg_file_name)
    plt.figure()

    # Time to compute oerational level update / agent count per iteration
    perf_stats["operational_level_us"].plot(
        figsize=(12.5, 6.25), xlabel="iteration", ylabel="time[µs]", legend=True
    )
    perf_stats["agent_count"].plot(secondary_y=True, ylabel="agents", legend=True)
    plt.savefig(result_dir / perf_op_lvl_svg_file_name)
    plt.figure()

    # Total time w.o. operational level / agent count per iteration
    perf_stats["delta"] = (
            perf_stats["iteration_loop_us"] - perf_stats["operational_level_us"]
    )
    perf_stats["delta"].plot(
        figsize=(12.5, 6.25), xlabel="iteration", ylabel="time[µs]", legend=True
    )
    perf_stats["agent_count"].plot(secondary_y=True, ylabel="agents", legend=True)
    plt.savefig(result_dir / perf_tt_svg_file_name)
    plt.figure()

    logging.info(f"created flamegraph for {test}")


def build_report(results_dir: pathlib.Path, results: dict[str, str], plots: dict[str, str]) -> None:
    template = """
<!DOCTYPE html>
<html lang="en">
<style>
.title {
    background-color: #777;
    color: white;
    padding: 6px;
    text-align: center;
    font-size: 15px;
    margin: 0 0 0 0;
}
.collapsible {
    background-color: #777;
    color: white;
    cursor: pointer;
    padding: 6px;
    width: 100%;
    border: none;
    text-align: center;
    outline: none;
    font-size: 12px;
}

.collapsible:after {
    content: '++++';
    color: white;
    font-weight: bold;
    float: right;
}

.collapsible:before {
    content: '++++';
    color: white;
    font-weight: bold;
    float: left;
}

.active:after {
    content: "----";
}

.active:before {
    content: "----";
}

.content {
    max-height: 0;
    overflow: hidden;
    text-align: center;
}
.plot {
    overflow: hidden;
    text-align: center;
}
.plot .title {
    font-size: 12px;
}
</style>
<head><title>Performance Test Results</title></head>
<body>
    <h1 class="title">Performance Test Results</h1>
    {% for title, content in results.items() %}
        <button class="collapsible">{{ title }}</button>
        <div class="content">
            <object type="image/svg+xml" data="./{{ content }}"></object>
        </div>
    {% endfor %}
    {% for title, content in plots.items() %}
        <div class=plot>
            <div class="title"> {{ title }} </div>
            <object type="image/svg+xml" data="./{{ content }}"></object>
        </div>
    {% endfor %}
    
<script defer>
var coll = document.getElementsByClassName("collapsible");
var i;

for (i = 0; i < coll.length; i++) {
  coll[i].addEventListener("click", function() {
    this.classList.toggle("active");
    var content = this.nextElementSibling;
    if (content.style.maxHeight){
      content.style.maxHeight = null;
    } else {
      content.style.maxHeight = content.scrollHeight + "px";
    } 
  });
}
</script>
</body>
</html>
    """
    tpl = jinja2.Environment().from_string(template)
    (results_dir / "report.html").write_text(tpl.render(results=results, plots=plots))


def run_tests(test_selection: str, args):
    build_dir = pathlib.Path("/build")
    result_dir = build_dir / "results"
    result_dir.mkdir()

    results, plots = {}, {}
    if test_selection in ["all", "large_street_network"]:
        logging.info("run large_street_network performance test")
        if not args or test_selection == "all":
            args = ["--limit", "4000"]
        run_test("large_street_network", args, build_dir, result_dir)
        results["Large Street Network"] = "large_street_network.svg"
        plots["Large Street Network Geometry"] = "large_street_network_geo.svg"
        plots["Large Street Network Total iteration time"] = "large_street_network_it_time.svg"
        plots["Large Street Network Time to compute operational level"] = "large_street_network_op_lvl.svg"
        plots["Large Street Network Total time w.o. operational level"] = "large_street_network_tt.svg"

    if test_selection in ["all", "grosser_stern"]:
        logging.info("run grosser_stern performance test")
        if not args or test_selection == "all":
            args = ["--limit", "100"]
        run_test("grosser_stern", args, build_dir, result_dir)
        # adds plot of geo instead of flamegraph
        results["Grosser Stern"] = "grosser_stern.svg"
        plots["Grosser Stern Geometry"] = "grosser_stern_geo.svg"
        plots["Grosser Stern Total iteration time"] = "grosser_stern_it_time.svg"
        plots["Grosser Stern Time to compute operational level"] = "grosser_stern_op_lvl.svg"
        plots["Grosser Stern Total time w.o. operational level"] = "grosser_stern_tt.svg"

    build_report(result_dir, results, plots)


def main():
    known, forwarded_args = parse_args()

    build_jupedsim()
    run_tests(known.test, forwarded_args)


if __name__ == "__main__":
    main()
