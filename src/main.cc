#include <iostream>
#include <sysexits.h>

#include "virtual_machine.hh"
#include "compiler.hh"

int main(int argc, char* argv[]) {
  Bytecode code;

  Compiler compiler;
  bool compiled = compiler.from_file("examples/expression.du", code);

  if (!compiled) return EX_SOFTWARE;

  code.dump_data();
  code.dump_text();

  VirtualMachine vm;
  Value result = vm.execute(&code);

  std::cout << "Result: " << result << "\n";

  return EX_OK;
}
