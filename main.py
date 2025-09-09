from __future__ import annotations

import atexit
import os
import readline
import sys
from copy import deepcopy
from dataclasses import dataclass, field
from typing import Any, Callable, cast


class LionError(Exception):
    pass


class Number(float):
    def __str__(self) -> str:
        return str(float(self)).removesuffix(".0")

    def __repr__(self) -> str:
        return self.__str__()


class Symbol(str):
    def __repr__(self) -> str:
        return self.__str__()


class String:
    def __init__(self, value: str):
        self.value = value

    def __str__(self) -> str:
        return f'"{self.value}"'

    def __repr__(self) -> str:
        return self.__str__()


class List(list["Value"]):
    def __str__(self) -> str:
        return "[" + " ".join(map(str, self)) + "]"

    def __repr__(self) -> str:
        return self.__str__()


type Value = Number | Symbol | String | List


@dataclass
class Environment:
    bindings: dict[str, Value] = field(default_factory=dict[str, Value])


def requires_args(
    count: int,
) -> Callable[[Callable[[Interpreter], None]], Callable[[Interpreter], None]]:
    def decorator(func: Callable[[Interpreter], None]) -> Callable[[Interpreter], None]:
        def wrapper(self: Interpreter) -> Any:
            if len(self.values) < count:
                func_name = func.__name__.replace("builtin_", "", 1)
                raise LionError(f"Error: not enough arguments to '{func_name}'")
            return func(self)

        return wrapper

    return decorator


@dataclass
class Interpreter:
    values: list[Value] = field(default_factory=list[Value])
    environments: list[Environment] = field(default_factory=list[Environment])
    saved_values: list[Value] = field(default_factory=list[Value])
    saved_environments: list[Environment] = field(default_factory=list[Environment])

    def __post_init__(self):
        base_env = Environment()
        base_env.bindings["pi"] = Number(3.14)
        self.environments.append(base_env)

    def interpret(self, value: Value):
        match value:
            case Number():
                self.values.append(value)
            case Symbol():
                for env in reversed(self.environments):
                    if value in env.bindings:
                        self.values.append(env.bindings[value])
                        self.builtin_eval()
                        return
                match value:
                    case "eval":
                        self.builtin_eval()
                    case "load":
                        self.builtin_load()
                    case "def":
                        self.builtin_def()
                    case "defm":
                        self.builtin_defm()
                    case "print":
                        self.builtin_print()
                    case "print_values":
                        print("|", " | ".join(map(str, self.values)), "|")
                    case "print_environments":
                        bindings: dict[str, Value] = {}
                        for env in self.environments:
                            bindings.update(env.bindings)
                        for key, value in bindings.items():
                            print(key, "=", value)
                    case "exit":
                        self.builtin_exit()
                    case "+":
                        self.builtin_add()
                    case "-":
                        self.builtin_sub()
                    case "*":
                        self.builtin_mul()
                    case "/":
                        self.builtin_div()
                    case _:
                        raise LionError(f"Error: undefined symbol '{value}'")
            case String():
                self.values.append(value)
            case List():
                self.values.append(value)

    def save(self):
        self.saved_values = deepcopy(self.values)
        self.saved_environments = deepcopy(self.environments)

    def restore(self):
        self.values = self.saved_values
        self.environments = self.saved_environments

    def repl(self):
        history_file = os.path.expanduser("~/.lion_history")
        readline.set_history_length(1000)

        try:
            readline.read_history_file(history_file)
        except FileNotFoundError:
            pass

        def save_history():
            readline.write_history_file(history_file)

        atexit.register(save_history)

        while True:
            self.save()
            try:
                self.interpret_code(input("> "))
            except LionError as ex:
                print(ex)
                self.restore()
                continue
            if self.values:
                print("|", " | ".join(map(str, self.values)), "|")

    def load(self, filename: str):
        with open(filename, "r") as f:
            self.interpret_code(f.read())

    def interpret_code(self, code: str):
        tokens: list[str] = []

        i = 0
        while i < len(code):
            char = code[i]
            if char.isspace():
                i += 1
            elif char in "[]":
                tokens.append(char)
                i += 1
            elif char == '"':
                start = i
                i += 1
                while i < len(code):
                    if code[i] == '"' and (i == 0 or code[i - 1] != "\\"):
                        i += 1
                        break
                    i += 1
                else:
                    raise LionError("Error: unterminated string literal")
                tokens.append(code[start:i])
            elif char == "(":
                start = i
                i += 1
                while i < len(code):
                    if code[i] == ")":
                        i += 1
                        break
                    i += 1
                else:
                    raise LionError("Error: unterminated comment")
            else:
                start = i
                while i < len(code) and not code[i].isspace() and code[i] not in '[]"':
                    i += 1
                tokens.append(code[start:i])

        stack = [List([])]
        for token in tokens:
            if token == "[":
                stack.append(List([]))
            elif token == "]":
                if len(stack) == 1:
                    raise LionError("Error: extra closing parenthesis")
                closed = stack.pop()
                stack[-1].append(List(closed))
            elif token[0].isdigit() or (
                token[0] == "-" and len(token) > 1 and token[1].isdigit()
            ):
                stack[-1].append(Number(float(token)))
            elif token.startswith('"') and token.endswith('"'):
                string_content = token[1:-1].replace('\\"', '"').replace("\\\\", "\\")
                stack[-1].append(String(string_content))
            else:
                stack[-1].append(Symbol(token))

        if len(stack) != 1:
            raise LionError("Error: unclosed parenthesis")

        for value in stack.pop():
            self.interpret(value)

    @requires_args(1)
    def builtin_eval(self):
        arg = self.values.pop()
        match arg:
            case List():
                self.environments.append(Environment())
                for value in arg:
                    self.interpret(value)
                self.environments.pop()
            case _:
                self.interpret(arg)

    @requires_args(1)
    def builtin_load(self):
        arg = self.values.pop()
        if isinstance(arg, String):
            self.load(arg.value)
        else:
            raise LionError("Error: first argument to 'load' must be a string")

    @requires_args(2)
    def builtin_def(self):
        arg2 = self.values.pop()
        arg1 = self.values.pop()
        if isinstance(arg1, List) and len(arg1) == 1 and isinstance(arg1[0], Symbol):
            self.environments[-1].bindings[cast(Symbol, arg1[0])] = arg2
        else:
            raise LionError(
                "Error: first argument to 'def' must be a symbol wrapped in a list"
            )

    @requires_args(1)
    def builtin_defm(self):
        arg = self.values.pop()
        if not isinstance(arg, List):
            raise LionError("Error: last argument to 'defm' must be a list")
        if not (all(isinstance(x, Symbol) for x in arg)):
            raise LionError("Error: last argument to 'defm' must be a list of symbols")
        if not len(self.values) >= len(arg):
            raise LionError(
                "Error: not enough arguments to 'defm', expected " + str(len(arg))
            )
        for symbol in reversed(arg):
            self.environments[-1].bindings[cast(Symbol, symbol)] = self.values.pop()

    @requires_args(1)
    def builtin_print(self):
        value = self.values.pop()
        if isinstance(value, String):
            print(value.value)
        else:
            print(value)

    @requires_args(0)
    def builtin_exit(self):
        raise SystemExit()

    @requires_args(2)
    def builtin_add(self):
        arg2 = self.values.pop()
        arg1 = self.values.pop()
        if isinstance(arg1, Number) and isinstance(arg2, Number):
            self.values.append(Number(arg1 + arg2))
        elif isinstance(arg1, Symbol) and isinstance(arg2, Symbol):
            self.values.append(Symbol(arg1 + arg2))
        elif isinstance(arg1, String) and isinstance(arg2, String):
            self.values.append(String(arg1.value + arg2.value))
        elif isinstance(arg1, List) and isinstance(arg2, List):
            self.values.append(List(arg1 + arg2))
        else:
            raise LionError("Error: cannot add values of different types")

    @requires_args(2)
    def builtin_sub(self):
        arg2 = self.values.pop()
        arg1 = self.values.pop()
        if isinstance(arg1, Number) and isinstance(arg2, Number):
            self.values.append(Number(arg1 - arg2))
        else:
            raise LionError("Error: both arguments to '-' must be numbers")

    @requires_args(2)
    def builtin_mul(self):
        arg2 = self.values.pop()
        arg1 = self.values.pop()
        if isinstance(arg1, Number) and isinstance(arg2, Number):
            self.values.append(Number(arg1 * arg2))
        else:
            raise LionError("Error: both arguments to '*' must be numbers")

    @requires_args(2)
    def builtin_div(self):
        arg2 = self.values.pop()
        arg1 = self.values.pop()
        if isinstance(arg1, Number) and isinstance(arg2, Number):
            self.values.append(Number(arg1 / arg2))
        else:
            raise LionError("Error: both arguments to '/' must be numbers")


def main():
    interpreter = Interpreter()

    if len(sys.argv) == 1:
        interpreter.repl()
    elif len(sys.argv) == 2:
        interpreter.load(sys.argv[1])
    else:
        print("Usage: lion [file]")
        sys.exit(1)


if __name__ == "__main__":
    main()
