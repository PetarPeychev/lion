package main

import (
	"fmt"
	"io"
	"log"
	"maps"
	"os"
	"path/filepath"
	"strings"

	"github.com/peterh/liner"
)

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
