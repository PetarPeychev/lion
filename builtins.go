package main

import "errors"

var Bindings = map[string]func([]Value, Environment) ([]Value, Environment, error){
	"parse": builtinParse,
	"apply": builtinApply,
	"bind":  builtinBind,
	"wrap":  builtinWrap,
	"+":     builtinAdd,
	"-":     builtinSub,
	"*":     builtinMul,
	"/":     builtinDiv,
}

func builtinParse(stack []Value, env Environment) ([]Value, Environment, error) {
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
}

func builtinApply(stack []Value, env Environment) ([]Value, Environment, error) {
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
}

func builtinBind(stack []Value, env Environment) ([]Value, Environment, error) {
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
			symbolQuote := arg2.(Quote).Values[0]
			var name string
			switch symbolQuote := symbolQuote.(type) {
			case Symbol:
				name = symbolQuote.Value
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
}

func builtinWrap(stack []Value, env Environment) ([]Value, Environment, error) {
	if len(stack) < 1 {
		return nil, env, errors.New("'wrap' expects at least one argument")
	}
	stack[len(stack)-1] = Quote{Values: []Value{stack[len(stack)-1]}}
	return stack, env, nil
}

func builtinAdd(stack []Value, env Environment) ([]Value, Environment, error) {
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
}

func builtinSub(stack []Value, env Environment) ([]Value, Environment, error) {
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
}

func builtinMul(stack []Value, env Environment) ([]Value, Environment, error) {
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
}

func builtinDiv(stack []Value, env Environment) ([]Value, Environment, error) {
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
			if arg2.(Number).Value == 0 {
				return nil, env, errors.New("division by zero")
			}
			stack = append(stack, Number{Value: arg1.(Number).Value / arg2.(Number).Value})
			return stack, env, nil
		default:
			return nil, env, errors.New("'/' expects a number as its second argument")
		}
	default:
		return nil, env, errors.New("'/' expects a number as its first argument")
	}
}
