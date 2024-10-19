#pragma once

#include <cstdint>
namespace ThreadSafe
{

	enum class QueuePolicy : uint32_t
	{
		DISCARD_OLDEST = 0,
		DISCARD_NEWEST = 1,
		NO_DISCARD = 2
	};

} // namespace ThreadSafe