"""Generate an exact function source-body model, not a full target link.

Usage: extract_main.py <source> <output> [signature-regex]

The signature defaults to `int main()`; pass e.g. `void init_init\\(\\)` to model
another entry function from the same file without hand-copying its body.
"""
import pathlib
import re
import sys

source = pathlib.Path(sys.argv[1])
signature = sys.argv[3] if len(sys.argv) > 3 else r"int main\(\)"
text = source.read_text()
start = re.search(rf"^{signature}\s*\{{", text, re.MULTILINE)
if start is None:
    raise SystemExit(f"actual {signature} definition not found")
# Count braces outside comments/string literals, without rewriting the body.
tokens = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.DOTALL)
depth = 0
for token in tokens.finditer(text, start.end() - 1):
    if token.group() == "{":
        depth += 1
    elif token.group() == "}":
        depth -= 1
        if depth == 0:
            end = token.end()
            break
else:
    raise SystemExit(f"actual {signature} definition has unbalanced braces")
config = re.findall(r"^#define (?:RUN_TEST|TEST_ID|RUN_UNIT_TESTS)\b[^\n]*", text, re.MULTILINE)
line = text.count("\n", 0, start.start()) + 1
pathlib.Path(sys.argv[2]).write_text(
    f"/* Generated verbatim from actual {signature}; host boundary facades only. */\n"
    + "\n".join(config) + f'\n#line {line} "{source.as_posix()}"\n'
    + text[start.start():end] + "\n"
)
