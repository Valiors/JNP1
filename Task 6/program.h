#ifndef JNP1_PROGRAM_H
#define JNP1_PROGRAM_H

#include <vector>
#include <memory>

namespace detail {
    class OOAsmInstruction;

    // Program represented as a vector of instructions.
    class Program {
    public:
        using InstructionsListType = std::initializer_list<std::shared_ptr<OOAsmInstruction>>;

        Program(InstructionsListType instructionsList) : program(instructionsList) {}

        [[nodiscard]] auto begin() const {
            return program.begin();
        }

        [[nodiscard]] auto end() const {
            return program.end();
        }

    private:
        std::vector<std::shared_ptr<OOAsmInstruction>> program;
    };
}

// Function for representing lists of instructions in OOAsm code.
detail::Program program(detail::Program::InstructionsListType instructionsList);

#endif // JNP1_PROGRAM_H
