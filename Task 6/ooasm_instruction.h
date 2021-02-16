#ifndef JNP1_OOASM_INSTRUCTION_H
#define JNP1_OOASM_INSTRUCTION_H

#include "ooasm_element.h"
#include "processor.h"

namespace detail {
    // Hierarchy of OOAsm instructions.
    class OOAsmInstruction {
    public:
        // Function preparing memory for executing instruction.
        virtual void prepare([[maybe_unused]] Memory &memory) const {}

        // Function executing instruction.
        virtual void perform([[maybe_unused]] Memory &memory,
                             [[maybe_unused]] Processor &processor) const {}

        virtual ~OOAsmInstruction() = default;
    };

    // Instructions are stored using shared pointers to allow copying.
    using InstructionPointer = std::shared_ptr<OOAsmInstruction>;

    // Classes representing OOAsm instructions, overriding prepare/perform functions.
    class Data : public OOAsmInstruction {
    public:
        Data(Identifier identifier,
             NumPointer number) : identifier(std::move(identifier)), number(std::move(number)) {}

        void prepare(Memory &memory) const override {
            memory.addVariable(identifier.getId(), number->getRvalue(memory));
        }

    private:
        Identifier identifier;
        NumPointer number;
    };

    class Mov : public OOAsmInstruction {
    public:
        Mov(LvaluePointer destination,
            RvaluePointer source) : destination(std::move(destination)), source(std::move(source)) {}

        void perform(Memory &memory, [[maybe_unused]] Processor &processor) const override {
            memory.setWord(destination->getLvalue(memory), source->getRvalue(memory));
        }

    private:
        LvaluePointer destination;
        RvaluePointer source;
    };

    class Add : public OOAsmInstruction {
    public:
        Add(LvaluePointer arg1,
            RvaluePointer arg2) : arg1(std::move(arg1)), arg2(std::move(arg2)) {}

        void perform(Memory &memory, Processor &processor) const override {
            auto newWord = memory.getWord(arg1->getLvalue(memory)) + arg2->getRvalue(memory);

            memory.setWord(arg1->getLvalue(memory), newWord);

            processor.updateFlags(newWord);
        }

    private:
        LvaluePointer arg1;
        RvaluePointer arg2;
    };

    class Sub : public OOAsmInstruction {
    public:
        Sub(LvaluePointer arg1,
            RvaluePointer arg2) : arg1(std::move(arg1)), arg2(std::move(arg2)) {}

        void perform(Memory &memory, Processor &processor) const override {
            auto newWord = memory.getWord(arg1->getLvalue(memory)) - arg2->getRvalue(memory);

            memory.setWord(arg1->getLvalue(memory), newWord);

            processor.updateFlags(newWord);
        }

    private:
        LvaluePointer arg1;
        RvaluePointer arg2;
    };

    class Inc : public OOAsmInstruction {
    public:
        explicit Inc(LvaluePointer arg) : arg(std::move(arg)) {}

        void perform(Memory &memory, Processor &processor) const override {
            auto newWord = memory.getWord(arg->getLvalue(memory)) + 1;

            memory.setWord(arg->getLvalue(memory), newWord);

            processor.updateFlags(newWord);
        }

    private:
        LvaluePointer arg;
    };

    class Dec : public OOAsmInstruction {
    public:
        explicit Dec(LvaluePointer arg) : arg(std::move(arg)) {}

        void perform(Memory &memory, Processor &processor) const override {
            auto newWord = memory.getWord(arg->getLvalue(memory)) - 1;

            memory.setWord(arg->getLvalue(memory), newWord);

            processor.updateFlags(newWord);
        }

    private:
        LvaluePointer arg;
    };

    class One : public OOAsmInstruction {
    public:
        explicit One(LvaluePointer arg) : arg(std::move(arg)) {}

        void perform(Memory &memory, [[maybe_unused]] Processor &processor) const override {
            memory.setWord(arg->getLvalue(memory), 1);
        }

    private:
        LvaluePointer arg;
    };

    class Onez : public OOAsmInstruction {
    public:
        explicit Onez(LvaluePointer arg) : arg(std::move(arg)) {}

        void perform(Memory &memory, Processor &processor) const override {
            if (processor.isZeroFlag()) {
                memory.setWord(arg->getLvalue(memory), 1);
            }
        }

    private:
        LvaluePointer arg;
    };

    class Ones : public OOAsmInstruction {
    public:
        explicit Ones(LvaluePointer arg) : arg(std::move(arg)) {}

        void perform(Memory &memory, Processor &processor) const override {
            if (processor.isSignFlag()) {
                memory.setWord(arg->getLvalue(memory), 1);
            }
        }

    private:
        LvaluePointer arg;
    };
}

// Functions for expressing OOAsm instructions in OOAsm code.
detail::InstructionPointer data(const char *name, detail::NumPointer number);

detail::InstructionPointer mov(detail::LvaluePointer destination, detail::RvaluePointer source);

detail::InstructionPointer add(detail::LvaluePointer arg1, detail::RvaluePointer arg2);

detail::InstructionPointer sub(detail::LvaluePointer arg1, detail::RvaluePointer arg2);

detail::InstructionPointer inc(detail::LvaluePointer arg);

detail::InstructionPointer dec(detail::LvaluePointer arg);

detail::InstructionPointer one(detail::LvaluePointer arg);

detail::InstructionPointer ones(detail::LvaluePointer arg);

detail::InstructionPointer onez(detail::LvaluePointer arg);

#endif // JNP1_OOASM_INSTRUCTION_H
