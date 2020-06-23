# Dukkha

Dukkha is a toy programming language and a corresponding implementation in C++ (more like C with classes).

## Features

Source code is compiled into bytecode that is executed by a virtual machine.

### Virtual Machine

Each instruction is 1 byte long. However, some instructions may have different variants depending on the size of the operand (e.g `Constant16`/`Constant32`). A table describing each instruction is presented below (S refers to the stack).

| Instruction | Operands | Description                                                       |
|-------------|----------|-------------------------------------------------------------------|
| Return      | None     | Halt                                                              |
| Constant16  | A        | Load a constant at $A                                             |
| Constant32  | AB       | Load a constant at $AB                                            |
| Pop         |          | pop(S)                                                            |
| Negate      | None     | Calculate -pop(S) and push it on top of the stack                 |
| Add         | None     | Calculate pop(S) + pop(S) and push it on top of the stack         |
| Subtract    | None     | Calculate pop(S) - pop(S) and push it on top of the stack         |
| Multiply    | None     | Calculate pop(S) * pop(S) and push it on top of the stack         |
| Divide      | None     | Calculate pop(S) / pop(S) and push it on top of the stack         |
| Exp         | None     | Calculate pop(S) ^ pop(S) and push it on top of the stack         |
| Not         | None     | Calculate logical ~pop(S) and push it on top of the stack         |
| Greater     | None     | Calculate logical pop(S) > pop(S) and push it on top of the stack |
| Less        | None     | Calculate logical pop(S) < pop(S) and push it on top of the stack |
| StoreGlobal | A        | Store value pop(S) as a global named $A                           |
| LoadGlobal  | A        | Load a global value named $A and push it on top of the stack      |
| StoreLocal  | A        | Store pop(S) at %A                                                |
| LoadLocal   | A        | Load a value %A and push it on top of the stack                   |

### Grammar

Below is BNF representation of the language (for now, I'll add more rules as I go).

```
<program> := <declaration>+ EOF;

<declaration> := <variable_declaration> |
                 "{" <declaration>+ "}" |
                 <statement>;

<variable_declaration> := "let" <identifier> ("=" <expression>)? ";";

<statement> := <print> | <expression> ";";
<print> := "print" <expression> ";";

<expression> := <or>;

<or> := <and> ("or" <and>)+;
<and> := <not> ("and" <not>)+;
<not> := "not" <comparison> | <comparison>;
<comparison> := <addition> (<comparison_op> <addition>)+;

<addition> := <multiplication> (("+" | "-") <multiplication>)+;
<multiplication> := <unary> (("*" | "/") <unary>)+;
<unary> := "-" <exp> | <exp>;
<exp> := <arbitrary> ("**" <arbitrary>)+;

<arbitrary> := <number> | "(" <expression> ")" | "true | "false" | <identifier>;

<comparison_op> := "==" | "!=" | ">=" | "<=" | ">" | "<";
```

## Usage

```
dukkha <file.du>
```
