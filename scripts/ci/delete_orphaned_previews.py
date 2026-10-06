#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Find PR preview directories whose pull requests are no longer open."""

import argparse
import os
import shutil
from pathlib import Path

from github import Auth, Github
from github.GithubException import GithubException


def find_orphaned_previews(root: Path, repo) -> list[Path]:
    if not root.exists():
        return []

    orphaned = []

    for preview in root.iterdir():
        if not preview.is_dir():
            continue

        try:
            pr_number = int(preview.name)
        except ValueError:
            continue

        try:
            pr = repo.get_pull(pr_number)
        except GithubException as error:
            if error.status == 404:
                print(
                    f"::notice::Pull request #{pr_number} not found; keeping preview"
                )
                continue
            raise

        if pr.state != "open":
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
        "--repo",
        required=True,
        help="GitHub repository in owner/name format",
    )
    args = parser.parse_args()

    github_token = os.environ.get("GITHUB_TOKEN")
    if not github_token:
        raise RuntimeError("GITHUB_TOKEN environment variable is not set")

    auth = Auth.Token(github_token)
    github = Github(auth=auth)
    repo = github.get_repo(args.repo)

    orphaned = find_orphaned_previews(args.root, repo)
    delete_orphaned_previews(orphaned)


if __name__ == "__main__":
    main()
