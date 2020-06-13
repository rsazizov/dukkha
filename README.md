# Dukkha

Dukkha is a toy programming language and a corresponding implementation in C++.

## Features

Source code is compiled into bytecode that is executed by a virtual machine.

### Virtual Machine

Each instruction is 1 byte long. However, some instructions may have different variants depending on the size of the operand (e.g `Constant16`/`Constant32`). A table describing each instruction is presented below (S refers to the stack).

| Instruction | Operands | Description                                               |
|-------------|----------|-----------------------------------------------------------|
| Return      | None     | Halt                                                      |
| Constant16  | A        | Load a constant at address A                              |
| Constant32  | AB       | Load a constant at address AB                             |
| Negate      | None     | Calculate -pop(S) and push it on top of the stack         |
| Add         | None     | Calculate pop(S) + pop(S) and push it on top of the stack |
| Subtract    | None     | Calculate pop(S) - pop(S) and push it on top of the stack |
| Multiply    | None     | Calculate pop(S) * pop(S) and push it on top of the stack |
| Divide      | None     | Calculate pop(S)/pop(S) and push it on top of the stack   |

### Grammar

Below is BNF representation of the language (for now, I'll add more rules as I go).

```
<expression> := <number>
              | "(" <expression> ")"
              | <number> <binary> <number>
              | <unary> <expression>;

<binary> := "+";
<unary> := "-";
```

## Usage

```
dukkha <file.du>
```
