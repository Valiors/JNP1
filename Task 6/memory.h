#ifndef JNP1_MEMORY_H
#define JNP1_MEMORY_H

#include <vector>
#include <unordered_map>
#include <exception>
#include <ostream>
#include <cstdint>

namespace detail {
    // Exceptions for memory safety.
    class VariableNotFoundException : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override {
            return "Variable with a given name not found.";
        }
    };

    class AccessOutsideMemoryException : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override {
            return "Access outside memory.";
        }
    };

    class TooManyVariablesException : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override {
            return "Maximum number of variables exceeded.";
        }
    };

    // Class representing concept of a computer memory.
    class Memory {
    public:
        // Memory words are 64-bit signed integers.
        using WordType = int64_t;

        // Memory is addressed using 64-bit unsigned integers.
        using AddressType = uint64_t;

        // Memory is represented as a vector of words.
        using MemoryType = std::vector<WordType>;

        // Mapping from variable name to its address in memory.
        using LeaMapType = std::unordered_map<std::string, AddressType>;

        // Creates memory with count words.
        explicit Memory(AddressType count) : words(count), variableCount(0), leaMap() {}

        // Returns word at a given address. Throws AccessOutsideMemoryException
        // if address does not represent a word in memory.
        [[nodiscard]] WordType getWord(AddressType address) const {
            if (address < words.size()) {
                return words[address];
            } else {
                throw AccessOutsideMemoryException();
            }
        }

        // Sets word at a given address to a given word.
        // Throws AccessOutsideMemoryException if address does not
        // represent a word in memory.
        void setWord(AddressType address, WordType word) {
            if (address < words.size()) {
                words[address] = word;
            } else {
                throw AccessOutsideMemoryException();
            }
        }

        // Creates named variable with a given initial value.
        // Throws TooManyVariablesException if new variable would exceed memory.
        void addVariable(const std::string &name, WordType value) {
            if (variableCount < words.size()) {
                auto it = leaMap.find(name);

                if (it == leaMap.end()) {
                    leaMap.insert({name, variableCount});
                }

                words[variableCount++] = value;
            } else {
                throw TooManyVariablesException();
            }
        }

        // Finds variable address by a given name.
        // Throws VariableNotFoundException if there is no variable with a given name.
        [[nodiscard]] AddressType getVariableAddress(const std::string &name) const {
            auto it = leaMap.find(name);

            if (it == leaMap.end()) {
                throw VariableNotFoundException();
            }

            return it->second;
        }

        // Dumps memory to a given std::ostream.
        void dumpMemory(std::ostream &os) const {
            for (auto word : words) {
                os << word << ' ';
            }
        }

        // Clears memory.
        void clear() {
            std::fill(words.begin(), words.end(), 0);

            variableCount = 0;

            leaMap.clear();
        }

    private:
        MemoryType words;
        AddressType variableCount;
        LeaMapType leaMap;
    };
}

#endif // JNP1_MEMORY_H
