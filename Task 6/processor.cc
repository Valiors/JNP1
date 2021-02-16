#include "ooasm_instruction.h"

void detail::Processor::run(const Program &program, detail::Memory &memory) {
    memory.clear();

    for (const auto &instruction : program) {
        instruction->prepare(memory);
    }

    for (const auto &instruction : program) {
        instruction->perform(memory, *this);
    }
}