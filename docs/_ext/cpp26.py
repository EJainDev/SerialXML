"""Highlight C++26 annotation expressions, including their whitespace and braces."""

from pygments.lexer import bygroups, include, inherit
from pygments.lexers import CppLexer
from pygments.token import Operator, Punctuation, Whitespace


class Cpp26Lexer(CppLexer):
    tokens = {
        state: [
            (r"(\[\[)(\s*)(=)", bygroups(Punctuation, Whitespace, Operator), "annotation-expression"),
            inherit,
        ]
        for state in ("statements", "classname", "enumname")
    }
    tokens["annotation-expression"] = [
        (r"\]\]", Punctuation, "#pop"),
        (r"[{}]", Punctuation),
        include("whitespace"),
        include("statements"),
    ]


def setup(app):
    app.add_lexer("cpp", Cpp26Lexer)
    return {"version": "1.0", "parallel_read_safe": True, "parallel_write_safe": True}
