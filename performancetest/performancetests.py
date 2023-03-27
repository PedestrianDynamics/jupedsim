#! /usr/bin/env python3
import json
import logging
import sys
import time
from pathlib import Path

from driver.driver import JpsCoreDriver
from driver.environment import Environment
from driver.utils import copy_all_files


def run_scenario(
    *, jpscore_path: Path, scenario_path: Path, working_directory: Path
):
    working_directory.mkdir(parents=True, exist_ok=True)
    copy_all_files(
        src=scenario_path,
        dest=working_directory,
    )

    jpscore_driver = JpsCoreDriver(
        jpscore_path=jpscore_path, working_directory=working_directory
    )
    logging.info(f"Running {scenario_path.name}")

    start_time = time.time()
    jpscore_driver.run()
    end_time = time.time()
    return end_time - start_time


def pre_check_scenarion_input(path: Path):
    """
    Performance tests can run for several hours, hence we check
    if all input files are present
    """
    if not path.exists():
        raise Exception(f"Path not found {path}")

    if not path.is_dir():
        raise Exception(f"Path is not a directory {path}")

    inifile = path / "inifile.xml"
    if not inifile.exists():
        raise Exception(f"Inifile not found {inifile}")

    if not inifile.is_file():
        raise Exception(f"Inifile is not a file {inifile}")


def main():
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    env = Environment()
    scenario_names = [
        "sisame_evac_shortest_ff_router_velocity",
        "sisame_evac_shortest_ff_router_gcfm",
    ]
    scenario_paths = [env.performancetests_path / id for id in scenario_names]
    for p in scenario_paths:
        try:
            pre_check_scenarion_input(p)
        except Exception as e:
            logging.error(f"Error during pre run check: {e}")
            sys.exit(1)

    results = {
        id: run_scenario(
            jpscore_path=env.jpscore_path,
            scenario_path=path,
            working_directory=Path.cwd() / "performancetests_results" / id,
        )
        for id, path in zip(scenario_names, scenario_paths)
    }
    print(json.dumps(results, indent=4))


if __name__ == "__main__":
    main()
