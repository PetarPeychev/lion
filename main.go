package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"maps"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/peterh/liner"
)

// TODO:
// - [ ] Add builtin functions.
// - [ ] Add global environment and 'def/undef' builtin.
// - [ ] Add file reading to import.
// - [ ] Add lib.lion with some basic functions.

type Value interface {
	String() string
}

type Number struct {
	Value float64
}

func (n Number) String() string {
	return strconv.FormatFloat(n.Value, 'f', -1, 64)
}

type Symbol struct {
	Value string
}

func (s Symbol) String() string {
	return s.Value
}

type String struct {
	Value string
}

func (s String) String() string {
	return "\"" + s.Value + "\""
}

type Boolean struct {
	Value bool
}

func (b Boolean) String() string {
	if b.Value {
		return "true"
	}
	return "false"
}

type Quote struct {
	Values []Value
}

func (q Quote) String() string {
	var sb strings.Builder
	sb.WriteString("[")
	for i, v := range q.Values {
		if i > 0 {
			sb.WriteString(" ")
		}
		sb.WriteString(v.String())
	}
	sb.WriteString("]")
	return sb.String()
}

type Environment struct {
	Bindings map[string]Quote
	Parent   *Environment
}

func (e *Environment) Get(name string) (Quote, bool) {
	if q, ok := e.Bindings[name]; ok {
		return q, true
	}
	if e.Parent != nil {
		return e.Parent.Get(name)
	}
	return Quote{}, false
}

func (e *Environment) Set(name string, quote Quote) {
	e.Bindings[name] = quote
}

func (e *Environment) String() string {
	quotes := make(map[string]Quote)

	var envs []*Environment
	current := e
	for current != nil {
		envs = append(envs, current)
		current = current.Parent
	}

	for i := len(envs) - 1; i >= 0; i-- {
		maps.Copy(quotes, envs[i].Bindings)
	}

	var sb strings.Builder
	sb.WriteString("{")

	first := true
	for key, value := range quotes {
		if !first {
			sb.WriteString(", ")
		}
		sb.WriteString(key)
		sb.WriteString(": ")
		sb.WriteString(value.String())
		first = false
	}

	sb.WriteString("}")
	return sb.String()
}

func Parse(code string) (quote Quote, err error) {
	var result []Value
	i := 0

	for i < len(code) {
		for i < len(code) && (code[i] == ' ' || code[i] == '\t' || code[i] == '\n' || code[i] == '\r') {
			i++
		}

		if i >= len(code) {
			break
		}

		if code[i] == '(' {
			depth := 1
			i++
			for i < len(code) && depth > 0 {
				switch code[i] {
				case '(':
					depth++
				case ')':
					depth--
				}
				i++
			}
			if depth > 0 {
				err = errors.New("unterminated quoted expression: missing closing parenthesis")
				return
			}
			continue
		}

		if code[i] == ')' {
			err = errors.New("unexpected closing parenthesis ')' without matching opening parenthesis")
			return
		}

		if code[i] == '"' {
			i++
			start := i
			for i < len(code) && code[i] != '"' {
				if code[i] == '\\' {
					if i+1 >= len(code) {
						err = errors.New("unterminated string: escape sequence at end of input")
						return
					}
					i += 2
				} else {
					i++
				}
			}
			if i >= len(code) {
				err = errors.New("unterminated string: missing closing quote")
				return
			}
			result = append(result, String{Value: code[start:i]})
			i++
			continue
		}

		if code[i] == '[' {
			i++
			start := i
			depth := 1
			for i < len(code) && depth > 0 {
				switch code[i] {
				case '[':
					depth++
				case ']':
					depth--
				}
				i++
			}
			if depth > 0 {
				err = errors.New("unterminated quoted expression: missing closing bracket ']'")
				return
			}
			var nestedQuote Quote
			nestedQuote, err = Parse(code[start : i-1])
			if err != nil {
				return
			}
			result = append(result, nestedQuote)
			continue
		}

		if code[i] == ']' {
			err = errors.New("unexpected closing bracket ']' without matching opening bracket")
			return
		}

		start := i
		for i < len(code) && code[i] != ' ' && code[i] != '\t' && code[i] != '\n' && code[i] != '\r' &&
			code[i] != '(' && code[i] != ')' && code[i] != '[' && code[i] != ']' && code[i] != '"' {
			i++
		}

		token := code[start:i]
		if token == "" {
			continue
		}

		if num, err := strconv.ParseFloat(token, 64); err == nil {
			result = append(result, Number{Value: num})
			continue
		}

		if token == "true" {
			result = append(result, Boolean{Value: true})
			continue
		}
		if token == "false" {
			result = append(result, Boolean{Value: false})
			continue
		}

		result = append(result, Symbol{Value: token})
	}

	return Quote{Values: result}, nil
}

// Apply evaluates a quote in the current environment.
func Apply(stack []Value, env Environment, quote Quote) ([]Value, Environment, error) {
	for _, v := range quote.Values {
		switch v := v.(type) {
		case Quote:
			stack = append(stack, Quote{Values: v.Values})
		case Number:
			stack = append(stack, Number{Value: v.Value})
		case Boolean:
			stack = append(stack, Boolean{Value: v.Value})
		case String:
			stack = append(stack, String{Value: v.Value})
		case Symbol:
			quote, ok := env.Get(v.Value)
			if !ok {
				switch v.Value {
				case "parse":
					if len(stack) < 1 {
						return nil, env, errors.New("'parse' expects at least one argument")
					}
					arg := stack[len(stack)-1]
					switch arg := arg.(type) {
					case String:
						stack = stack[:len(stack)-1]
						quote, err := Parse(arg.Value)
						if err != nil {
							return nil, env, err
						}
						stack = append(stack, quote)
						return stack, env, nil
					default:
						return nil, env, errors.New("'parse' expects a string as its first argument")
					}
				case "apply":
					if len(stack) < 1 {
						return nil, env, errors.New("'apply' expects at least one argument")
					}
					arg := stack[len(stack)-1]
					switch arg := arg.(type) {
					case Quote:
						var err error
						stack = stack[:len(stack)-1]
						stack, env, err = Apply(stack, env, arg)
						if err != nil {
							return nil, env, err
						}
						return stack, env, nil
					default:
						return nil, env, errors.New("'apply' expects a quoted list as its first argument")
					}
				case "bind":
					if len(stack) < 2 {
						return nil, env, errors.New("'bind' expects two arguments")
					}
					arg1 := stack[len(stack)-2]
					arg2 := stack[len(stack)-1]
					stack = stack[:len(stack)-2]

					switch arg1 := arg1.(type) {
					case Quote:
						switch arg2.(type) {
						case Quote:
							if len(arg2.(Quote).Values) != 1 {
								return nil, env, errors.New("'bind' expects a quoted list with one element")
							}
							symbol_quote := arg2.(Quote).Values[0]
							var name string
							switch symbol_quote := symbol_quote.(type) {
							case Symbol:
								name = symbol_quote.Value
							default:
								return nil, env, errors.New("'bind' expects a quoted symbol as its second argument")
							}
							env.Set(name, arg1)
							return stack, env, nil
						default:
							return nil, env, errors.New("'bind' expects a quoted list as its second argument")
						}
					default:
						return nil, env, errors.New("'bind' expects a quoted list as its first argument")
					}
				case "wrap":
					if len(stack) < 1 {
						return nil, env, errors.New("'wrap' expects at least one argument")
					}
					stack[len(stack)-1] = Quote{Values: []Value{stack[len(stack)-1]}}
					return stack, env, nil
				case "+":
					if len(stack) < 2 {
						return nil, env, errors.New("'+' expects two arguments")
					}
					arg1 := stack[len(stack)-2]
					arg2 := stack[len(stack)-1]
					stack = stack[:len(stack)-2]
					switch arg1.(type) {
					case Number:
						switch arg2.(type) {
						case Number:
							stack = append(stack, Number{Value: arg1.(Number).Value + arg2.(Number).Value})
							return stack, env, nil
						default:
							return nil, env, errors.New("'+' expects a number as its second argument")
						}
					default:
						return nil, env, errors.New("'+' expects a number as its first argument")
					}
				case "-":
					if len(stack) < 2 {
						return nil, env, errors.New("'-' expects two arguments")
					}
					arg1 := stack[len(stack)-2]
					arg2 := stack[len(stack)-1]
					stack = stack[:len(stack)-2]
					switch arg1.(type) {
					case Number:
						switch arg2.(type) {
						case Number:
							stack = append(stack, Number{Value: arg1.(Number).Value - arg2.(Number).Value})
							return stack, env, nil
						default:
							return nil, env, errors.New("'-' expects a number as its second argument")
						}
					default:
						return nil, env, errors.New("'-' expects a number as its first argument")
					}
				case "*":
					if len(stack) < 2 {
						return nil, env, errors.New("'*' expects two arguments")
					}
					arg1 := stack[len(stack)-2]
					arg2 := stack[len(stack)-1]
					stack = stack[:len(stack)-2]
					switch arg1.(type) {
					case Number:
						switch arg2.(type) {
						case Number:
							stack = append(stack, Number{Value: arg1.(Number).Value * arg2.(Number).Value})
							return stack, env, nil
						default:
							return nil, env, errors.New("'*' expects a number as its second argument")
						}
					default:
						return nil, env, errors.New("'*' expects a number as its first argument")
					}
				case "/":
					if len(stack) < 2 {
						return nil, env, errors.New("'/' expects two arguments")
					}
					arg1 := stack[len(stack)-2]
					arg2 := stack[len(stack)-1]
					stack = stack[:len(stack)-2]
					switch arg1.(type) {
					case Number:
						switch arg2.(type) {
						case Number:
							stack = append(stack, Number{Value: arg1.(Number).Value / arg2.(Number).Value})
							return stack, env, nil
						default:
							return nil, env, errors.New("'/' expects a number as its second argument")
						}
					default:
						return nil, env, errors.New("'/' expects a number as its first argument")
					}
				}
				return nil, Environment{}, errors.New("undefined symbol '" + v.Value + "'")
			}
			var err error
			stack, env, err = Apply(stack, env, quote)
			if err != nil {
				return nil, env, err
			}
		}
	}
	return stack, env, nil
}

// Call evaluates a quote in a new temporary child environment.
func Call(stack []Value, env Environment, quote Quote) ([]Value, error) {
	new_env := Environment{Parent: &env}
	var err error
	stack, _, err = Apply(stack, new_env, quote)
	if err != nil {
		return nil, err
	}
	return stack, nil
}

func main() {
	line := liner.NewLiner()
	defer line.Close()

	line.SetCtrlCAborts(true)

	historyFile := filepath.Join(os.TempDir(), ".liner_example_history")
	if f, err := os.Open(historyFile); err == nil {
		line.ReadHistory(f)
		f.Close()
	}

	var stack []Value
	env := Environment{make(map[string]Quote), nil}

	env.Bindings["pi"] = Quote{Values: []Value{Number{Value: 3.14}}}
	env.Bindings["ints"] = Quote{Values: []Value{Quote{Values: []Value{Number{Value: 1}, Number{Value: 2}, Number{Value: 3}}}}}

	for {
		if input, err := line.Prompt("> "); err == nil {
			input = strings.TrimSpace(input)

			if input == "quit" {
				break
			}

			line.AppendHistory(input)

			var quote Quote
			quote, err = Parse(input)
			if err != nil {
				fmt.Println(err)
				continue
			}

			var new_stack []Value
			var new_env Environment
			new_stack, new_env, err = Apply(stack, Environment{maps.Clone(env.Bindings), env.Parent}, quote)
			if err != nil {
				fmt.Println(err)
				continue
			}
			stack = new_stack
			env = new_env

			fmt.Println(stack)
		} else if err == liner.ErrPromptAborted {
			break
		} else if err == io.EOF {
			break
		} else {
			log.Print("Error reading line: ", err)
			break
		}
	}

	if f, err := os.Create(historyFile); err != nil {
		log.Print("Error writing history file: ", err)
	} else {
		line.WriteHistory(f)
		f.Close()
	}
}
