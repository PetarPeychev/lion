package main

import (
	"fmt"
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

func Parse(code string) Quote {
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
			continue
		}

		if code[i] == '"' {
			i++
			start := i
			for i < len(code) && code[i] != '"' {
				if code[i] == '\\' {
					i += 2
				} else {
					i++
				}
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
			nestedQuote := Parse(code[start : i-1])
			result = append(result, nestedQuote)
			continue
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

	return Quote{Values: result}
}

func main() {
	code := "(this is a comment) true false [1 2 3 [4 5 6]] \"hello world\" 69.420"
	quote := Parse(code)
	fmt.Println(quote)
}
