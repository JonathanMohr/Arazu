#include "instruction.hpp"

Assembler::Instruction::Instruction(uint64_t l, uint64_t c, StringPool::String f)
    : line(l), column(c), file(f)
{

}
