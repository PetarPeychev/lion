from pathlib import Path

from lion.words import Word, String, Quote, Symbol
from lion.parser import parse

# Utility Functions (Not in Builtins)
def take_type(i, types: tuple[Word]) -> Word:
    if len(i.stack) > 0 and isinstance(i.stack[-1], types):
        return i.stack.pop()
    else:
        i.error(f"expected word of type {types}")


# Concatenative Combinators (http://tunes.org/~iepos/joy.html)
def l_dup(i) -> None:
    word = take_type(i, Word)
    i.stack.append(word)
    i.stack.append(word)


def l_drop(i) -> None:
    take_type(i, Word)


def l_swap(i) -> None:
    first = take_type(i, Word)
    second = take_type(i, Word)
    i.stack.append(first)
    i.stack.append(second)


def l_dip(i) -> None:
    quote = take_type(i, Quote)
    top = take_type(i, Word)
    i.evaluate(quote)
    i.stack.append(top)


def l_eval(i) -> None:
    quote = take_type(i, Quote)
    i.evaluate(quote)


def l_quote(i) -> None:
    word = take_type(i, Word)
    i.stack.append(Quote([word]))


def l_cat(i) -> None:
    quote_first: Quote = take_type(i, Quote)
    quote_second: Quote = take_type(i, Quote)
    i.stack.append(Quote(quote_second.val.extend(quote_first.val)))


def l_cons(i) -> None:
    quote = take_type(i, Quote)
    word = take_type(i, Word)
    i.stack.append(Quote([word] + quote.val))


# Input/Output
def l_print(i) -> None:
    string: String = take_type(i, (String))
    print(bytes(string.val, "utf-8").decode("unicode_escape"), end="")


def l_read(i) -> None:
    string = take_type(i, String)
    i.stack.append(String(Path(string.val).read_text()))


def l_write(i) -> None:
    path = take_type(i, String)
    string = take_type(i, String)
    Path(path.val).write_text(string.val)


# Inception
def l_parse(i) -> None:
    string = take_type(i, String)
    i.stack.append(parse(string.val))


def l_def(i) -> None:
    quote = take_type(i, Quote)
    name = take_type(i, Quote)

    if len(name.val) == 1 and isinstance(name.val[0], Symbol):
        i.vocabulary[name.val[0]] = quote
    else:
        i.error("word for new vocabulary entry must be a Symbol")
