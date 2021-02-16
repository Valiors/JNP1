#ifndef JNP1_OOASM_ELEMENT_H
#define JNP1_OOASM_ELEMENT_H

#include <memory>
#include <string>
#include "memory.h"

namespace detail {
    class WrongIdentifierException : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override {
            return "Wrong identifier.";
        }
    };

    // Hierarchy of OOAsm elements.
    class OOAsmElement {
    public:
        virtual ~OOAsmElement() = default;
    };

    class Lvalue : public OOAsmElement {
    public:
        // Returns address corresponding to a given lvalue.
        [[nodiscard]] virtual Memory::AddressType getLvalue(const Memory &memory) const = 0;
    };

    class Rvalue : public OOAsmElement {
    public:
        // Returns word corresponding to a given rvalue.
        [[nodiscard]] virtual Memory::WordType getRvalue(const Memory &memory) const = 0;
    };

    using LvaluePointer = std::unique_ptr<Lvalue>;
    using RvaluePointer = std::unique_ptr<Rvalue>;

    class Identifier : public OOAsmElement {
    public:
        // Proper identifier has length from 1 to 10 inclusive.
        explicit Identifier(const char *name) {
            if (name == nullptr) {
                throw WrongIdentifierException();
            }

            id = name;

            if (id.empty() || id.size() > 10) {
                throw WrongIdentifierException();
            }
        }

        [[nodiscard]] const std::string &getId() const {
            return id;
        }

    private:
        std::string id;
    };

    // Classes expressing lvalues/rvalues, overriding value getters.
    class Num : public Rvalue {
    public:
        explicit Num(Memory::WordType number) : number(number) {}

        [[nodiscard]] Memory::WordType getRvalue([[maybe_unused]] const Memory &memory) const override {
            return number;
        }

    private:
        Memory::WordType number;
    };

    using NumPointer = std::unique_ptr<Num>;

    class Mem : public Lvalue, public Rvalue {
    public:
        explicit Mem(RvaluePointer address) : address(std::move(address)) {}

        [[nodiscard]] Memory::AddressType getLvalue(const Memory &memory) const override {
            return static_cast<Memory::AddressType>(address->getRvalue(memory));
        }

        [[nodiscard]] Memory::WordType getRvalue(const Memory &memory) const override {
            return memory.getWord(static_cast<Memory::AddressType>(address->getRvalue(memory)));
        }

    private:
        RvaluePointer address;
    };

    class Lea : public Rvalue {
    public:
        explicit Lea(Identifier identifier) : identifier(std::move(identifier)) {}

        [[nodiscard]] Memory::WordType getRvalue(const Memory &memory) const override {
            return memory.getVariableAddress(identifier.getId());
        }

    private:
        Identifier identifier;
    };
}

// Functions for expressing OOAsm elements in OOAsm code.
std::unique_ptr<detail::Num> num(detail::Memory::WordType number);

std::unique_ptr<detail::Mem> mem(detail::RvaluePointer address);

std::unique_ptr<detail::Lea> lea(const char *name);

#endif // JNP1_OOASM_ELEMENT_H
