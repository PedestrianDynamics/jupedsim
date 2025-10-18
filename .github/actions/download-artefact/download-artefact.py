#!/usr/bin/env python
"""
Download artifacts from GitHub Actions workflow runs.
"""

import argparse
import json
import os
import sys
import zipfile
from pathlib import Path
from typing import Optional, Dict, Any, List

import requests


def print_step(message: str) -> None:
    """Print a step message in GitHub Actions format."""
    print(f"::group::{message}")
    print(message)
    print("::endgroup::")


def print_info(message: str) -> None:
    """Print an info message."""
    print(f"INFO: {message}")


def print_success(message: str) -> None:
    """Print a success message."""
    print(f"SUCCESS: {message}")


def print_error(message: str) -> None:
    """Print an error message in GitHub Actions format."""
    print(f"::error::{message}")
    print(f"ERROR: {message}", file=sys.stderr)


def set_output(name: str, value: str) -> None:
    """Set a GitHub Actions output variable."""
    github_output = os.environ.get("GITHUB_OUTPUT")
    if github_output:
        with open(github_output, "a") as f:
            f.write(f"{name}={value}\n")
    print(f"Output: {name}={value}")


def make_auth_headers(token: str) -> Dict[str, str]:
    """
    Get authentication headers for GitHub API requests.

    Args:
        token: GitHub authentication token

    Returns:
        Dictionary of headers
    """
    return {
        "Authorization": f"Bearer {token}",
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
    }


def list_workflow_run_artifacts(
    repository: str, run_id: str, token: str
) -> List[Dict[str, Any]]:
    """
    List all artifacts for a workflow run.

    Args:
        repository: GitHub repository in format 'owner/repo'
        run_id: Workflow run ID
        token: GitHub authentication token

    Returns:
        List of artifact dictionaries

    Raises:
        SystemExit: If API call fails
    """
    url = f"https://api.github.com/repos/{repository}/actions/runs/{run_id}/artifacts"
    headers = make_auth_headers(token)

    try:
        response = requests.get(url, headers=headers, timeout=30)
        response.raise_for_status()
        data = response.json()
        return data.get("artifacts", [])

    except requests.exceptions.HTTPError as e:
        print_error(f"HTTP error fetching artifacts: {e}")
        if e.response is not None:
            print_error(f"Response: {e.response.text}")
        sys.exit(1)
    except requests.exceptions.RequestException as e:
        print_error(f"Request error fetching artifacts: {e}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print_error(f"Failed to parse API response: {e}")
        sys.exit(1)


def download_artifact_zip(
    repository: str, artifact_id: str, token: str
) -> bytes:
    """
    Download an artifact as a zip file.

    Args:
        repository: GitHub repository in format 'owner/repo'
        artifact_id: Artifact ID to download
        token: GitHub authentication token

    Returns:
        Artifact zip file content as bytes

    Raises:
        SystemExit: If API call fails
    """
    url = f"https://api.github.com/repos/{repository}/actions/artifacts/{artifact_id}/zip"
    headers = make_auth_headers(token)

    try:
        response = requests.get(url, headers=headers, timeout=120, stream=True)
        response.raise_for_status()
        return response.content

    except requests.exceptions.HTTPError as e:
        print_error(f"HTTP error downloading artifact: {e}")
        if e.response is not None:
            print_error(f"Response: {e.response.text}")
        sys.exit(1)
    except requests.exceptions.RequestException as e:
        print_error(f"Request error downloading artifact: {e}")
        sys.exit(1)


def lookup(
    repository: str,
    run_id: str,
    artifact_name: str,
    token: str,
) -> str:
    """
    Look up an artifact ID by name within a workflow run.

    Args:
        repository: GitHub repository in format 'owner/repo'
        run_id: Workflow run ID
        artifact_name: Name of the artifact to find
        token: GitHub authentication token

    Returns:
        Artifact ID as a string

    Raises:
        SystemExit: If artifact is not found or API call fails
    """
    print_step(f"Looking up artifact '{artifact_name}' in run {run_id}")

    # Fetch all artifacts for the run
    artifacts = list_workflow_run_artifacts(repository, run_id, token)
    print_info(f"Found {len(artifacts)} total artifact(s) in run")

    # Filter artifacts by name
    matching_artifacts = [
        a for a in artifacts if a.get("name") == artifact_name
    ]

    if not matching_artifacts:
        print_error(
            f"No artifact named '{artifact_name}' found in run {run_id}"
        )
        sys.exit(1)

    print_info(
        f"Found {len(matching_artifacts)} artifact(s) named '{artifact_name}'"
    )

    # Sort by created_at to get the latest
    matching_artifacts.sort(key=lambda a: a.get("created_at", ""), reverse=True)

    selected_artifact = matching_artifacts[0]
    artifact_id = str(selected_artifact["id"])
    created_at = selected_artifact.get("created_at", "unknown")
    attempt = selected_artifact.get("workflow_run", {}).get(
        "run_attempt", "unknown"
    )

    print_success(f"Selected artifact ID: {artifact_id}")
    print_info(f"  Created at: {created_at}")
    print_info(f"  Run attempt: {attempt}")

    return artifact_id


def download(
    repository: str, artifact_id: str, download_path: str, token: str
) -> None:
    """
    Download and extract an artifact by ID.

    Args:
        repository: GitHub repository in format 'owner/repo'
        artifact_id: Artifact ID to download
        download_path: Path to extract the artifact contents
        token: GitHub authentication token

    Raises:
        SystemExit: If download or extraction fails
    """
    print_step(f"Downloading artifact {artifact_id}")

    # Create download directory
    download_dir = Path(download_path)
    download_dir.mkdir(parents=True, exist_ok=True)
    print_info(f"Download path: {download_dir.absolute()}")

    # Download artifact as zip
    zip_path = download_dir.parent / f"artifact-{artifact_id}.zip"

    print_info("Fetching artifact from GitHub API...")
    zip_content = download_artifact_zip(repository, artifact_id, token)

    # Write zip file
    with open(zip_path, "wb") as f:
        f.write(zip_content)

    file_size = zip_path.stat().st_size
    print_success(f"Downloaded {file_size:,} bytes")

    # Extract the zip file
    try:
        print_info("Extracting artifact...")
        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            zip_ref.extractall(download_dir)

        # Clean up zip file
        zip_path.unlink()

        # List extracted files
        files = list(download_dir.rglob("*"))
        file_count = len([f for f in files if f.is_file()])
        print_success(f"Extracted {file_count} file(s)")

        # Show directory contents
        print("\nDownloaded contents:")
        for item in sorted(download_dir.iterdir()):
            if item.is_file():
                size = item.stat().st_size
                print(f"   {item.name} ({size:,} bytes)")
            else:
                print(f"   {item.name}/")

    except zipfile.BadZipFile as e:
        print_error(f"Failed to extract artifact: {e}")
        sys.exit(1)
    except Exception as e:
        print_error(f"Unexpected error during extraction: {e}")
        sys.exit(1)


def main() -> None:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Download artifacts from GitHub Actions workflow runs"
    )
    parser.add_argument("--run-id", required=True, help="Workflow run ID")
    parser.add_argument(
        "--artifact-name",
        required=True,
        help="Name of the artifact to download",
    )
    parser.add_argument(
        "--download-path",
        required=True,
        help="Path to extract the artifact contents",
    )
    parser.add_argument(
        "--repository",
        required=True,
        help="GitHub repository in format owner/repo",
    )
    args = parser.parse_args()

    # Check for GH_TOKEN
    token = os.environ.get("GH_TOKEN")
    if not token:
        print_error("GH_TOKEN environment variable is not set")
        sys.exit(1)

    print("=" * 60)
    print("GitHub Artifact Downloader")
    print("=" * 60)

    # Lookup artifact
    artifact_id = lookup(
        repository=args.repository,
        run_id=args.run_id,
        artifact_name=args.artifact_name,
        token=token,
    )

    # Set output for GitHub Actions
    set_output("artifact-id", artifact_id)

    # Download artifact
    download(
        repository=args.repository,
        artifact_id=artifact_id,
        download_path=args.download_path,
        token=token,
    )

    # Set output for GitHub Actions
    set_output("download-path", args.download_path)

    print("\n" + "=" * 60)
    print_success("Artifact download completed successfully!")
    print("=" * 60)


if __name__ == "__main__":
    main()
