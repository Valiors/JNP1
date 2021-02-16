#ifndef JNP1_PROCESSOR_H
#define JNP1_PROCESSOR_H

#include "program.h"
#include "memory.h"

namespace detail {
    // Computer processor with zero/sign flags and ability to execute programs.
    class Processor {
    public:
        Processor() : zeroFlag(false), signFlag(false) {}

        [[nodiscard]] bool isZeroFlag() const {
            return zeroFlag;
        }

        [[nodiscard]] bool isSignFlag() const {
            return signFlag;
        }

        // Runs a program.
        void run(const Program &program, detail::Memory &memory);

        // Updates flags according to result of arithmetic operation.
        void updateFlags(detail::Memory::WordType result) {
            zeroFlag = result == 0;
            signFlag = result < 0;
        }

    private:
        bool zeroFlag;
        bool signFlag;
    };
}

#endif // JNP1_PROCESSOR_H
