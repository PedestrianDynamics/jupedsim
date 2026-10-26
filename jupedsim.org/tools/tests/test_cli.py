# SPDX-License-Identifier: LGPL-3.0-or-later
"""The CLI frame: version, usage, exit codes, error output, discovery."""

import argparse
import os
import pkgutil
import subprocess
import sys
import tomllib
from collections.abc import Callable
from pathlib import Path

import jps_website_tooling
import pytest
from jps_website_tooling import __version__, cli
from jps_website_tooling.cli import ToolError, UsageError, main
from jps_website_tooling.subcommand import SubcommandBuilder, Subparsers

TOOLS_DIR = Path(__file__).resolve().parents[1]


def _probe(
    handler: Callable[[argparse.Namespace], int],
) -> list[SubcommandBuilder]:
    """Build a single ``probe`` subcommand handled by ``handler``.

    Args:
        handler: The subcommand handler ``(args) -> exit code``.

    Returns:
        A builders list for ``main(..., builders=...)``.
    """

    def builder(subparsers: Subparsers) -> None:
        """Add the ``probe`` subparser."""
        probe = subparsers.add_parser("probe")
        probe.set_defaults(func=handler)

    return [builder]


def _raiser(exc: Exception) -> Callable[[argparse.Namespace], int]:
    """Build a handler that raises ``exc``.

    Args:
        exc: The exception instance to raise.

    Returns:
        A handler usable with ``_probe``.
    """

    def handler(args: argparse.Namespace) -> int:
        """Raise the captured exception."""
        raise exc

    return handler


def test_version_flag(capsys: pytest.CaptureFixture[str]) -> None:
    assert main(["--version"]) == 0
    assert (
        capsys.readouterr().out.strip() == f"jps-website-tooling {__version__}"
    )


def test_no_command_is_usage_error(capsys: pytest.CaptureFixture[str]) -> None:
    assert main([]) == 2
    assert "usage: jps-website-tooling" in capsys.readouterr().err


def test_nested_group_without_inner_command_is_usage_error(
    capsys: pytest.CaptureFixture[str],
) -> None:
    def builder(subparsers: Subparsers) -> None:
        """Add a ``preview`` group with one inner ``rebase`` command."""
        group = subparsers.add_parser("preview")
        inner = group.add_subparsers(dest="preview_command")
        inner.add_parser("rebase").set_defaults(func=lambda args: 0)

    assert main(["preview", "rebase"], builders=[builder]) == 0
    assert main(["preview"], builders=[builder]) == 2
    assert "usage:" in capsys.readouterr().err


def test_errors_module_is_the_home_of_the_exceptions() -> None:
    from jps_website_tooling import errors

    assert cli.ToolError is errors.ToolError
    assert cli.UsageError is errors.UsageError
    assert issubclass(errors.UsageError, errors.ToolError)


def test_discovery_finds_every_cmd_module() -> None:
    cmd_modules = sorted(
        m.name
        for m in pkgutil.iter_modules(jps_website_tooling.__path__)
        if m.name.startswith(cli.CMD_PREFIX)
    )
    assert "cmd_versions" in cmd_modules
    builders = cli.discover_subcommands()
    assert [b.__module__.rsplit(".", 1)[1] for b in builders] == cmd_modules
    # The discovered subcommands are what the real parser offers.
    parser = cli.build_parser()
    choices = next(a.choices for a in parser._actions if a.dest == "command")
    assert set(choices) == {
        m.removeprefix(cli.CMD_PREFIX).replace("_", "-") for m in cmd_modules
    }


def test_every_subcommand_help_names_its_extra_packages(
    capsys: pytest.CaptureFixture[str],
) -> None:
    # Each cmd_* module declares REQUIREMENTS (distribution specs beyond the
    # stdlib) and its --help shows them, so a user of an uninstalled checkout
    # knows what to pip install. Every spec must be a package dependency.
    import importlib
    import tomllib

    deps = tomllib.loads((TOOLS_DIR / "pyproject.toml").read_text())["project"][
        "dependencies"
    ]
    for module_name in sorted(
        m.name
        for m in pkgutil.iter_modules(jps_website_tooling.__path__)
        if m.name.startswith(cli.CMD_PREFIX)
    ):
        module = importlib.import_module(f"jps_website_tooling.{module_name}")
        requirements = module.REQUIREMENTS
        assert isinstance(requirements, tuple), module_name
        name = module_name.removeprefix(cli.CMD_PREFIX).replace("_", "-")
        assert main([name, "--help"]) == 0
        text = capsys.readouterr().out
        if requirements:
            assert "Additional packages:" in text, module_name
            for spec in requirements:
                if spec.startswith("-r "):
                    # A requirements file of the checkout, not a package dep.
                    assert spec.removeprefix("-r ") in text, (module_name, spec)
                    continue
                assert spec in text, (module_name, spec)
                assert spec in deps, (module_name, spec)
        else:
            assert "Additional packages: none" in text, module_name


def test_requirements_epilog_with_requirement_files() -> None:
    from jps_website_tooling.subcommand import requirements_epilog

    assert requirements_epilog(["-r a/req.txt", "-r b/req.txt"]) == (
        "Additional packages: see a/req.txt, b/req.txt "
        "(python -m pip install -r a/req.txt -r b/req.txt)."
    )


def test_handler_return_code_is_exit_code() -> None:
    assert main(["probe"], builders=_probe(lambda args: 0)) == 0
    assert main(["probe"], builders=_probe(lambda args: 3)) == 3


def test_tool_error_is_one_annotation(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    builders = _probe(_raiser(ToolError("line one\nline two 100%")))
    assert main(["probe"], builders=builders) == 1
    captured = capsys.readouterr().out
    assert captured.count("::error::") == 1
    assert "::error::line one%0Aline two 100%25\n" in captured


def test_tool_error_is_readable_outside_actions(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    builders = _probe(_raiser(ToolError("line one\nline two")))
    assert main(["probe"], builders=builders) == 1
    assert capsys.readouterr().out == "error: line one\nline two\n"


def test_usage_error_exit_2(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    builders = _probe(_raiser(UsageError("bad flag")))
    assert main(["probe"], builders=builders) == 2
    assert capsys.readouterr().out == "error: bad flag\n"


def test_python_m_without_install(tmp_path: Path) -> None:
    env = dict(os.environ, PYTHONPATH=str(TOOLS_DIR))
    result = subprocess.run(
        [sys.executable, "-m", "jps_website_tooling", "--version"],
        cwd=tmp_path,
        env=env,
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, result.stderr
    assert result.stdout.strip() == f"jps-website-tooling {__version__}"


def test_pyproject_declares_console_script() -> None:
    data = tomllib.loads((TOOLS_DIR / "pyproject.toml").read_text())
    project = data["project"]
    assert project["name"] == "jps-website-tooling"
    assert project["requires-python"] == ">=3.14"
    assert (
        project["scripts"]["jps-website-tooling"]
        == "jps_website_tooling.cli:main"
    )
    names = {d.split("[")[0].split("~")[0] for d in project["dependencies"]}
    assert names == {"packaging", "pagefind", "PyGithub"}
