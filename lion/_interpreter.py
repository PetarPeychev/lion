import re
from pathlib import Path


def parse(code: str):
    no_comments = re.sub(r"#[^#\n]*$", "", code)
    tokens = re.findall(r'(?:[^\s"]+|"(?:[^"])*")', no_comments)

    no_parens = []
    for t in tokens:
        if not (t.startswith('"') and t.endswith('"')):
            split_parens = re.split(r"(\(|\))", t)
            no_parens.extend([tok for tok in split_parens if tok != ""])
        else:
            no_parens.append(t)

    tokens_parsed = []
    for t in no_parens:
        if t in ["true", "false"]:
            tokens_parsed.append(t == "true")
        elif is_number(t):
            tokens_parsed.append(float(t))
        else:
            tokens_parsed.append(t)

    return tokens_parsed


def is_number(string: str):
    try:
        float(string)
        return True
    except ValueError:
        return False


def define():
    quote = stack.pop()
    name = stack.pop()[0]
    if name in vocabulary:
        raise RuntimeError(f"atom with the name {name} already exists")
    vocabulary[name] = quote


def swap():
    top = stack.pop()
    bot = stack.pop()
    stack.append(top)
    stack.append(bot)


def dip():
    quote = stack.pop()
    top = stack.pop()
    eval(quote)
    stack.append(top)


def plus():
    top = stack.pop()
    bottom = stack.pop()
    stack.append(bottom + top)


def minus():
    top = stack.pop()
    bottom = stack.pop()
    stack.append(bottom - top)


def times():
    top = stack.pop()
    bottom = stack.pop()
    stack.append(bottom * top)


def div():
    top = stack.pop()
    bottom = stack.pop()
    stack.append(bottom / top)


stack = []
quoted_atoms = []
quote_level = 0
vocabulary = {
    "def": define,
    "dup": lambda: stack.append(stack[-1]),
    "drop": lambda: stack.pop(),
    "swap": swap,
    "dip": dip,
    "print": lambda: print(stack.pop()),
    "read": lambda: stack.append(Path(stack.pop()).read_text()),
    "write": lambda: Path(stack.pop()).write_text(stack.pop()),
    "parse": lambda: stack.append(parse(stack.pop())),
    "eval": lambda: eval(stack.pop()),
    "+": plus,
    "-": minus,
    "*": times,
    "/": div,
}


def eval(atoms: list[str]):
    global vocabulary
    global stack
    global quoted_atoms
    global quote_level

    for atom in atoms:
        if isinstance(atom, (bool, float)):
            if quote_level == 0:
                stack.append(atom)
            else:
                quoted_atoms.append(atom)
        elif atom.startswith('"') and atom.endswith('"'):
            if quote_level == 0:
                stack.append(atom[1:-1])
            else:
                quoted_atoms.append(atom)
        elif atom == "(":
            if quote_level > 0:
                quoted_atoms.append(atom)
            quote_level += 1
        elif atom == ")":
            if quote_level == 1:
                stack.append(quoted_atoms.copy())
                quoted_atoms.clear()
            elif quote_level > 1:
                quoted_atoms.append(atom)
            else:
                raise RuntimeError("encountered ) without matching (")
            quote_level -= 1
        else:  # symbols
            if quote_level == 0 and atom in vocabulary:
                if isinstance(vocabulary[atom], list):
                    eval(vocabulary[atom])
                elif callable(vocabulary[atom]):
                    vocabulary[atom]()
                else:
                    raise RuntimeError(
                        f"corrupt vocabulary contents {vocabulary[atom]}"
                    )
            elif quote_level > 0:
                quoted_atoms.append(atom)
            else:
                raise RuntimeError(f"unrecognised function {atom}")
