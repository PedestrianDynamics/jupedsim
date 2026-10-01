#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Find PR preview directories whose pull requests are no longer open."""

import argparse
import shutil
from pathlib import Path

from github import Auth, Github


def list_open_pr_numbers(github: Github, repository: str) -> set[int]:
    repo = github.get_repo(repository)
    return {pr.number for pr in repo.get_pulls(state="open")}


def list_stable_open_pr_numbers(github: Github, repository: str) -> set[int]:
    prs = list_open_pr_numbers(github, repository)

    while True:
        again = list_open_pr_numbers(github, repository)
        if again == prs:
            return prs
        prs = again


def find_orphaned_previews(root: Path, open_prs: set[int]) -> list[Path]:
    orphaned = []

    for preview in root.iterdir():
        if not preview.is_dir():
            continue

        try:
            pr_number = int(preview.name)
        except ValueError:
            continue

        if pr_number not in open_prs:
            orphaned.append(preview)

    return orphaned


def delete_orphaned_previews(orphaned: list[Path]) -> None:
    for preview in orphaned:
        print(f"Deleting orphaned preview: {preview}")
        shutil.rmtree(preview)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        type=Path,
        help="Path to the pull-requests preview directory",
    )
    parser.add_argument(
        "--github-token",
        required=True,
        help="GitHub token used to query pull requests",
    )
    args = parser.parse_args()

    auth = Auth.Token(args.github_token)
    github = Github(auth=auth)

    open_prs = list_stable_open_pr_numbers(
        github, "PedestrianDynamics/jupedsim"
    )
    print(f"Open PRs: {sorted(open_prs)}")

    orphaned = find_orphaned_previews(args.root, open_prs)
    delete_orphaned_previews(orphaned)


if __name__ == "__main__":
    main()
