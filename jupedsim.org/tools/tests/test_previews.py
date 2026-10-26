# SPDX-License-Identifier: LGPL-3.0-or-later
"""PR previews: links rebased under a prefix, previews of closed PRs pruned."""

import json
from pathlib import Path

import pytest
from github import GithubException
from jps_website_tooling import previews
from jps_website_tooling.cli import main
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.previews import (
    RebaseResult,
    prune_previews,
    publish_preview,
)
from tests.conftest import PagesRepo, run_git, write

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
    result = previews.rebase_links(tree, PREFIX)
    assert result == RebaseResult(files=2, links=7, urls=1)
    assert (tree / "alias.html").is_symlink()
    assert (tree / "index.html").read_text().count("/pull-requests/42/") == 5
    routing = (tree / "docs/dev/concepts/routing.html").read_text()
    assert 'data-json="/pull-requests/42/docs/versions.json"' in routing
    assert 'href="/pull-requests/42/docs/v1x/v1.4.2/"' in routing


def test_rebase_is_idempotent(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    previews.rebase_links(tree, PREFIX)
    again = previews.rebase_links(tree, PREFIX)
    assert again == RebaseResult(0, 0, 0)
    for path in tree.rglob("*"):
        if path.is_file() and not path.is_symlink():
            assert "pull-requests/42/pull-requests" not in path.read_text()


def test_rebase_links_leaves_docs_stable_alone(tmp_path: Path) -> None:
    # Previews build with version "stable": docs/stable is a directory and
    # rebase_links never creates or touches a docs/stable symlink.
    tree = make_preview_tree(tmp_path)
    (tree / "docs" / "stable").mkdir()
    (tree / "docs" / "stable" / "index.html").write_text("<p>stable</p>\n")
    previews.rebase_links(tree, PREFIX)
    assert (tree / "docs" / "stable").is_dir()
    assert not (tree / "docs" / "stable").is_symlink()
    tree2 = make_preview_tree(tmp_path / "two")
    previews.rebase_links(tree2, PREFIX)
    assert not (tree2 / "docs" / "stable").exists()


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


def test_cli_prune_requires_token(
    previews_repo: PagesRepo, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.delenv("GITHUB_TOKEN", raising=False)
    monkeypatch.setenv("GITHUB_REPOSITORY", "o/r")
    tip = previews_repo.tip()
    args = [
        "preview",
        "prune",
        "--checkout",
        str(previews_repo.checkout),
    ]
    assert main([*args]) == 2
    assert previews_repo.tip() == tip


def test_cli_prune_with_lookup(
    previews_repo: PagesRepo,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.setenv("GITHUB_TOKEN", "t")
    monkeypatch.setenv("GITHUB_REPOSITORY", "o/r")
    seen: dict[str, str] = {}

    def fake_lookup(repo: str, token: str) -> previews.PullRequestLookup:
        """Record the arguments and serve fixed states."""
        seen["repo"], seen["token"] = repo, token
        return lookup({7: False, 8: True})

    monkeypatch.setattr(previews, "github_pr_lookup", fake_lookup)
    args = [
        "preview",
        "prune",
        "--checkout",
        str(previews_repo.checkout),
    ]
    assert main(args) == 0
    assert seen == {"repo": "o/r", "token": "t"}
    out = capsys.readouterr().out
    assert f"pages_sha={previews_repo.tip()}\nchanged=true\ndeleted=7\n" in out
    tree = previews_repo.tree()
    assert (
        not (tree / "pull-requests/7").exists()
        and (tree / "pull-requests/8").is_dir()
    )
    # Nothing left to delete: no commit, empty deleted output.
    tip = previews_repo.tip()
    assert main(args) == 0
    assert "changed=false\ndeleted=\n" in capsys.readouterr().out
    assert previews_repo.tip() == tip


def test_cli_preview_help_footers(capsys: pytest.CaptureFixture[str]) -> None:
    assert main(["preview", "--help"]) == 0
    assert "PyGithub" in capsys.readouterr().out
    assert main(["preview", "publish", "--help"]) == 0
    assert "Additional packages: none" in capsys.readouterr().out
    assert main(["preview", "prune", "--help"]) == 0
    assert "PyGithub" in capsys.readouterr().out


@pytest.fixture
def previews_repo(tmp_path: Path, git_env: None) -> PagesRepo:
    """The previews repository: branch ``main`` with two previews.

    Args:
        tmp_path: Pytest's scratch directory.
        git_env: Git isolation (dependency only).

    Returns:
        The bare repository and a clean clone, on ``main``.
    """
    bare = tmp_path / "previews.git"
    run_git(["init", "-q", "--bare", str(bare)], tmp_path)
    seed = tmp_path / "seed"
    run_git(["clone", "-q", str(bare), str(seed)], tmp_path)
    write(seed / "index.html", "<p>previews</p>\n")
    for n in (7, 8):
        write(seed / "pull-requests" / str(n) / "index.html", f"<p>{n}</p>\n")
    run_git(["checkout", "-q", "-b", "main"], seed)
    run_git(["add", "-A"], seed)
    run_git(["commit", "-q", "-m", "seed"], seed)
    run_git(["push", "-q", "origin", "main"], seed)
    checkout = tmp_path / "previews"
    run_git(["clone", "-q", "-b", "main", str(bare), str(checkout)], tmp_path)
    return PagesRepo(bare, checkout, "main")


def preview_build(tmp: Path) -> Path:
    """A website build as the PR build produces it (docs under docs/stable/).

    Args:
        tmp: Parent directory.

    Returns:
        The build directory.
    """
    build = tmp / "build"
    write(build / "index.html", INDEX_HTML)
    write(
        build / "docs" / "stable" / "index.html", '<a href="/articles/">A</a>\n'
    )
    write(build / "docs" / "versions.json", json.dumps(VERSIONS) + "\n")
    write(build / "pagefind" / "pagefind.js", "// index\n")
    return build


def test_publish_preview(previews_repo: PagesRepo, tmp_path: Path) -> None:
    build = preview_build(tmp_path)
    r = publish_preview(build, 42, previews_repo.checkout)
    assert r.changed and r.pages_sha == previews_repo.tip()
    assert previews_repo.message() == "docs: preview for PR #42"
    tree = previews_repo.tree()
    html = (tree / "pull-requests/42/index.html").read_text()
    assert 'href="/pull-requests/42/docs/stable/"' in html
    assert 'data-pagefind="/pull-requests/42/pagefind/pagefind.js"' in html
    docs = (tree / "pull-requests/42/docs/stable/index.html").read_text()
    assert 'href="/pull-requests/42/articles/"' in docs
    assert (tree / "pull-requests/7/index.html").is_file()
    assert (tree / "index.html").is_file()


def test_publish_preview_unchanged(
    previews_repo: PagesRepo, tmp_path: Path
) -> None:
    build = preview_build(tmp_path)
    first = publish_preview(build, 42, previews_repo.checkout)
    second = publish_preview(build, 42, previews_repo.checkout)
    assert first.changed and not second.changed
    assert second.pages_sha == first.pages_sha == previews_repo.tip()


def test_publish_preview_retried_after_rejection(
    previews_repo: PagesRepo, tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    build = preview_build(tmp_path)
    real = previews.rsync_tree
    calls = {"n": 0}

    def racing_rsync(
        src: Path, dst: Path, excludes: tuple[str, ...] = ()
    ) -> None:
        """Push a competitor on the first staging, then rsync as usual."""
        calls["n"] += 1
        if calls["n"] == 1:
            previews_repo.push_competitor("competitor.html")
        real(src, dst, excludes)

    monkeypatch.setattr(previews, "rsync_tree", racing_rsync)
    slept: list[float] = []
    r = publish_preview(build, 42, previews_repo.checkout, sleep=slept.append)
    assert r.changed and slept == [5] and calls["n"] == 2
    tree = previews_repo.tree()
    assert (tree / "competitor.html").is_file()
    assert (tree / "pull-requests/42/index.html").is_file()


def test_publish_preview_refuses_bad_input(
    previews_repo: PagesRepo, tmp_path: Path
) -> None:
    build = preview_build(tmp_path)
    tip = previews_repo.tip()
    with pytest.raises(UsageError, match="number"):
        publish_preview(build, 0, previews_repo.checkout)
    (tmp_path / "empty").mkdir()
    with pytest.raises(UsageError, match="index.html"):
        publish_preview(tmp_path / "empty", 1, previews_repo.checkout)
    inside = preview_build(previews_repo.checkout / "artifact")
    with pytest.raises(UsageError, match="inside the checkout"):
        publish_preview(inside, 1, previews_repo.checkout)
    assert previews_repo.tip() == tip


def test_prune_previews_commits_numbers(previews_repo: PagesRepo) -> None:
    result, deleted = prune_previews(
        previews_repo.checkout, lookup({7: False, 8: True})
    )
    assert result.changed and deleted == [7]
    assert previews_repo.message() == "docs: delete orphaned PR previews (#7)"
    tree = previews_repo.tree()
    assert not (tree / "pull-requests/7").exists()
    assert (tree / "pull-requests/8/index.html").is_file()


def test_prune_previews_nothing_to_delete(previews_repo: PagesRepo) -> None:
    tip = previews_repo.tip()
    result, deleted = prune_previews(
        previews_repo.checkout, lookup({7: True, 8: True})
    )
    assert not result.changed and deleted == [] and previews_repo.tip() == tip


def test_cli_preview_publish(
    previews_repo: PagesRepo, tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    build = preview_build(tmp_path)
    args = [
        "preview",
        "publish",
        "--build",
        str(build),
        "--number",
        "42",
        "--checkout",
        str(previews_repo.checkout),
    ]
    assert main(args) == 0
    assert (
        f"pages_sha={previews_repo.tip()}\nchanged=true\n"
        in capsys.readouterr().out
    )
    assert main([*args[:5], "x", *args[6:]]) == 2  # --number must be an int


def test_prune_previews_recomputes_per_attempt(
    previews_repo: PagesRepo, monkeypatch: pytest.MonkeyPatch
) -> None:
    # During attempt 1 a competitor removes #7 itself and adds a closed #9;
    # the pushed commit must reflect what attempt 2 actually deleted.
    real = previews.prune
    calls = {"n": 0}

    def racing_prune(
        root: Path, is_open: previews.PullRequestLookup
    ) -> list[Path]:
        """Let a competitor change the branch before the first deletion."""
        calls["n"] += 1
        if calls["n"] == 1:
            other = previews_repo.tree()
            run_git(["rm", "-rq", "pull-requests/7"], other)
            write(other / "pull-requests" / "9" / "index.html", "<p>9</p>\n")
            run_git(["add", "-A"], other)
            run_git(["commit", "-q", "-m", "competitor"], other)
            run_git(["push", "-q", "origin", "main"], other)
        return real(root, is_open)

    monkeypatch.setattr(previews, "prune", racing_prune)
    slept: list[float] = []
    result, deleted = prune_previews(
        previews_repo.checkout,
        lookup({7: False, 8: True, 9: False}),
        sleep=slept.append,
    )
    assert result.changed and slept == [5] and deleted == [9]
    assert previews_repo.message() == "docs: delete orphaned PR previews (#9)"
    tree = previews_repo.tree()
    assert not (tree / "pull-requests/9").exists()
    assert (tree / "pull-requests/8/index.html").is_file()


def test_cli_prune_reads_github_repository(
    previews_repo: PagesRepo, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setenv("GITHUB_TOKEN", "t")
    monkeypatch.delenv("GITHUB_REPOSITORY", raising=False)
    tip = previews_repo.tip()
    args = ["preview", "prune", "--checkout", str(previews_repo.checkout)]
    assert main(args) == 2  # GITHUB_REPOSITORY missing
    seen: dict[str, str] = {}

    def fake_lookup(repo: str, token: str) -> previews.PullRequestLookup:
        """Record the repository and serve fixed states."""
        seen["repo"] = repo
        return lookup({7: True, 8: True})

    monkeypatch.setattr(previews, "github_pr_lookup", fake_lookup)
    monkeypatch.setenv("GITHUB_REPOSITORY", "PedestrianDynamics/jupedsim")
    assert main(args) == 0
    assert seen == {"repo": "PedestrianDynamics/jupedsim"}
    assert previews_repo.tip() == tip
    assert main([*args, "--repo", "o/r"]) == 2
    assert main([*args, "--branch", "main"]) == 2


def test_cli_preview_rebase_is_gone(tmp_path: Path) -> None:
    tree = make_preview_tree(tmp_path)
    assert main(["preview", "rebase", str(tree), PREFIX]) == 2
