#!/usr/bin/env python3
# pyright: strict

"""Bump the C++ dependencies that Renovate cannot manage.

Run by .github/workflows/pipeline-bump-cpp-deps.yml, covering two families:

  cmake   GitHub-archive entries in cmake/dependencies.json and
          tools/report_generator/dependencies.json, bumped to each repo's latest release with
          the sha256 recomputed.
  meson   subprojects/*.wrap, refreshed via `meson wrap update`.
"""

import hashlib
import json
import os
import re
import shutil
import subprocess
import urllib.error
import urllib.request
from pathlib import Path
from typing import NoReturn

_REPO_ROOT = Path(__file__).resolve().parent.parent
_ARCHIVE_URL_RE = re.compile(
    r"https://github\.com/([^/]+/[^/]+)/archive/refs/tags/(.+)\.tar\.gz"
)
_WRAP_DIR_RE = re.compile(r"^directory\s*=\s*(.+)$", re.MULTILINE)


def _abort(msg: str) -> NoReturn:
    print(f"error: {msg}", flush=True)
    raise SystemExit(1)


def _http_get(url: str, *, accept: str | None = None) -> bytes | None:
    req = urllib.request.Request(url)
    req.add_header("User-Agent", "proxy-bump-dependencies")
    if accept is not None:
        req.add_header("Accept", accept)
    token = os.environ.get("GITHUB_TOKEN")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    try:
        with urllib.request.urlopen(req, timeout=120) as resp:
            return resp.read()
    except (urllib.error.URLError, TimeoutError):
        return None


def _github_latest_tag(repo: str) -> str | None:
    body = _http_get(
        f"https://api.github.com/repos/{repo}/releases/latest",
        accept="application/json",
    )
    if body is None:
        return None
    try:
        data = json.loads(body)
    except json.JSONDecodeError:
        return None
    if isinstance(data, dict):
        name = data.get("tag_name")
        if isinstance(name, str):
            return name
    return None


def _update_registry(rel_path: str, label: str) -> None:
    """Bump each GitHub-archive entry to its latest release (downloading only on a change)."""
    print(f"Checking {label} ({rel_path}) ...", flush=True)
    path = _REPO_ROOT / rel_path
    deps = json.loads(path.read_text(encoding="utf-8"))
    changed = False
    for dep in deps:
        name, url = dep["name"], dep["url"]
        m = _ARCHIVE_URL_RE.fullmatch(url)
        if m is None:
            _abort(f"{label}/{name}: url is not a GitHub archive tarball: {url}")
        repo, current = m.group(1), m.group(2)
        latest = _github_latest_tag(repo)
        if latest is None:
            _abort(f"{label}/{name}: could not determine the latest release of {repo}")
        if latest == current:
            continue  # already current -- no download
        new_url = f"https://github.com/{repo}/archive/refs/tags/{latest}.tar.gz"
        body = _http_get(new_url)
        if body is None:
            _abort(f"{label}/{name}: could not download {new_url}")
        print(f"  {name}: {current} -> {latest}", flush=True)
        dep["url"] = new_url
        dep["sha256"] = hashlib.sha256(body).hexdigest()
        changed = True
    if changed:
        path.write_text(json.dumps(deps, indent=2) + "\n", encoding="utf-8")


def _wrap_version(wrap: Path) -> str:
    m = _WRAP_DIR_RE.search(wrap.read_text(encoding="utf-8"))
    return m.group(1).strip() if m else "?"


def _update_meson() -> None:
    print("Updating Meson wraps ...", flush=True)
    wraps = sorted((_REPO_ROOT / "subprojects").glob("*.wrap"))
    if not wraps:
        _abort("no wrap files found in subprojects/")
    if shutil.which("meson") is None:
        _abort("meson not found on PATH")
    for wrap in wraps:
        name = wrap.stem
        before = _wrap_version(wrap)
        result = subprocess.run(
            ["meson", "wrap", "update", name],
            cwd=_REPO_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            _abort(f"`meson wrap update {name}` failed: {result.stderr.strip()}")
        after = _wrap_version(wrap)
        if after != before:
            print(f"  {name}: {before} -> {after}", flush=True)


if __name__ == "__main__":
    _update_registry("cmake/dependencies.json", "cmake")
    _update_registry("tools/report_generator/dependencies.json", "report_generator")
    _update_meson()
