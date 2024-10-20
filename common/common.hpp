#pragma once

#include "logger.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

/**
 * @brief A macro to indicate that a function parameter is intentionally unused.
 *
 * This macro suppresses compiler warnings about unused parameters by assigning
 * the parameter to std::ignore. It is useful for maintaining clean code
 * while adhering to function signatures that require certain parameters.
 *
 * @param name The name of the parameter that is unused.
 */
#define UNUSED_PARAMETER(name) std::ignore = name

/**
 * @brief Macro to disable copy and move constructors, as well as copy and move assignment operators
 * This macro makes a class non-copyable and non-movable.
 *
 * @usage: Place UNCOPYABLE(ClassName) inside the private or public section of the class.
 * This will prevent the class from being copied or moved.
 *
 * @example:
 * class MyClass {
 * public:
 *     MyClass() = default;
 *     UNCOPYABLE(MyClass);
 * };
 */

#define UNCOPYABLE(ClassName)                        \
    ClassName(const ClassName&) = delete;            \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete;                 \
    ClassName& operator=(ClassName&&) = delete;

/**
 *  @brief Macro to enforce the Rule of Five for a given class.
 *
 *  This macro generates the necessary declarations for the five special member functions:
 *  - Destructor
 *  - Copy Constructor
 *  - Copy Assignment Operator
 *  - Move Constructor
 *  - Move Assignment Operator
 *
 *  It is intended to be used in classes that manage resources (e.g., dynamic memory, file handles)
 *  to ensure proper resource management and avoid memory leaks or undefined behavior.
 *
 *  @param className The name of the class for which the Rule of Five is being enforced.
 */
#define RULE_OF_FIVE(className)                   \
    ~className();                                 \
    className(const className& other);            \
    className& operator=(const className& other); \
    className(className&& other) noexcept;        \
    className& operator=(className&& other) noexcept;
