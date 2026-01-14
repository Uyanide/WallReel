/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-11-30 20:59:57
 * @LastEditTime: 2025-12-07 06:08:18
 * @Description: THE utils header that every project needs :)
 */

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