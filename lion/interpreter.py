from lion.words import Symbol, Quote
from lion import builtins


class Interpreter:
    def __init__(self):
        self.stack = []
        self.builtins = {
            Symbol("dup"): builtins.l_dup,
            Symbol("drop"): builtins.l_drop,
            Symbol("swap"): builtins.l_swap,
            Symbol("dip"): builtins.l_dip,
            Symbol("eval"): builtins.l_eval,
            Symbol("quote"): builtins.l_quote,
            Symbol("cat"): builtins.l_cat,
            Symbol("cons"): builtins.l_cons,
            Symbol("uncons"): builtins.l_uncons,
            Symbol("def"): builtins.l_def,
            Symbol("parse"): builtins.l_parse,
            Symbol("type"): builtins.l_type,
            Symbol("string"): builtins.l_string,
            Symbol("number"): builtins.l_number,
            Symbol("symbol"): builtins.l_symbol,
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
            Symbol("and"): builtins.l_and,
            Symbol("or"): builtins.l_or,
            Symbol("not"): builtins.l_not,
            Symbol("eq"): builtins.l_eq,
        }
        self.vocabulary = {}

    def error(self, msg: str) -> None:
        raise RuntimeError(msg)

    def evaluate(self, quote: Quote) -> None:
        if not isinstance(quote, Quote):
            self.error(f"can't evaluate a word of type {type(quote)}")
        for word in quote.val:
            if isinstance(word, Symbol):
                if word in self.builtins:
                    self.builtins[word](self)
                elif word in self.vocabulary:
                    self.evaluate(self.vocabulary[word])
                else:
                    self.error(f"unrecognised word {word}")
            else:
                self.stack.append(word)
