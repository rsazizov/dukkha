#include <iostream>
#include <sysexits.h>

#include "virtual_machine.hh"
#include "compiler.hh"

int main(int argc, char* argv[]) {
  Bytecode code;

  Compiler compiler;
  compiler.from_file("examples/expression.du", code);

  VirtualMachine vm;
  Value result = vm.execute(&code);

  std::cout << "Result: " << result << "\n";

  return EX_OK;
}
