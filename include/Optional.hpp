#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

#include <stdexcept>
#include <iostream>

template <typename T>
class Optional {
public:
    static Optional<T> None() {
	return Optional(true);
    }
    static Optional<T> Just(T value) {
	Optional<T> some = Optional(false);
	some.x = value;
	return some;
    }
    bool isEmpty() const { return empty; }
    T getDefault(T default_value) {
	return empty ? default_value : x;
    }
    // throws: std::runtime_error
    T get() const {
	if (empty) {
	    throw std::runtime_error("Get on None");
	}
	return x;
    }

    template <typename B>
    Optional<B> map(Optional<B> (*f)(T)) {
	if(empty) {
	    return Optional<B>::None();
	} else {
	    return f(x);
	}
    }
private:
    Optional(bool is_none) : empty(is_none) {}
    bool empty;
    T x;
};
template <typename T>
Optional<T> Just(T value) {
    return Optional<T>::Just(value);
}
template <typename T>
Optional<T> None() {
    return Optional<T>::None();
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Optional<T> &o) {
    if(o.isEmpty()) {
	return os << "Empty()";
    } else {
	return os << "Just(" << o.get() << ")";
    }
}


#endif /* _OPTIONAL_H_ */
