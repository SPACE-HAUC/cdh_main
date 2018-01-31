#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

#include <stdexcept>

template <typename T>
class Optional {
public:
    static Optional<T> None() {
	return Optional(true);
    }
    static Optional<T> Just(T value) {
	auto some = Optional(false);
	some.x = value;
	return some;
    }
    bool isEmpty() { return empty; }
    T getDefault(T default_value) {
	return empty ? default_value : x;
    }
    // throws: std::runtime_error
    T get() {
	if (empty) {
	    throw std::runtime_error("Get on None");
	}
	return x;
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

#endif /* _OPTIONAL_H_ */
