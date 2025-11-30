#ifndef UTILS_H
#define UTILS_H

#include <utility>

template <typename Callable>
class Defer {
    Callable m_func;

  public:
    explicit Defer(Callable&& func)
        : m_func(std::forward<Callable>(func)) {}

    Defer()             = delete;
    Defer(const Defer&) = delete;

    ~Defer() {
        m_func();
    }
};

#endif  // UTILS_H