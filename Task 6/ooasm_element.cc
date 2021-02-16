#include "ooasm_element.h"

std::unique_ptr<detail::Num> num(detail::Memory::WordType number) {
    return std::make_unique<detail::Num>(number);
}

std::unique_ptr<detail::Mem> mem(detail::RvaluePointer address) {
    return std::make_unique<detail::Mem>(std::move(address));
}

std::unique_ptr<detail::Lea> lea(const char *name) {
    return std::make_unique<detail::Lea>(detail::Identifier(name));
}