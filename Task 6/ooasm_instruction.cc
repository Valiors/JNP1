#include "ooasm_instruction.h"

detail::InstructionPointer data(const char *name, detail::NumPointer number) {
    return std::make_shared<detail::Data>(detail::Identifier(name), std::move(number));
}

detail::InstructionPointer mov(detail::LvaluePointer destination, detail::RvaluePointer source) {
    return std::make_shared<detail::Mov>(std::move(destination), std::move(source));
}

detail::InstructionPointer add(detail::LvaluePointer arg1, detail::RvaluePointer arg2) {
    return std::make_shared<detail::Add>(std::move(arg1), std::move(arg2));
}

detail::InstructionPointer sub(detail::LvaluePointer arg1, detail::RvaluePointer arg2) {
    return std::make_shared<detail::Sub>(std::move(arg1), std::move(arg2));
}

detail::InstructionPointer inc(detail::LvaluePointer arg) {
    return std::make_shared<detail::Inc>(std::move(arg));
}

detail::InstructionPointer dec(detail::LvaluePointer arg) {
    return std::make_shared<detail::Dec>(std::move(arg));
}

detail::InstructionPointer one(detail::LvaluePointer arg) {
    return std::make_shared<detail::One>(std::move(arg));
}

detail::InstructionPointer ones(detail::LvaluePointer arg) {
    return std::make_shared<detail::Ones>(std::move(arg));
}

detail::InstructionPointer onez(detail::LvaluePointer arg) {
    return std::make_shared<detail::Onez>(std::move(arg));
}