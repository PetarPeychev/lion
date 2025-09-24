package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/peterh/liner"
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

func Eval(stack []Value, quote Quote) ([]Value, error) {
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
			stack = append(stack, Symbol{Value: v.Value})
		}
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
			new_stack, err = Eval(stack, quote)
			if err != nil {
				fmt.Println(err)
				continue
			}
			stack = new_stack

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
