#!/usr/bin/env python

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import datetime
import re
from pathlib import Path


def collect_files(
    file_name_patterns: list[str],
    exclude_directories: list[str],
    start_dir: Path,
):
    files = []
    for pattern in file_name_patterns:
        for p in start_dir.rglob(f"**/{pattern}"):
            if exclude_directories is None or not any(
                [str(p).startswith(dir) for dir in exclude_directories]
            ):
                files.append(p)

    return files


def update_year_to(file: Path, year: int, pattern: re.Pattern):
    content = file.read_text().splitlines()
    found = False
    for line, idx in zip(content, range(5)):
        found = pattern.search(line) is not None
        if found:
            content[idx] = pattern.sub(
                f"Copyright © 2012-{year} Forschungszentrum Jülich GmbH", line
            )
            break
    file.write_text("\n".join(content) + "\n")
    return found


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-e",
        "--exclude",
        type=str,
        action="append",
        help="directories (path prefixes) to exclude",
    )
    parser.add_argument(
        "-y",
        "--year",
        type=int,
        help="New end year for copyright header",
        default=datetime.date.today().year,
    )
    parser.add_argument("root", type=Path, default=Path.cwd())
    return parser.parse_args()


def main():
    args = parse_arguments()
    patterns = ["*.py", "*.hpp", "*.cpp", "*.h", "*.c"]
    copyright_pattern = re.compile(
        "Copyright © 2012-\\d{4} Forschungszentrum Jülich GmbH"
    )
    files = collect_files(patterns, args.exclude, args.root)
    for file in files:
        if not update_year_to(file, args.year, copyright_pattern):
            print(f"{file} does not have a copyright header!")


if __name__ == "__main__":
    main()
