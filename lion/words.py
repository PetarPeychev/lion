from __future__ import annotations
from dataclasses import dataclass


@dataclass
class Number:
    val: float
    typename = "num"

    def __repr__(self) -> str:
        return f"{self.val:g}"

    def __hash__(self) -> int:
        return hash(self.val)


@dataclass
class String:
    val: str
    typename = "str"

    def __repr__(self) -> str:
        return '"' + self.val + '"'

    def __hash__(self) -> int:
        return hash(self.val)


@dataclass
class Boolean:
    val: bool
    typename = "bool"

    def __repr__(self) -> str:
        return "true" if self.val else "false"

    def __hash__(self) -> int:
        return hash(self.val)


@dataclass
class Symbol:
    val: str
    typename = "symb"

    def __repr__(self) -> str:
        return self.val

    def __hash__(self) -> int:
        return hash(self.val)


@dataclass
class Quote:
    val: list[Word]
    typename = "block"

    def __repr__(self) -> str:
        words = " ".join(map(repr, self.val))
        return "(" + words + ")"

    def __hash__(self) -> int:
        return hash(self.val)


Word = Number | String | Boolean | Symbol | Quote
