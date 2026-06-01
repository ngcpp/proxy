"""Repository rule that generates doc_examples.bzl by scanning docs/*.md files."""

def _doc_examples_impl(ctx):
    script = ctx.path(ctx.attr._script)
    docs_dir = ctx.path(ctx.attr._docs_marker).dirname
    python = ctx.path(ctx.attr._python)

    ctx.watch(script)
    ctx.watch(ctx.path(ctx.attr._extract_script))
    ctx.watch_tree(docs_dir)

    result = ctx.execute([str(python), str(script), "--format", "bzl"])
    if result.return_code != 0:
        fail("gen_doc_examples.py failed:\n" + result.stderr)

    ctx.file("doc_examples.bzl", result.stdout)
    ctx.file("BUILD.bazel", "")

doc_examples = repository_rule(
    implementation = _doc_examples_impl,
    attrs = {
        "_script": attr.label(default = "//tools:gen_doc_examples.py"),
        "_extract_script": attr.label(default = "//tools:extract_example_code.py"),
        "_docs_marker": attr.label(default = "//docs:BUILD.bazel"),
        "_python": attr.label(default = "@bootstrap_python//:python"),
    },
)
