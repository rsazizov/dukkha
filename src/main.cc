#include <iostream>
#include <sysexits.h>

#include "virtual_machine.hh"

int main(int argc, char* argv[]) {
  VirtualMachine vm;

  Bytecode code;

  std::uint8_t pi = code.push_const(3.14);

  code.push_op(VirtualMachine::Constant16, 1);
  code.push_op(pi, 1);
  code.push_op(VirtualMachine::Negate, 0);

  std::cout << vm.execute(&code) << "\n";

  return EX_OK;
}
