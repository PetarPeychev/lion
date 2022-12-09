import re
from lion.words import Number, String, Boolean, Symbol, Quote


def remove_comments(string):
    pattern = r"(\".*?\")|(#[^\r\n]*$)"
    # first group captures quoted strings (double or single)
    # second group captures comments (//single-line or /* multi-line */)
    regex = re.compile(pattern, re.MULTILINE | re.DOTALL)

    def _replacer(match):
        # if the 2nd group (capturing comments) is not None,
        # it means we have captured a non-quoted (real) comment string.
        if match.group(2) is not None:
            return ""  # so we will return empty to remove the comment
        else:  # otherwise, we will return the 1st group
            return match.group(1)  # captured quoted-string

    return regex.sub(_replacer, string)


def parse(code: str) -> Quote:
    no_comments = remove_comments(code)
    tokens = re.findall(r'(?:[^\s"]+|"(?:[^"])*")', no_comments)

    no_parens = []
    for t in tokens:
        if not (t.startswith('"') and t.endswith('"')):
            split_parens = re.split(r"(\(|\))", t)
            no_parens.extend([tok for tok in split_parens if tok != ""])
        else:
            no_parens.append(t)

    level = 0
    quoted = {}
    tokens_parsed = []
    for t in no_parens:
        if t == "(":
            level += 1
            quoted[level] = []
        elif t == ")":
            quote = Quote(quoted[level])
            level -= 1
            if level == 0:
                tokens_parsed.append(quote)
            elif level > 0:
                quoted[level].append(quote)
        elif t in ["true", "false"]:
            boolean = Boolean(t == "true")
            if level == 0:
                tokens_parsed.append(boolean)
            elif level > 0:
                quoted[level].append(boolean)
        elif is_number(t):
            number = Number(float(t))
            if level == 0:
                tokens_parsed.append(number)
            elif level > 0:
                quoted[level].append(number)
        elif t.startswith('"') and t.endswith('"'):
            string = String(t[1:-1])
            if level == 0:
                tokens_parsed.append(string)
            elif level > 0:
                quoted[level].append(string)
        else:
            symbol = Symbol(t)
            if level == 0:
                tokens_parsed.append(symbol)
            elif level > 0:
                quoted[level].append(symbol)

    return Quote(tokens_parsed)


def is_number(string: str):
    try:
        float(string)
        return True
    except ValueError:
        return False
