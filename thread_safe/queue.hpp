#pragma once

#include "common/common.hpp"

namespace ThreadSafe
{
namespace QueuePolicy
{
enum class Discard : uint32_t
{
    DISCARD_OLDEST = 0,
    DISCARD_NEWEST = 1,
    NO_DISCARD = 2
};

enum class Control : uint32_t
{
    OPEN_PUSH = 1,
    OPEN_POP = 2,
    OPEN_PUSH_AND_POP = 3,
    NO_CONTROL = 4
};

} // namespace QueuePolicy

} // namespace ThreadSafe