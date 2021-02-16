#ifndef JNP1_COMPUTER_H
#define JNP1_COMPUTER_H

#include "processor.h"

// Computer consists of memory and processor executing programs.
class Computer {
public:
    explicit Computer(detail::Memory::AddressType count) : memory(count), processor() {}

    void memory_dump(std::ostream &os) const {
        memory.dumpMemory(os);
    }

    void boot(const detail::Program &program) {
        processor.run(program, memory);
    }

private:
    detail::Memory memory;
    detail::Processor processor;
};

#endif // JNP1_COMPUTER_H
