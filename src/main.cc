#include <iostream>
#include <sysexits.h>

#include "virtual_machine.hh"
#include "compiler.hh"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: dukkha <file.du>\n";
    return EX_USAGE;
  }

  Bytecode code;

  Compiler compiler;
  bool compiled = compiler.from_file(argv[1], code);

  if (!compiled) return EX_SOFTWARE;

  /* code.dump_data(); */
  /* code.dump_text(); */

  VirtualMachine vm;
  vm.execute(&code);

  return EX_OK;
}
