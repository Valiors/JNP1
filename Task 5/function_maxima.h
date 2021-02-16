#ifndef JNP1_FUNCTION_MAXIMA_H
#define JNP1_FUNCTION_MAXIMA_H

#include <set>
#include <memory>
#include <functional>
#include <exception>
#include <utility>

// Exception raised when passing invalid arguments
// to value_at member function of a FunctionMaxima class.
class InvalidArg : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "invalid argument value";
    }
};

// Namespace for hiding implementation details.
namespace detail {
    template<typename A, typename V>
    class FunctionMaximaImpl {
    public:
        class point_type {
        public:
            friend class FunctionMaximaImpl;

            const A &arg() const noexcept {
                return *_arg;
            }

            const V &value() const noexcept {
                return *_value;
            }

        private:
            // In order to allow sharing arguments and values among points in several
            // copies of a function, we have chosen to use shared pointers.
            using ArgPointerType = std::shared_ptr<const A>;
            using ValuePointerType = std::shared_ptr<const V>;

            // Constructs a new point from a given (argument, value) pair.
            point_type(const A &a, const V &v) : _arg(std::make_shared<const A>(a)),
                                                 _value(std::make_shared<const V>(v)) {}

            ArgPointerType _arg;
            ValuePointerType _value;
        };

    private:
        // Comparator for a set of function points. Sorts by increasing arguments.
        struct ComparePointsByArguments {
            using is_transparent = void;

            bool operator()(const point_type &a, const point_type &b) const {
                return std::less<A>()(a.arg(), b.arg());
            }

            bool operator()(const A &a, const point_type &b) const {
                return std::less<A>()(a, b.arg());
            }

            bool operator()(const point_type &a, const A &b) const {
                return std::less<A>()(a.arg(), b);
            }
        };

        // Comparator for a set of local maxima of a function. Sorts by
        // non-increasing values and increasing arguments.
        struct ComparePointsByMaximaOrder {
            bool operator()(const point_type &a, const point_type &b) const {
                return std::less<V>()(b.value(), a.value()) ||
                       (equivalentValues(a.value(), b.value()) && std::less<A>()(a.arg(), b.arg()));
            }
        };

        // Function is implemented as a multiset storing (argument, value) pairs
        // along with a set for fast retrieval of local maxima. Although function
        // cannot have more than one value assigned to an argument, we have chosen
        // multiset in order to ease the implementation of exception guarantees.
        using FunctionType = std::multiset<point_type, ComparePointsByArguments>;
        using LocalMaximaSetType = std::set<point_type, ComparePointsByMaximaOrder>;

    public:
        using iterator = typename FunctionType::const_iterator;
        using mx_iterator = typename LocalMaximaSetType::const_iterator;

        using size_type = typename FunctionType::size_type;

        void set_value(const A &a, const V &v) {
            // Find a point corresponding to an argument.
            iterator it = find(a);

            point_type pt(a, v);

            // If the same (argument, value) pair already exists, return.
            if (it != end() && equivalentPoints(*it, pt)) {
                return;
            }

            mx_iterator mxIt = mxFind(it);

            // Insert new (argument, value) pair into function.
            iterator fnIt = function.insert(pt);

            // Fix maxima status of the new value and its neighbors.
            try {
                auto nextElementGuard = ChangeLocalMaximaGuard(safeNextWithAvoid(fnIt, it), it, *this);
                auto currElementGuard = ChangeLocalMaximaGuard(fnIt, it, *this);
                auto prevElementGuard = ChangeLocalMaximaGuard(safePrevWithAvoid(fnIt, it), it, *this);

                nextElementGuard.done();
                currElementGuard.done();
                prevElementGuard.done();
            }
            catch (...) {
                function.erase(fnIt);
                throw;
            }

            // Erase point associated with the old value, if one exists.
            if (it != end()) {
                function.erase(it);
                mxEraseIfNotEnd(mxIt);
            }
        }

        void erase(const A &a) {
            // Find point corresponding to an argument.
            iterator it = find(a);
            mx_iterator mxIt = mxFind(it);

            if (it != end()) {
                // Fix maxima status of neighbors of a point to erase.
                auto nextElementGuard = ChangeLocalMaximaGuard(safeNext(it), it, *this);
                auto prevElementGuard = ChangeLocalMaximaGuard(safePrev(it), it, *this);

                nextElementGuard.done();
                prevElementGuard.done();

                // Erase point.
                function.erase(it);
                mxEraseIfNotEnd(mxIt);
            }
        }

        const V &value_at(const A &a) const {
            auto it = find(a);

            if (it == end()) {
                throw InvalidArg();
            } else {
                return it->value();
            }
        }

        iterator begin() const noexcept {
            return function.cbegin();
        }

        iterator end() const noexcept {
            return function.cend();
        }

        iterator find(const A &a) const {
            return function.find(a);
        }

        mx_iterator mx_begin() const noexcept {
            return localMaxima.cbegin();
        }

        mx_iterator mx_end() const noexcept {
            return localMaxima.cend();
        }

        size_type size() const noexcept {
            return function.size();
        }

    private:
        // Convenience functions for implementation purposes.
        [[nodiscard]] bool empty() const noexcept {
            return size() == 0;
        }

        static bool equivalentArguments(const A &a, const A &b) {
            return !std::less<A>()(a, b) && !std::less<A>()(b, a);
        }

        static bool equivalentValues(const V &a, const V &b) {
            return !std::less<V>()(a, b) && !std::less<V>()(b, a);
        }

        static bool equivalentPoints(const point_type &pt1, const point_type &pt2) {
            return equivalentArguments(pt1.arg(), pt2.arg()) && equivalentValues(pt1.value(), pt2.value());
        }

        bool valueLessOrEqual(const V &v1, const V &v2) const {
            return std::less<V>()(v1, v2) || equivalentValues(v1, v2);
        }

        // Returns previous iterator or end() if that is impossible.
        iterator safePrev(const iterator &it) const {
            return it != begin() && it != end() ? std::prev(it) : end();
        }

        // Returns next iterator or end() if that is impossible.
        iterator safeNext(const iterator &it) const {
            return it != end() ? std::next(it) : it;
        }

        // Returns previous iterator, ignoring ignore iterator, or end() if there are none.
        iterator safePrevWithAvoid(const iterator &it, const iterator &ignore) const {
            iterator prevIt = safePrev(it);
            return prevIt == ignore ? safePrev(prevIt) : prevIt;
        }

        // Returns next iterator, ignoring ignore iterator, or end() if there are none.
        iterator safeNextWithAvoid(const iterator &it, const iterator &ignore) const {
            iterator nextIt = safeNext(it);
            return nextIt == ignore ? safeNext(nextIt) : nextIt;
        }

        // Checks if an iterator points to a local maxima, ignoring presence of ignore iterator.
        bool isLocalMaxima(const iterator &it, const iterator &ignore) const {
            return it != ignore && firstMaximaCondition(it, ignore) && secondMaximaCondition(it, ignore);
        }

        // Finds local maxima corresponding to a given iterator from a function.
        mx_iterator mxFind(const iterator &it) const {
            return it != end() ? localMaxima.find(*it) : mx_end();
        }

        // Erases element pointer by iterator from a set of local maxima.
        void mxEraseIfNotEnd(const mx_iterator &it) {
            if (it != mx_end()) {
                localMaxima.erase(it);
            }
        }

        // Check if the first condition of a local maxima is met.
        bool firstMaximaCondition(const iterator &it, const iterator &ignore) const {
            if (it == end()) {
                return false;
            }

            if (it == begin()) {
                return true;
            }

            auto leftIt = std::prev(it);

            if (leftIt == ignore) {
                leftIt = safePrev(leftIt);
            }

            return leftIt == end() || valueLessOrEqual(leftIt->value(), it->value());
        }

        // Check if the second condition of a local maxima is met.
        bool secondMaximaCondition(const iterator &it, const iterator &ignore) const {
            if (it == end()) {
                return false;
            }

            auto rightIt = std::next(it);

            if (rightIt == ignore) {
                rightIt = safeNext(rightIt);
            }

            return rightIt == end() || valueLessOrEqual(rightIt->value(), it->value());
        }

        // Transaction manager for updating maxima status of a single function point.
        class ChangeLocalMaximaGuard {
        public:
            // Checks if it is local maxima.
            // Should this point no longer be maxima removal is delayed to destructor.
            ChangeLocalMaximaGuard(const iterator &it, const iterator &ignore, FunctionMaximaImpl &fm) : _fm(fm) {
                if (it == ignore) {
                    return;
                }

                mxIt = _fm.mxFind(it);

                bool wasLocalMaxima = mxIt != _fm.mx_end();
                bool isLocalMaxima = _fm.isLocalMaxima(it, ignore);

                if (isLocalMaxima) {
                    if (!wasLocalMaxima) {
                        mxIt = _fm.localMaxima.insert(*it).first;
                        insert = true;
                    }
                } else if (wasLocalMaxima) {
                    erase = true;
                }
            }

            // Mark operation as completed successfully.
            void done() noexcept {
                reverse = false;
            }

            // If operation wasn't successful revert it.
            // If maxima in given point should be remove it's removed here.
            ~ChangeLocalMaximaGuard() {
                if (reverse) {
                    if (insert) {
                        _fm.localMaxima.erase(mxIt);
                    }
                } else {
                    if (erase) {
                        _fm.localMaxima.erase(mxIt);
                    }
                }
            }

        private:
            bool reverse = true;
            bool erase = false;
            bool insert = false;

            mx_iterator mxIt;

            FunctionMaximaImpl &_fm;
        };

        FunctionType function;
        LocalMaximaSetType localMaxima;
    };
}

// FunctionMaxima class with Copy-On-Write semantics. It delegates
// work to FunctionMaximaImpl through underlying shared pointer and
// wrapper functions. Default copy control for this class has a strong
// exception guarantee, because deep copying happens only in cowMutator
// which in turn uses copy constructor of FunctionMaximaImpl, which has
// a strong guarantee, because std::set/std::multiset do.
template<typename A, typename V>
class FunctionMaxima {
public:
    using Impl = detail::FunctionMaximaImpl<A, V>;

    using point_type = typename Impl::point_type;

    using iterator = typename Impl::iterator;
    using mx_iterator = typename Impl::mx_iterator;

    using size_type = typename Impl::size_type;

    FunctionMaxima() : cow(std::make_shared<Impl>()) {}

    const V &value_at(const A &a) const {
        return cow->value_at(a);
    }

    void set_value(const A &a, const V &v) {
        cowMutator(&Impl::set_value, a, v);
    }

    void erase(const A &a) {
        cowMutator(&Impl::erase, a);
    }

    iterator begin() const noexcept {
        return cow->begin();
    }

    iterator end() const noexcept {
        return cow->end();
    }

    iterator find(const A &a) const {
        return cow->find(a);
    }

    mx_iterator mx_begin() const noexcept {
        return cow->mx_begin();
    }

    mx_iterator mx_end() const noexcept {
        return cow->mx_end();
    }

    size_type size() const noexcept {
        return cow->size();
    }

private:
    // Template function for performing mutating operations f(args), which
    // require copying of underlying data structures due to COW semantics.
    template<typename F, typename... Args>
    void cowMutator(F &&f, Args &&... args) {
        // If more than one function points to the same data, then we need to
        // make ourselves a copy before calling f(args).
        if (cow.use_count() > 1) {
            auto cowBackup = cow;

            try {
                cow = std::make_shared<Impl>(*cow);

                ((*cow).*(std::forward<F>(f)))(std::forward<Args>(args)...);
            }
            catch (...) {
                cow = cowBackup;

                throw;
            }
        } else {
            ((*cow).*(std::forward<F>(f)))(std::forward<Args>(args)...);
        }
    }

    // Shared pointer to an implementation of a FunctionMaxima.
    std::shared_ptr<Impl> cow;
};

#endif // JNP1_FUNCTION_MAXIMA_H
