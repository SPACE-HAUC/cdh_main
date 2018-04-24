#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

#include <stdexcept>
#include <iostream>

/**
 * @brief A class for representing optional values. This is useful for
 * encoding data that may or may not exist.
 * Optional values are called 'Options'.
 * Data that is present is called `Just` a value, or a 'present' value.
 * Data that is not present is called `None`, or an 'empty' value.
 */
template <typename T>
class Optional {
public:

    /**
     * @brief Construct a new empty value.
     *
     * @return A new optional value that is None.
     */
    static Optional<T> None() {
        return Optional(true);
    }

    /**
     * @brief Construct a new present value.
     *
     * @param value The value to store.
     * @return A new optional value that is Just the value.
     */
    static Optional<T> Just(T value) {
        Optional<T> some = Optional(false);
        some.x = value;
        return some;
    }

    /**
     * @brief Is this value empty? (Is it None?)
     *
     * @return Is this value empty?
     */
    bool isEmpty() const { return empty; }

    /**
     * @brief Return this Option's value, if present. Otherwise,
     * return the given default.
     *
     * @param default_value The default value to return in case of None.
     * @return The value in this Option or `default_value`.
     */
    T getDefault(T default_value) {
        return empty ? default_value : x;
    }

    // throws: std::runtime_error

    /**
     * @brief Get this Option's value. *Note that this method throws
     * std::runtime_error if this is a None.*
     *
     * @return This Option's value.
     */
    T get() const {
        if (empty) {
            throw std::runtime_error("Get on None");
        }
        return x;
    }


    /**
     * @brief Apply the given function to this Option's value, if it
     * is present, to obtain a new optional value.
     *
     * @param f The function to apply.
     * @return The new value.
     */
    template <typename B>
    Optional<B> map(B (*f)(T)) {
        if(empty) {
            return Optional<B>::None();
        } else {
            return Optional<B>::Just(f(x));
        }
    }

    /**
     * @brief Apply the given function to this Option's value, if it
     * is present, to obtain a new optional value by flattening the
     * return of the function into a single optional value.
     *
     * @param f The function to apply.
     * @return The new value.
     */
    template <typename B>
    Optional<B> flatMap(Optional<B> (*f)(T)) {
        if(empty) {
            return Optional<B>::None();
        } else {
            return f(x);
        }
    }

private:
    /**
     * @brief Default constructor for optional values. Constructs an
     * empty value. This should not be used directly. Use `Just` and
     * `None` instead.
     *
     * @param is_none Is the value `None`?
     * @return A new optional value.
     */
    Optional() : empty(true) {}

    /**
     * @brief Direct constructor for optional values. This should not
     * be used directly. Use `Just` and `None` instead.
     *
     * @param is_none Is the value `None`?
     * @return A new optional value.
     */
    Optional(bool is_none) : empty(is_none) {}

    /** Is this value empty? */
    bool empty;
    /** The value being stored. */
    T x;
};

/**
 * @brief Construct a new present optional value.
 *
 * @param value The value to store.
 * @return A new optional value.
 */
template <typename T>
Optional<T> Just(T value) {
    return Optional<T>::Just(value);
}

/**
 * @brief Construct a new empty optional value.
 *
 * @return A new optional value.
 */
template <typename T>
Optional<T> None() {
    return Optional<T>::None();
}


/**
 * @brief Overloaded output stream operator to support transparent
 * printing of optional values.
 *
 * @param os The output stream.
 * @param o The optional value.
 * @return The output stream.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const Optional<T> &o) {
    if(o.isEmpty()) {
        return os << "Empty()";
    } else {
        return os << "Just(" << o.get() << ")";
    }
}


#endif /* _OPTIONAL_H_ */
