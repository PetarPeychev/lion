from pathlib import Path
from copy import deepcopy

from lion.words import Word, String, Quote, Symbol, Number, Boolean
from lion.parser import parse

# Utility Functions (Not in Builtins)
def take_type(i, types: tuple[Word]) -> Word:
    if len(i.stack) > 0 and isinstance(i.stack[-1], types):
        return i.stack.pop()
    else:
        i.error(f"expected word of type {types}")


def l_call(i, v) -> None:
    quote = take_type(i, Quote)
    i.evaluate(quote, deepcopy(v))


def l_run(i, v) -> None:
    quote = take_type(i, Quote)
    i.evaluate(quote, v)


def l_wrap(i, v) -> None:
    word = take_type(i, Word)
    i.stack.append(Quote([word]))


def l_unwrap(i, v) -> None:
    quote = take_type(i, Quote)
    for word in quote.val:
        i.stack.append(word)


def l_cat(i, v) -> None:
    quote_first: Quote = take_type(i, Quote)
    quote_second: Quote = take_type(i, Quote)
    i.stack.append(Quote(quote_second.val + quote_first.val))


def l_uncons(i, v) -> None:
    quote = take_type(i, Quote)

    if len(quote.val) > 0:
        i.stack.append(quote.val[0])
        i.stack.append(Quote(quote.val[1:]))
    else:
        i.error("attempted uncons on empty quote")


# Input/Output
def l_print(i, v) -> None:
    string: String = take_type(i, (String))
    print(bytes(string.val, "utf-8").decode("unicode_escape"), end="")


def l_read(i, v) -> None:
    string = take_type(i, String)
    i.stack.append(String(Path(string.val).read_text()))


def l_write(i, v) -> None:
    path = take_type(i, String)
    string = take_type(i, String)
    Path(path.val).write_text(string.val)


# Inception
def l_parse(i, v) -> None:
    string = take_type(i, String)
    i.stack.append(parse(string.val))


def l_defun(i, v) -> None:
    name = take_type(i, Quote)
    quote = take_type(i, Quote)

    if len(name.val) == 1 and isinstance(name.val[0], Symbol):
        if name.val[0] in v:
            i.error(f"definition for {str(name.val[0])} already exists")
        v[name.val[0]] = (True, quote)
    else:
        i.error("word for new vocabulary entry must be a Symbol")


def l_defmacro(i, v) -> None:
    name = take_type(i, Quote)
    quote = take_type(i, Quote)

    if len(name.val) == 1 and isinstance(name.val[0], Symbol):
        if name.val[0] in v:
            i.error(f"definition for {str(name.val[0])} already exists")
        v[name.val[0]] = (False, quote)
    else:
        i.error("word for new vocabulary entry must be a Symbol")


def l_type(i, v) -> None:
    word = take_type(i, Word)
    i.stack.append(String(word.typename))


def l_string(i, v) -> None:
    word = take_type(i, Word)
    i.stack.append(String(str(word)))


def l_number(i, v) -> None:
    string = take_type(i, String)
    i.stack.append(Number(float(string)))


def l_symbol(i, v) -> None:
    string = take_type(i, String)
    i.stack.append(Symbol(string.val))


# Math
def l_add(i, v) -> None:
    num_first = take_type(i, Number)
    num_second = take_type(i, Number)
    i.stack.append(Number(num_second.val + num_first.val))


def l_subtract(i, v) -> None:
    num_first = take_type(i, Number)
    num_second = take_type(i, Number)
    i.stack.append(Number(num_second.val - num_first.val))


def l_multiply(i, v) -> None:
    num_first = take_type(i, Number)
    num_second = take_type(i, Number)
    i.stack.append(Number(num_second.val * num_first.val))


def l_divide(i, v) -> None:
    num_first = take_type(i, Number)
    num_second = take_type(i, Number)
    i.stack.append(Number(num_second.val / num_first.val))


# Strings
def l_split(i, v) -> None:
    string: String = take_type(i, String)
    quote = Quote([String(s) for s in string.val])
    i.stack.append(quote)


def l_join(i, v) -> None:
    quote: Quote = take_type(i, Quote)
    string = String("".join([x.val for x in quote.val]))
    i.stack.append(string)


# Logical Operators
def l_ifte(i, v) -> None:
    quote_else: Quote = take_type(i, Quote)
    quote_then: Quote = take_type(i, Quote)
    boolean: Boolean = take_type(i, Boolean)

    if boolean.val:
        i.evaluate(quote_then, deepcopy(v))
    else:
        i.evaluate(quote_else, deepcopy(v))


def l_and(i, v) -> None:
    boolean1: Boolean = take_type(i, Boolean)
    boolean2: Boolean = take_type(i, Boolean)
    i.stack.append(Boolean(boolean2.val and boolean1.val))


def l_or(i, v) -> None:
    boolean1: Boolean = take_type(i, Boolean)
    boolean2: Boolean = take_type(i, Boolean)
    i.stack.append(Boolean(boolean2.val or boolean1.val))


def l_not(i, v) -> None:
    boolean: Boolean = take_type(i, Boolean)
    i.stack.append(Boolean(not boolean.val))


def l_eq(i, v) -> None:
    word1: Word = take_type(i, Word)
    word2: Word = take_type(i, Word)
    i.stack.append(Boolean(word2 == word1))
