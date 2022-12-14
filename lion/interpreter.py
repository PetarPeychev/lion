from copy import deepcopy

from lion.words import Symbol, Quote
from lion import builtins


class Interpreter:
    def __init__(self):
        self.stack = []
        self.builtins = {
            Symbol("error"): builtins.l_error,
            Symbol("call"): builtins.l_call,
            Symbol("run"): builtins.l_run,
            Symbol("wrap"): builtins.l_wrap,
            Symbol("unwrap"): builtins.l_unwrap,
            Symbol("uncons"): builtins.l_uncons,
            Symbol("def"): builtins.l_def,
            Symbol("defm"): builtins.l_defm,
            Symbol("parse"): builtins.l_parse,
            Symbol("type"): builtins.l_type,
            Symbol("str"): builtins.l_string,
            Symbol("num"): builtins.l_number,
            Symbol("sym"): builtins.l_symbol,
            Symbol("print"): builtins.l_print,
            Symbol("read"): builtins.l_read,
            Symbol("write"): builtins.l_write,
            Symbol("+"): builtins.l_add,
            Symbol("-"): builtins.l_subtract,
            Symbol("*"): builtins.l_multiply,
            Symbol("/"): builtins.l_divide,
            Symbol("split"): builtins.l_split,
            Symbol("join"): builtins.l_join,
            Symbol("ifte"): builtins.l_ifte,
            Symbol("eq?"): builtins.l_eq,
        }

    def error(self, msg: str) -> None:
        print(self.stack)
        raise RuntimeError(msg)

    def evaluate(
        self, quote: Quote, vocabulary: dict[Symbol, tuple[bool, Quote]]
    ) -> None:
        if not isinstance(quote, Quote):
            self.error(f"can't evaluate a word of type {type(quote)}")
        for word in quote.val:
            if isinstance(word, Symbol):
                if word in self.builtins:
                    self.builtins[word](self, vocabulary)
                elif word in vocabulary:
                    if vocabulary[word][0]:
                        self.evaluate(vocabulary[word][1], deepcopy(vocabulary))
                    else:
                        self.evaluate(vocabulary[word][1], vocabulary)
                else:
                    self.error(f"unrecognised word {word}")
            else:
                self.stack.append(word)
