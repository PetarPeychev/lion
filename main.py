from copy import deepcopy
from dataclasses import dataclass, field


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
        return "[ " + " ".join(map(str, self)) + " ]"

    def __repr__(self) -> str:
        return self.__str__()


type Value = Number | Symbol | List


@dataclass
class Environment:
    bindings: dict[str, Value] = field(default_factory=dict[str, Value])


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

    def evaluate(self, value: Value):
        match value:
            case Number():
                self.values.append(value)
            case Symbol():
                for env in reversed(self.environments):
                    if value in env.bindings:
                        self.evaluate(env.bindings[value])
                        return
                raise LionError(f"Error: undefined symbol '{value}'")
            case List():
                self.values.append(value)

    def save(self):
        self.saved_values = deepcopy(self.values)
        self.saved_environments = deepcopy(self.environments)

    def restore(self):
        self.values = self.saved_values
        self.environments = self.saved_environments


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
                elif token[0].isdigit() or token[0] == "-":
                    stack[-1].append(Number(float(token)))
                else:
                    stack[-1].append(Symbol(token))

            if len(stack) != 1:
                raise LionError("Error: unclosed parenthesis")

            for value in stack.pop():
                interpreter.evaluate(value)

            if interpreter.values:
                print(" ".join(map(str, interpreter.values)))

            if interpreter.environments:
                print(interpreter.environments)
        except LionError as ex:
            print(ex)
            interpreter.restore()
            continue


if __name__ == "__main__":
    main()
