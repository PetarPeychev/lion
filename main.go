package main

import (
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

func main() {
	println(Quote{[]Value{
		Number{3.14},
		String{"hello"},
		Quote{[]Value{Number{1}, Number{2}, Number{3}}},
		Boolean{true},
		Symbol{"foo"},
		Boolean{false},
	}}.String())
}
