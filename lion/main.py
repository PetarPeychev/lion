import sys
import readline
from copy import deepcopy
from pathlib import Path

from lion.parser import parse
from lion.interpreter import Interpreter


def main() -> None:
    args = sys.argv[1:]

    interpreter = Interpreter()
    vocabulary = {}
    interpret_file(interpreter, "core/core.lion", vocabulary)

    if len(args) == 0:
        repl(interpreter, vocabulary)
    elif len(args) == 1:
        interpret_file(interpreter, args[0], vocabulary)


def repl(interpreter: Interpreter, vocabulary: dict) -> None:
    print("Lion 1.0.0:")

    previous_stack = []
    previous_vocabulary = {}
    while True:
        code = input("> ")
        try:
            quote = parse(code)
            previous_stack = deepcopy(interpreter.stack)
            previous_vocabulary = deepcopy(vocabulary)
            interpreter.evaluate(quote, vocabulary)
        except Exception as e:
            interpreter.stack = previous_stack
            vocabulary = previous_vocabulary
            print(f"Error: {e}")
        print(interpreter.stack)


def interpret_file(interpreter: Interpreter, filename: str, vocabulary: dict) -> None:
    try:
        code = Path(filename).read_text()
        quote = parse(code)
        interpreter.evaluate(quote, vocabulary)
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
