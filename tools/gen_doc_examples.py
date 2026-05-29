#!/usr/bin/env python3
# pyright: strict

"""Generate the doc-test example list in one of three build-system formats.

Scans docs/** for Markdown files that contain a ## Example C++ code block
and writes the result to stdout in the requested format.

  --format json    JSON array consumed by CMake:   [{md, deps}, ...]
  --format txt     Colon-separated lines for Meson: <md> or <md>:<dep1>,...

Run from the repository root:
  python3 tools/gen_doc_examples.py --format <fmt>
"""

import argparse
import json
import sys
from dataclasses import dataclass
from pathlib import Path

from extract_example_code import try_extract_example_code


@dataclass(frozen=True)
class DepInfo:
    cmake: str  # CMake link target,    e.g. "fmt::fmt"
    meson: str  # Meson dep variable,   e.g. "fmt_dep"


@dataclass(frozen=True)
class Example:
    rel_md: str           # path relative to docs/, e.g. "spec/allocate_proxy.md"
    deps: tuple[str, ...]  # logical dep names, e.g. ("fmt",)


_REPO_ROOT = Path(__file__).parent.parent
_DOCS_DIR = _REPO_ROOT / "docs"

# Add one entry here when introducing a new library dependency.
_DEPS: dict[str, DepInfo] = {
    "fmt": DepInfo(cmake="fmt::fmt", meson="fmt_dep"),
}

# Hardcoded per-file dependencies (path relative to docs/).
_FILE_DEPS: dict[str, tuple[str, ...]] = {
    "spec/skills_fmt_format.md": ("fmt",),
}

def _collect(docs_dir: Path) -> list[Example]:
    examples: list[Example] = []
    for md in sorted(docs_dir.rglob("*.md")):
        if try_extract_example_code(md) is None:
            continue
        rel_md = md.relative_to(docs_dir).as_posix()
        examples.append(Example(rel_md=rel_md, deps=_FILE_DEPS.get(rel_md, ())))
    return examples


def _generate_json(examples: list[Example]) -> str:
    return json.dumps([{"md": e.rel_md, "deps": [_DEPS[d].cmake for d in e.deps]} for e in examples]) + "\n"


def _generate_txt(examples: list[Example]) -> str:
    lines: list[str] = []
    for e in examples:
        lines.append(f"{e.rel_md}:{','.join(_DEPS[d].meson for d in e.deps)}" if e.deps else e.rel_md)
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--format", choices=["json", "txt"], required=True)
    args = parser.parse_args()

    examples = _collect(_DOCS_DIR)
    generators = {"json": _generate_json, "txt": _generate_txt}
    sys.stdout.write(generators[args.format](examples))


if __name__ == "__main__":
    main()
