#ifndef COMPUTER_H
#define COMPUTER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace detail {

// Always false, for use in static assertions which should only be checked after the template
// parameter is known.
    template<typename> constexpr bool tfalse = false;

// ArrayVec is an array-based type similar to std::vector, which can be used in C++17 constexpr. The
// limitation necessary to implement it is having a fixed amount of storage. These arrays will store
// things like instructions and labels, so in order not to parametrize everything over their count,
// an ArrayVecRef type is provided, which implements push_back, indexing, and iteration, but does
// not require providing the size as a template parameter. It can only live as long as the ArrayVec
// it was created from, and all changes apply to the underlying ArrayVec directly.

    template<typename T>
    struct ArrayVecRef {
        constexpr void push_back(T x) {
            array[(*size)++] = x;
        }

        constexpr T *begin() {
            return array;
        }

        constexpr T *end() {
            return array + *size;
        }

        constexpr T &operator[](size_t index) {
            return array[index];
        }

        T *array = nullptr;
        size_t *size = nullptr;
    };

    template<typename T, size_t capacity>
    struct ArrayVec {
        constexpr ArrayVecRef<T> as_ref() {
            return {array, &size};
        }

        T array[capacity]{};
        size_t size = 0;
    };

// Checks whether a character in an identifier is valid, and if so, converts it to a number in
// [1, 37) range. The range starts from 1 so that shorter strings are distinguished from 'a'
// letters, and has 36 elements because there are 26 letters (case-insensitive) and 10 digits.
    constexpr uint32_t to_id_code(char c) {
        if ('a' <= c && c <= 'z')
            return c - 'a' + 1;
        else if ('A' <= c && c <= 'Z')
            return c - 'A' + 1;
        else if ('0' <= c && c <= '9')
            return c - '0' + 27;
        else
            throw std::invalid_argument("invalid character in identifier");
    }

} // namespace detail

// Checks whether an identifier is valid, and if so, converts it to an unsigned 32-bit integer. This
// works, because the identifier is up to 6 characters long and each character has at most 37 valid
// values, and 37**6 < 2**32.
constexpr uint32_t Id(const char *raw) {
    if (raw == nullptr)
        throw std::invalid_argument("identifier is a null pointer");

    auto str = std::string_view(raw);
    if (str.empty() || str.size() > 6)
        throw std::invalid_argument("identifier has invalid length");

    uint32_t hash = 0;
    for (auto chr : str)
        hash = hash * 37 + detail::to_id_code(chr);
    return hash;
}

// Declarations of all assembly language elements. These do not require definitions, because they
// are only necessary for matching on them using partial specialization. This is cleaner than
// defining these types themselves, because it makes it possible to use template variables and
// lambdas, which greatly cut down the boilerplate required when defining new instructions.

template<auto value>
struct Num;

template<typename R>
struct Mem;

template<uint32_t id>
struct Lea;

template<uint32_t id, typename Init>
struct D;

template<typename L, typename R>
struct Mov;

template<typename L, typename R>
struct Add;

template<typename L, typename R>
struct Sub;

template<typename L>
struct Inc;

template<typename L>
struct Dec;

template<typename L, typename R>
struct And;

template<typename L, typename R>
struct Or;

template<typename L>
struct Not;

template<typename L, typename R>
struct Cmp;

template<uint32_t id>
struct Label;

template<uint32_t id>
struct Jmp;

template<uint32_t id>
struct Jz;

template<uint32_t id>
struct Js;

// Forward declaration of the Computer struct. Necessary to define the function pointer type that
// instruction implementations will be converted to.

template<size_t memory_size, typename Word>
struct Computer;

namespace detail {

// These structs contain all information required to execute the program. Template variable
// specialization waiting ahead will convert the types to these structs, which constexpr will be
// then able to use in the same way normal runtime code would do.

    template<typename Word>
    struct BakedVariable {
        uint32_t id = 0;
        size_t address = 0;
        // The initial value to which the variable will be set before the program execution.
        Word init = 0;
    };

    struct BakedLabel {
        uint32_t id = 0;
        size_t address = 0;
    };

    template<size_t memory_size, typename Word>
    struct BakedInstruction {
        void (*execute)(Computer<memory_size, Word> &computer);
    };

// Extracts information from a D type to a BakedVariable struct. The variable address is not set at
// this point. Imperative constexpr code will assign the correct address later, which is cleaner
// than doing this with template metaprogramming.

    template<typename Word, typename I> constexpr std::optional<BakedVariable<Word>>
            match_variable = std::nullopt;

    template<typename Word, uint32_t id, auto init> constexpr std::optional<BakedVariable<Word>>
            match_variable<Word, D<id, Num<init>>> = BakedVariable<Word>{id, 0, static_cast<Word>(init)};

// Extracts information from a Label type to a BakedLabel struct. The label address is not set at
// this point. Imperative constexpr code will assign the correct address later, which is cleaner
// than doing this with template metaprogramming.

    template<typename I> constexpr std::optional<BakedLabel>
            match_label = std::nullopt;

    template<uint32_t id> constexpr std::optional<BakedLabel>
            match_label<Label<id>> = BakedLabel{id, 0};

// Set up some template variables. These will be used for matching on various Mem/Num/Add/... types,
// so that they can convert their information into constexpr types for the rest of the code to use.
// If a pattern is not matched, it will fall back to this implementation. The static_assert will
// then fire, displaying a message informing about the issue and also displaying the I type which
// could not be matched.

    template<typename I> constexpr auto lvalue = [](auto &) {
        static_assert(tfalse<I>, "pattern not matched in `lvalue`");
    };

    template<typename I> constexpr auto rvalue = [](auto &) {
        static_assert(tfalse<I>, "pattern not matched in `rvalue`");
    };

    template<typename I> constexpr auto instruction = [](auto &) {
        static_assert(tfalse<I>, "pattern not matched in `instruction`");
    };

// Use partial variable specialization to create a lambda which computer an lvalue's address using
// the computer state. Instructions' implementations will later use this lambda to compute the
// address and access the computer memory.

    template<typename R> constexpr auto lvalue<Mem<R>> = [](auto &computer) {
        return computer.address_cast(rvalue<R>(computer));
    };

// Use partial variable specialization to create a lambda which computes an rvalue using the
// computer state. These will be used by the instructions' implementations.

    template<auto value> constexpr auto rvalue<Num<value>> = [](auto &) {
        return value;
    };

    template<typename R> constexpr auto rvalue<Mem<R>> = [](auto &computer) {
        return computer.memory[computer.address_cast(rvalue<R>(computer))];
    };

    template<uint32_t id> constexpr auto rvalue<Lea<id>> = [](auto &computer) {
        for (auto &&variable : computer.variables)
            if (variable.id == id)
                return variable.address;
        throw std::invalid_argument("variable does not exist");
    };

// Use partial variable specialization again to match on the instruction type, and create a lambda
// which modifies the computer state appropriately. These will naturally have different types, but
// they can all be converted to a function pointer, which takes a Computer<memory_size, Word>& and
// returns void. The code will later collect these function pointers into an array and use normal
// constexpr to process them.

// There is some duplication over here that would be trivial to fix with some higher-order lambdas,
// but it would probably make the program logic needlessly confusing.

    template<uint32_t id, auto init> constexpr auto instruction<D<id, Num<init>>> = [](auto &computer) {
        ++computer.instruction_pointer;
    };

    template<typename L, typename R> constexpr auto instruction<Mov<L, R>> = [](auto &computer) {
        computer.memory[lvalue<L>(computer)] = rvalue<R>(computer);
        ++computer.instruction_pointer;
    };

    template<typename L, typename R> constexpr auto instruction<Add<L, R>> = [](auto &computer) {
        auto &target = computer.memory[lvalue<L>(computer)];
        target += rvalue<R>(computer);
        computer.zero_flag = target == 0;
        computer.sign_flag = target < 0;
        ++computer.instruction_pointer;
    };

    template<typename L, typename R> constexpr auto instruction<Sub<L, R>> = [](auto &computer) {
        auto &target = computer.memory[lvalue<L>(computer)];
        target -= rvalue<R>(computer);
        computer.zero_flag = target == 0;
        computer.sign_flag = target < 0;
        ++computer.instruction_pointer;
    };

    template<typename L> constexpr auto instruction<Inc<L>> = instruction<Add<L, Num<1>>>;

    template<typename L> constexpr auto instruction<Dec<L>> = instruction<Sub<L, Num<1>>>;

    template<typename L, typename R> constexpr auto instruction<And<L, R>> = [](auto &computer) {
        auto &target = computer.memory[lvalue<L>(computer)];
        target &= rvalue<R>(computer);
        computer.zero_flag = target == 0;
        ++computer.instruction_pointer;
    };

    template<typename L, typename R> constexpr auto instruction<Or<L, R>> = [](auto &computer) {
        auto &target = computer.memory[lvalue<L>(computer)];
        target |= rvalue<R>(computer);
        computer.zero_flag = target == 0;
        ++computer.instruction_pointer;
    };

    template<typename L> constexpr auto instruction<Not<L>> = [](auto &computer) {
        auto &target = computer.memory[lvalue<L>(computer)];
        target = ~target;
        computer.zero_flag = target == 0;
        ++computer.instruction_pointer;
    };

    template<typename L, typename R> constexpr auto instruction<Cmp<L, R>> = [](auto &computer) {
        auto result = computer.word_cast(rvalue<L>(computer));
        result -= rvalue<R>(computer);
        computer.zero_flag = result == 0;
        computer.sign_flag = result < 0;
        ++computer.instruction_pointer;
    };

    template<uint32_t id> constexpr auto instruction<Label<id>> = [](auto &computer) {
        ++computer.instruction_pointer;
    };

    template<uint32_t id> constexpr auto instruction<Jmp<id>> = [](auto &computer) {
        for (auto &&label : computer.labels) {
            if (label.id == id) {
                computer.instruction_pointer = label.address;
                return;
            }
        }
        throw std::invalid_argument("label does not exist");
    };

    template<uint32_t id> constexpr auto instruction<Jz<id>> = [](auto &computer) {
        if (computer.zero_flag)
            instruction<Jmp<id>>(computer);
        else
            ++computer.instruction_pointer;
    };

    template<uint32_t id> constexpr auto instruction<Js<id>> = [](auto &computer) {
        if (computer.sign_flag)
            instruction<Jmp<id>>(computer);
        else
            ++computer.instruction_pointer;
    };

// These functions use the helper match_variable and match_label specializations, and push it to an
// array. It's more convenient to do this, because then the rest of the code does not have to care
// about skipping invalid entries.

    template<typename I, typename Word>
    constexpr void parse_variable(
            ArrayVecRef<BakedVariable<Word>> variables,
            size_t &address
    ) {
        auto variable = match_variable<Word, I>;
        if (variable.has_value()) {
            variable->address = address++;
            variables.push_back(*variable);
        }
    }

    template<typename I>
    constexpr void parse_label(ArrayVecRef<BakedLabel> labels, size_t address) {
        auto label = match_label<I>;
        if (label.has_value()) {
            label->address = address;
            labels.push_back(*label);
        }
    }

} // namespace detail

// This type has access to the parameter pack with all instructions, variable declarations, and
// labels. Because it's inconvenient to work with these, it has three methods that convert these
// to arrays of structs which the actual implementation can later use in an imperative manner.
template<typename... Is>
struct Program {
    template<typename Word>
    static constexpr auto generate_variables() {
        using namespace detail;
        ArrayVec<BakedVariable<Word>, sizeof...(Is)> variables_tmp;
        size_t address = 0;
        (parse_variable<Is>(variables_tmp.as_ref(), address), ...);
        return variables_tmp;
    }

    static constexpr auto generate_labels() {
        using namespace detail;
        ArrayVec<BakedLabel, sizeof...(Is)> labels_tmp;
        size_t address = 0;
        (parse_label<Is>(labels_tmp.as_ref(), address++), ...);
        return labels_tmp;
    }

    template<size_t memory_size, typename Word>
    static constexpr auto generate_instructions() {
        using namespace detail;
        ArrayVec<BakedInstruction<memory_size, Word>, sizeof...(Is)> instructions_tmp;
        (instructions_tmp.as_ref().push_back({instruction<Is>}), ...);
        return instructions_tmp;
    }
};

template<size_t memory_size, typename Word>
struct Computer {
    using Address = std::make_unsigned_t<Word>;

    static_assert(memory_size == 0 || memory_size - 1 <= std::numeric_limits<Address>::max());

    // Compute the result as a normal constexpr function. Because of that, it is not guaranteed to
    // be executed during compilation.
    template<typename P>
    static constexpr auto boot_dynamic() {
        auto variables = P::template generate_variables<Word>();
        auto labels = P::generate_labels();
        auto instructions = P::template generate_instructions<memory_size, Word>();
        auto computer = Computer{};
        computer.variables = variables.as_ref();
        computer.labels = labels.as_ref();
        computer.instructions = instructions.as_ref();
        computer.initialize_variables();
        computer.execute();
        return computer.memory;
    }

    // Wrapper around boot_dynamic, which assigns the result to a constexpr variable to make sure it
    // is computed during compilation. If this is not possible, the compiler will emit an error
    // here.
    template<typename P>
    static constexpr auto boot() {
        constexpr auto memory = boot_dynamic<P>();
        return memory;
    }

    constexpr void initialize_variables() {
        for (auto &&variable : variables)
            memory[variable.address] = variable.init;
    }

    constexpr void execute() {
        while (instruction_pointer < *instructions.size) {
            auto instruction = instructions[instruction_pointer];
            (instruction.execute)(*this);
        }
    }

    template<typename T>
    constexpr Word word_cast(T x) {
        return x;
    }

    template<typename T>
    constexpr Address address_cast(T x) {
        return x;
    }

    std::array<Word, memory_size> memory{};
    size_t instruction_pointer = 0;
    bool zero_flag = false;
    bool sign_flag = false;
    detail::ArrayVecRef<detail::BakedVariable<Word>> variables{};
    detail::ArrayVecRef<detail::BakedLabel> labels{};
    detail::ArrayVecRef<detail::BakedInstruction<memory_size, Word>> instructions{};
};

#endif // COMPUTER_H
