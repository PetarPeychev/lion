from __future__ import annotations

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


class List(list["Value"]):
    def __str__(self) -> str:
        return "[" + " ".join(map(str, self)) + "]"

    def __repr__(self) -> str:
        return self.__str__()


type Value = Number | Symbol | List


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
                    case "def":
                        self.builtin_def()
                    case "print":
                        self.builtin_print()
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
            case List():
                self.values.append(value)

    def save(self):
        self.saved_values = deepcopy(self.values)
        self.saved_environments = deepcopy(self.environments)

    def restore(self):
        self.values = self.saved_values
        self.environments = self.saved_environments

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
    def builtin_print(self):
        print(self.values.pop())

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
    while True:
        print("> ", end="")
        interpreter.save()
        try:
            tokens = list(
                filter(None, input().replace("[", " [ ").replace("]", " ] ").split())
            )
            stack = [List([])]
            for token in tokens:
                if token == "[":
                    stack.append(List([]))
                elif token == "]":
                    if len(stack) == 1:
                        raise LionError("Error: extra closing parenthesis")
                    closed = stack.pop()
                    stack[-1].append(List(closed))
                elif (
                    token[0].isdigit()
                    or token[0] == "-"
                    and len(token) > 1
                    and token[1].isdigit()
                ):
                    stack[-1].append(Number(float(token)))
                else:
                    stack[-1].append(Symbol(token))

            if len(stack) != 1:
                raise LionError("Error: unclosed parenthesis")

            for value in stack.pop():
                interpreter.interpret(value)

            if interpreter.values:
                print("|", " | ".join(map(str, interpreter.values)), "|")

            # if interpreter.environments:
            #     print(interpreter.environments)
        except LionError as ex:
            print(ex)
            interpreter.restore()
            continue


if __name__ == "__main__":
    main()
