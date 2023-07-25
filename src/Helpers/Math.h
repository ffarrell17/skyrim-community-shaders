#pragma once
#include <optional>
#include "Configuration\FeatureValue.h"

namespace Helpers
{
	class Math
	{
	public:

		 template <typename T>
		static T Lerp(T& start, T& end, float progress)
		{
			if (start == end)
				return end;
			else
				return static_cast<T>(static_cast<float>(start) + (static_cast<float>(end) - static_cast<float>(start)) * progress);
		}

		template <typename T>
		static T Lerp(const T& start, const T& end, float progress)
		{
			if (start == end)
				return end;
			else
				return static_cast<T>(static_cast<float>(start) + (static_cast<float>(end) - static_cast<float>(start)) * progress);
		}
	};
}