# Dukkha

Dukkha is a toy programming language and a corresponding implementation in C++ (more like C with classes).

## Features

- [x] Expressions
- [x] Global variables
- [x] Local variables
- [x] if-else if-else
- [x] while-else
- [ ] Python-ish data model
- [ ] Symbols
- [ ] Closure
- [ ] Garbage collection
- [ ] Modules

## Virtual Machine

Compiled programs have `.rodata` (`Bytecode::m_consts`) and `.text` (`Bytecode::m_code`)
sections for constants and instructions respectively. Dukkha programs run on a stack based virtual machine. Below you can find the instruction set for the vm. S refers to the stack and pop(S)'s
should be read from right to left.  Also, `$A` refers to a value at address A defined in `.rodata` section of a compiled program
and `%A` referes to a value on a stack with offset A (from the bottom of the stack);

| Instruction | Operands | Description                                                       |
|-------------|----------|-------------------------------------------------------------------|
| Return      | None     | Halt                                                              |
| Constant16  | A16      | Load a constant at $A                                             |
| Pop         | None     | pop(S)                                                            |
| Negate      | None     | Calculate -pop(S) and push it on top of the stack                 |
| Add         | None     | Calculate pop(S) + pop(S) and push it on top of the stack         |
| Subtract    | None     | Calculate pop(S) - pop(S) and push it on top of the stack         |
| Multiply    | None     | Calculate pop(S) * pop(S) and push it on top of the stack         |
| Divide      | None     | Calculate pop(S) / pop(S) and push it on top of the stack         |
| Exp         | None     | Calculate pop(S) ^ pop(S) and push it on top of the stack         |
| Not         | None     | Calculate logical ~pop(S) and push it on top of the stack         |
| Greater     | None     | Calculate logical pop(S) > pop(S) and push it on top of the stack |
| Less        | None     | Calculate logical pop(S) < pop(S) and push it on top of the stack |
| StoreGlobal | A16      | Store value pop(S) as a global named $A                           |
| LoadGlobal  | A16      | Load a global value named $A and push it on top of the stack      |
| StoreLocal  | A16      | Store pop(S) at %A                                                |
| LoadLocal   | A16      | Load a value %A and push it on top of the stack                   |
| JumpIfFalse | A64      | Set instruction pointer to A if pop(S) == false                   |
| Jump        | A64      | Set instruction pointer to A                                      |

Here is an example of a program and its compiled bytecode:

```javascript
let answer = '42';
let message = 'The answer is';

print(message + ' ' + answer);
```

```
.rodata:
$00000 answer
$00001 42
$00002 message
$00003 The answer is
$00004 message
$00005
$00006 answer
.text:
$00000:001 alcg $0
$00002:001 push $1
$00004:001 stg $0
$00006:002 alcg $2
$00008:002 push $3
$00010:002 stg $2
$00012:004 loadg $4
$00014:004 push $5
$00016:004 add
$00017:004 loadg $6
$00019:004 add
$00020:004 cout
$00021:004 ret
```

## Grammar

Below is BNF representation of the language (for now, I'll add more rules as I go).

```
<program> := <declaration>+ EOF;

<declaration> := <variable_declaration> |
                 <block_declaration> |
                 <statement>;

<block_declaration> := "{" <declaration>+ "}";
<variable_declaration> := "let" <identifier> ("=" <expression>)? ";";

<statement> := <print_statement> |
               <variable_assignment> |
               <if_statement> |
               <while_statement> |
               <expression> ";";

<variable_assignment> := <identifier> "=" <expression> ";";

<print_statement> := "print" <expression> ";";
<if_statement> := "if" <expression> <block_declaration>
                  ("else" "if" <expression> <block_declaration>)*
                  ("else" <block_declaration>)?;
<while_statement> := "while" <expression> <block_declaration> ("else" <block_declaration>)?;

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
