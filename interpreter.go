package main

import (
	"errors"
	"maps"
	"strconv"
	"strings"
)

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
			var err error
			quote, ok := env.Get(v.Value)
			if !ok {
				// var binding func([]Value, Environment) ([]Value, Environment, error)
				// if binding, ok = Bindings[v.Value]; ok {
				// 	stack, env, err = binding(stack, env)
				// 	if err != nil {
				// 		return nil, env, err
				// 	}
				// 	return stack, env, nil
				// }
				return nil, Environment{}, errors.New("undefined symbol '" + v.Value + "'")
			}
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
