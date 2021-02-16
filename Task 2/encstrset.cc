#include "encstrset.h"

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <climits>

// Printing debug messages.
namespace {
#ifdef NDEBUG
    constexpr bool debug = false;
#else
    constexpr bool debug = true;
#endif

    // Debug form of ciphered string - character's codes, separated by spaces, in 2-digit HEX form.
    std::string hex_cipher(const std::string &cipher) {
        std::ostringstream hex_form;

        for (std::string::size_type i = 0; i < cipher.size(); i++) {
            hex_form << std::hex << std::uppercase
                     << std::setfill('0') << std::setw(2)
                     << static_cast<unsigned>(static_cast<unsigned char>(cipher[i]));

            // No unsigned wrapping - cipher.size() >= 1 here.
            if (i < cipher.size() - 1) {
                hex_form << " ";
            }
        }

        return hex_form.str();
    }

    // Debug form of ID.
    std::string out_form(unsigned long id) {
        return std::to_string(id);
    }

    // Debug form of string, surrounded by a given char.
    std::string out_form(const std::string &str, const char surrounding = '\"') {
        return surrounding + str + surrounding;
    }

    // Debug form of C-string, surrounded by a given char.
    std::string out_form(const char *str, const char surrounding = '\"') {
        return str == nullptr ? "NULL" : out_form(std::string(str), surrounding);
    }

    // Prints empty argument list.
    void print_function_args() {
        std::cerr << "()" << std::endl;
    }

    // Prints argument list in the form (arg1, arg2, ..., argn).
    template<typename T, typename... Args>
    void print_function_args(T &&first, Args &&... args) {
        std::cerr << '(' << first;
        ((std::cerr << ", " << out_form(args)), ...);
        std::cerr << ')' << std::endl;
    }
}

// Prints function name with arguments in the form func(arg1, arg2, ..., argn).
#define PRINT_FUNCTION(...) do { \
    if (debug) { std::cerr << __func__; print_function_args(__VA_ARGS__); } \
} while (false)

// Prints debug message MSG for a function named FUNC.
#define PRINT_FUNC_DEBUG_MESSAGE(FUNC, MSG) do { \
    if (debug) { std::cerr << FUNC << ": " << MSG << std::endl; } \
} while (false)

// Prints debug message MSG for a function from the point of call.
#define PRINT_DEBUG_MESSAGE(MSG) PRINT_FUNC_DEBUG_MESSAGE(__func__, MSG)

// Helper functions and structures for performing operations from encstrset interface.
namespace {
    // Set of ciphers.
    using CiphersSet = std::unordered_set<std::string>;

    // Map id -> CiphersSet.
    using CiphersSetByID = std::unordered_map<unsigned long, CiphersSet>;

    // Getter for mapping from id to CiphersSet to avoid static initialization fiasco.
    CiphersSetByID &get_set_by_id() {
        static CiphersSetByID set_by_id;

        return set_by_id;
    }

    // Returns value string XOR-ciphered by key string.
    std::string ciphered_string(const std::string &value, const std::string &key) {
        if (key.empty()) {
            return value;
        }

        std::string ciphered(value.size(), '\0');

        for (std::string::size_type i = 0; i < value.size(); i++) {
            ciphered[i] = value[i] ^ key[i % key.size()];
        }

        return ciphered;
    }

    // Merges functionality of encstrset_insert/remove/test by abstracting change to data structures.
    template<typename T>
    bool encstrset_change(unsigned long id, const char *value, const char *key,
                          [[maybe_unused]] const char *name, T &&change) {

        if (value == nullptr) {
            PRINT_FUNC_DEBUG_MESSAGE(name, "invalid value (NULL)");

            return false;
        }

        auto iterator = get_set_by_id().find(id);

        if (iterator != get_set_by_id().end()) {
            auto &ciphers_set = iterator->second;

            std::string cipher = ciphered_string(value, key == nullptr ? "" : key);

            // Performs requested change to data structures and returns result.
            return change(ciphers_set, cipher);
        }

        PRINT_FUNC_DEBUG_MESSAGE(name, "set #" << id << " does not exist");

        return false;
    }
}

namespace jnp1 {
    unsigned long encstrset_new() {
        // Every set is given next non-negative number, starting from 0.
        static unsigned long set_counter = 0;

        assert(set_counter < ULONG_MAX);

        PRINT_FUNCTION();

        // Constructs empty set with id set_counter.
        get_set_by_id()[set_counter];

        PRINT_DEBUG_MESSAGE("set #" << set_counter << " created");

        return set_counter++;
    }

    void encstrset_delete(unsigned long id) {
        PRINT_FUNCTION(id);

        [[maybe_unused]] bool deleted = get_set_by_id().erase(id);

        PRINT_DEBUG_MESSAGE("set #" << id << (deleted ? " deleted" : " does not exist"));
    }

    size_t encstrset_size(unsigned long id) {
        PRINT_FUNCTION(id);

        auto iterator = get_set_by_id().find(id);

        if (iterator != get_set_by_id().end()) {
            auto size = iterator->second.size();

            PRINT_DEBUG_MESSAGE("set #" << id << " contains " << size << " element(s)");

            return size;
        }

        PRINT_DEBUG_MESSAGE("set #" << id << " does not exist");

        return 0;
    }

    bool encstrset_insert(unsigned long id, const char *value, const char *key) {
        PRINT_FUNCTION(id, value, key);

        const auto &name = __func__;

        // Insert cipher into ciphers_set.
        return encstrset_change(id, value, key, name, [&](CiphersSet &ciphers_set, const std::string &cipher) {
            bool inserted = ciphers_set.insert(cipher).second;

            PRINT_FUNC_DEBUG_MESSAGE(name, "set #" << id << ", cypher " << out_form(hex_cipher(cipher))
                                                   << (inserted ? " inserted" : " was already present"));

            return inserted;
        });
    }

    bool encstrset_remove(unsigned long id, const char *value, const char *key) {
        PRINT_FUNCTION(id, value, key);

        const auto &name = __func__;

        // Remove cipher from ciphers_set.
        return encstrset_change(id, value, key, name, [&](CiphersSet &ciphers_set, const std::string &cipher) {
            bool removed = ciphers_set.erase(cipher);

            PRINT_FUNC_DEBUG_MESSAGE(name, "set #" << id << ", cypher " << out_form(hex_cipher(cipher))
                                                   << (removed ? " removed" : " was not present"));

            return removed;
        });
    }

    bool encstrset_test(unsigned long id, const char *value, const char *key) {
        PRINT_FUNCTION(id, value, key);

        const auto &name = __func__;

        // Test if cipher is present in ciphers_set.
        return encstrset_change(id, value, key, name, [&](CiphersSet &ciphers_set, const std::string &cipher) {
            bool present = ciphers_set.count(cipher);

            PRINT_FUNC_DEBUG_MESSAGE(name, "set #" << id << ", cypher " << out_form(hex_cipher(cipher))
                                                   << (present ? " is present" : " is not present"));

            return present;
        });
    }

    void encstrset_clear(unsigned long id) {
        PRINT_FUNCTION(id);

        auto iterator = get_set_by_id().find(id);

        if (iterator != get_set_by_id().end()) {
            iterator->second.clear();

            PRINT_DEBUG_MESSAGE("set #" << id << " cleared");
        } else {
            PRINT_DEBUG_MESSAGE("set #" << id << " does not exist");
        }
    }

    void encstrset_copy(unsigned long src_id, unsigned long dst_id) {
        PRINT_FUNCTION(src_id, dst_id);

        auto src_iterator = get_set_by_id().find(src_id);

        if (src_iterator == get_set_by_id().end()) {
            PRINT_DEBUG_MESSAGE("set #" << src_id << " does not exist");

            return;
        }

        auto dst_iterator = get_set_by_id().find(dst_id);

        if (dst_iterator == get_set_by_id().end()) {
            PRINT_DEBUG_MESSAGE("set #" << dst_id << " does not exist");

            return;
        }

        const auto &src_ciphers_set = src_iterator->second;
        auto &dst_ciphers_set = dst_iterator->second;

        // Copy ciphers from src_ciphers_set to dst_ciphers_set one by one.
        for (const std::string &cipher : src_ciphers_set) {
            if (dst_ciphers_set.insert(cipher).second) {
                PRINT_DEBUG_MESSAGE("cypher " << out_form(hex_cipher(cipher))
                                              << " copied from set #" << src_id
                                              << " to set #" << dst_id);
            } else {
                PRINT_DEBUG_MESSAGE("copied cypher " << out_form(hex_cipher(cipher))
                                                     << " was already present in set #" << dst_id);
            }
        }
    }
}