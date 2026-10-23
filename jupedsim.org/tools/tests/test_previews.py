# SPDX-License-Identifier: LGPL-3.0-or-later
"""PR previews: links rebased under a prefix, previews of closed PRs pruned."""

import json
from pathlib import Path

import pytest
from github import GithubException
from jps_site_tooling import previews
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.previews import RebaseResult

PREFIX = "/pull-requests/42/"

INDEX_HTML = """\
<a href="/docs/stable/">D</a> <a href='/articles/'>A</a> <img src="/_static/x.png">
<input data-pagefind="/pagefind/pagefind.js"> <form action="/search.html"></form>
<a href="//cdn.example.org/x.js">cdn</a> <a href="https://www.jupedsim.org/">abs</a>
<a href="notes/index.html">rel</a> <a href="#top">frag</a>
<p>Escaped text href=&quot;/not-an-attribute&quot;</p>
"""
ROUTING_HTML = (
    '<select data-json="/docs/versions.json"></select>'
    '<a href="/docs/v1x/v1.4.2/">v1</a>\n'
)
VERSIONS = [
    {"name": "dev", "version": "dev", "url": "/docs/dev/", "preferred": True},
    {"name": "x", "url": "https://www.jupedsim.org/docs/x/"},
]


def make_preview_tree(tmp_path: Path) -> Path:
    """Create the preview fixture of check.sh plus a symlinked HTML file.

    Args:
        tmp_path: Parent directory.

    Returns:
        The tree root.
    """
    tree = tmp_path / "tree"
    (tree / "docs" / "dev" / "concepts").mkdir(parents=True)
    (tree / "notes").mkdir()
    (tree / "index.html").write_text(INDEX_HTML)
    (tree / "docs" / "dev" / "concepts" / "routing.html").write_text(
        ROUTING_HTML
    )
    (tree / "docs" / "versions.json").write_text(
        json.dumps(VERSIONS, indent=2) + "\n"
    )
    (tree / "alias.html").symlink_to("index.html")
    return tree


def make_previews_root(tmp_path: Path) -> Path:
    """Create a pull-requests root with two previews and two other entries.

    Args:
        tmp_path: Parent directory.

    Returns:
        The root; ``12`` and ``34`` are previews, ``tmp`` and ``README`` are
        not.
    """
    root = tmp_path / "pull-requests"
    for name in ("12", "34", "tmp"):
        (root / name).mkdir(parents=True)
        (root / name / "index.html").write_text(f"<p>{name}</p>\n")
    (root / "README").write_text("previews\n")
    return root


def lookup(states: dict[int, bool | None]) -> previews.PullRequestLookup:
    """Build a lookup from a fixed mapping; unknown numbers are a ToolError.

    Args:
        states: PR number to open/closed/None (unknown).

    Returns:
        The lookup callable.
    """

    def is_open(number: int) -> bool | None:
        """Return the recorded state."""
        if number not in states:
            raise ToolError(f"api error for #{number}")
        return states[number]

    return is_open


def test_validate_prefix() -> None:
    assert previews.validate_prefix(PREFIX) == PREFIX
    for bad in ("/", "pull-requests/42/", "/x"):
        with pytest.raises(UsageError):
            previews.validate_prefix(bad)


def test_rebase_html_rewrites_only_root_absolute_attrs(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    assert previews.rebase_html(tree / "index.html", PREFIX) == 5
    text = (tree / "index.html").read_text()
    for want in (
        'href="/pull-requests/42/docs/stable/"',
        "href='/pull-requests/42/articles/'",
        'src="/pull-requests/42/_static/x.png"',
        'data-pagefind="/pull-requests/42/pagefind/pagefind.js"',
        'action="/pull-requests/42/search.html"',
        'href="//cdn.example.org/x.js"',
        'href="https://www.jupedsim.org/"',
        'href="notes/index.html"',
        'href="#top"',
        "href=&quot;/not-an-attribute&quot;",
    ):
        assert text.count(want) == 1, want


def test_rebase_versions(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    path = tree / "docs" / "versions.json"
    assert previews.rebase_versions(path, PREFIX) == 1
    entries = json.loads(path.read_text())
    assert entries[0]["url"] == "/pull-requests/42/docs/dev/"
    assert entries[1]["url"] == "https://www.jupedsim.org/docs/x/"
    assert previews.rebase_versions(path, PREFIX) == 0


def test_rebase_links_result_and_symlinked_html_skipped(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    result = previews.rebase_links(tree, PREFIX, stable="dev")
    assert result == RebaseResult(files=2, links=7, urls=1, stable_linked=True)
    assert (tree / "alias.html").is_symlink()
    assert (tree / "index.html").read_text().count("/pull-requests/42/") == 5
    routing = (tree / "docs/dev/concepts/routing.html").read_text()
    assert 'data-json="/pull-requests/42/docs/versions.json"' in routing
    assert 'href="/pull-requests/42/docs/v1x/v1.4.2/"' in routing


def test_rebase_is_idempotent(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    previews.rebase_links(tree, PREFIX, stable="dev")
    again = previews.rebase_links(tree, PREFIX, stable="dev")
    assert again == RebaseResult(0, 0, 0, False)
    for path in tree.rglob("*"):
        if path.is_file() and not path.is_symlink():
            assert "pull-requests/42/pull-requests" not in path.read_text()


def test_stable_link_only_when_missing_and_target_exists(
    tmp_path: Path,
) -> None:
    tree = make_preview_tree(tmp_path)
    assert previews.rebase_links(tree, PREFIX, stable="dev").stable_linked
    assert str((tree / "docs" / "stable").readlink()) == "dev"

    tree2 = make_preview_tree(tmp_path / "two")
    (tree2 / "docs" / "stable").symlink_to("other")
    assert not previews.rebase_links(tree2, PREFIX, stable="dev").stable_linked
    assert str((tree2 / "docs" / "stable").readlink()) == "other"

    tree3 = make_preview_tree(tmp_path / "three")
    (tree3 / "docs" / "stable").mkdir()
    assert not previews.rebase_links(tree3, PREFIX, stable="dev").stable_linked
    assert (tree3 / "docs" / "stable").is_dir()
    assert not (tree3 / "docs" / "stable").is_symlink()

    tree4 = make_preview_tree(tmp_path / "four")
    assert not previews.rebase_links(tree4, PREFIX, stable="nope").stable_linked
    assert not (tree4 / "docs" / "stable").exists()
    assert not (tree4 / "docs" / "stable").is_symlink()


def test_rebase_links_usage_errors(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    before = (tree / "index.html").read_text()
    with pytest.raises(UsageError):
        previews.rebase_links(tree, "bad")
    assert (tree / "index.html").read_text() == before
    (tree / "index.html").unlink()
    with pytest.raises(UsageError):
        previews.rebase_links(tree, PREFIX)


def test_find_orphaned_ignores_other_entries(tmp_path: Path) -> None:
    root = make_previews_root(tmp_path)
    found = previews.find_orphaned(root, lookup({12: True, 34: False}))
    assert found == [root / "34"]


def test_find_orphaned_keeps_unknown(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    root = make_previews_root(tmp_path)
    (root / "56").mkdir()
    found = previews.find_orphaned(
        root, lookup({12: True, 34: False, 56: None})
    )
    assert found == [root / "34"]
    assert "#56" in capsys.readouterr().out


def test_prune_deletes_and_returns(tmp_path: Path) -> None:
    root = make_previews_root(tmp_path)
    assert previews.prune(root, lookup({12: True, 34: False})) == [root / "34"]
    assert not (root / "34").exists()
    assert (root / "12" / "index.html").is_file()
    assert (root / "tmp").is_dir() and (root / "README").is_file()


def test_prune_aborts_on_api_error(tmp_path: Path) -> None:
    root = make_previews_root(tmp_path)
    with pytest.raises(ToolError, match="#34"):
        previews.prune(root, lookup({12: False}))
    assert (root / "12").is_dir() and (root / "34").is_dir()


def test_prune_missing_root_is_noop_and_file_root_is_usage_error(
    tmp_path: Path,
) -> None:
    assert previews.prune(tmp_path / "missing", lookup({})) == []
    (tmp_path / "file").write_text("x")
    with pytest.raises(UsageError):
        previews.prune(tmp_path / "file", lookup({}))


def test_github_pr_lookup(monkeypatch: pytest.MonkeyPatch) -> None:
    import github

    class FakePull:
        """A pull request with a state."""

        def __init__(self, state: str) -> None:
            self.state = state

    class FakeRepo:
        """A repository serving pull requests by number."""

        def get_pull(self, number: int) -> FakePull:
            """Return the pull or raise like PyGithub does."""
            if number == 404:
                raise GithubException(404, {"message": "Not Found"}, None)
            if number == 403:
                raise GithubException(403, {"message": "rate limited"}, None)
            return FakePull("open" if number == 1 else "closed")

    class FakeGithub:
        """Records the auth and serves one repository."""

        def __init__(self, auth: object) -> None:
            self.auth = auth

        def get_repo(self, name: str) -> FakeRepo:
            """Serve the fake repository; fail for the unknown one."""
            if name == "owner/missing":
                raise GithubException(401, {"message": "Bad credentials"}, None)
            assert name == "owner/repo"
            return FakeRepo()

    monkeypatch.setattr(github, "Github", FakeGithub)
    is_open = previews.github_pr_lookup("owner/repo", "token")
    assert is_open(1) is True
    assert is_open(2) is False
    assert is_open(404) is None
    with pytest.raises(ToolError, match="403"):
        is_open(403)
    # A failing get_repo (bad token, wrong repository) is a ToolError too.
    with pytest.raises(ToolError, match="owner/missing"):
        previews.github_pr_lookup("owner/missing", "token")


def test_cli_preview_rebase(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    tree = make_preview_tree(tmp_path)
    assert (
        main(["preview", "rebase", str(tree), PREFIX, "--stable", "dev"]) == 0
    )
    out = capsys.readouterr().out
    assert "7 links in 2 HTML files" in out and "docs/stable -> dev" in out
    assert main(["preview", "rebase", str(tree), "bad"]) == 2


def test_cli_prune_requires_token(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.delenv("GITHUB_TOKEN", raising=False)
    root = make_previews_root(tmp_path)
    assert main(["preview", "prune", str(root), "--repo", "o/r"]) == 2
    assert (root / "12").is_dir() and (root / "34").is_dir()


def test_cli_prune_quiet_when_nothing_to_delete(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.setenv("GITHUB_TOKEN", "t")
    monkeypatch.setattr(
        previews,
        "github_pr_lookup",
        lambda repo, token: lookup({12: True, 34: True}),
    )
    root = make_previews_root(tmp_path)
    assert main(["preview", "prune", str(root), "--repo", "o/r"]) == 0
    assert capsys.readouterr().out == ""


def test_cli_prune_with_lookup(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setenv("GITHUB_TOKEN", "t")
    seen: dict[str, str] = {}

    def fake_lookup(repo: str, token: str) -> previews.PullRequestLookup:
        """Record the arguments and serve fixed states."""
        seen["repo"], seen["token"] = repo, token
        return lookup({12: True, 34: False})

    monkeypatch.setattr(previews, "github_pr_lookup", fake_lookup)
    root = make_previews_root(tmp_path)
    assert main(["preview", "prune", str(root), "--repo", "o/r"]) == 0
    assert seen == {"repo": "o/r", "token": "t"}
    assert not (root / "34").exists() and (root / "12").is_dir()


def test_cli_preview_help_footers(capsys: pytest.CaptureFixture[str]) -> None:
    assert main(["preview", "--help"]) == 0
    assert "PyGithub" in capsys.readouterr().out
    assert main(["preview", "rebase", "--help"]) == 0
    assert "Additional packages: none" in capsys.readouterr().out
    assert main(["preview", "prune", "--help"]) == 0
    assert "PyGithub" in capsys.readouterr().out
