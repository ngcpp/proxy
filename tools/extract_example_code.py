#!/usr/bin/env python3
# pyright: strict

"""Extract the C++ example from a Markdown documentation file.

Usage: extract_example_code.py INPUT.md OUTPUT.cpp
"""

import re
import sys
from pathlib import Path
from typing import Optional


def try_extract_example_code(md_path: Path) -> Optional[str]:
    """Return the generated C++ source for *md_path*, or None if it has no example."""
    example_pattern = re.compile(
        r"## Example\r?\n\r?\n```cpp\r?\n(.*?)\r?\n```", re.DOTALL
    )
    with open(md_path, "r", encoding="utf-8") as f:
        content = f.read()

    blocks: list[str] = re.findall(example_pattern, content)
    if len(blocks) == 0:
        return None
    if len(blocks) > 1:
        raise ValueError(f"'{md_path}' has more than one '## Example' C++ block.")

    code = blocks[0]
    return f"// This file was auto-generated from:\n// {md_path}\n\n{code}"


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} INPUT.md OUTPUT.cpp", file=sys.stderr)
        sys.exit(1)
    code = try_extract_example_code(Path(sys.argv[1]))
    if code is not None:
        Path(sys.argv[2]).write_text(code, encoding="utf-8")
