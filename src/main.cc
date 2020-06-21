#include <iostream>
#include <sysexits.h>

#include "virtual_machine.hh"
#include "compiler.hh"

int main(int argc, char* argv[]) {
  Bytecode code;

  Compiler compiler;
  bool compiled = compiler.from_file("examples/statement.du", code);

  if (!compiled) return EX_SOFTWARE;

  code.dump_data();
  code.dump_text();

  VirtualMachine vm;
  vm.execute(&code);

  return EX_OK;
}
