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
				return static_cast<T>(start + (end - start) * progress);
		}

		template <typename T>
		static T Lerp(const T& start, const T& end, float progress)
		{
			if (start == end)
				return end;
			else
				return static_cast<T>(start + (end - start) * progress);
		}

		template <typename T>
		static T Lerp(std::optional<T>& start, std::optional<T>& end, float progress)
		{
			if (start.value() == end.value())
				return end.value();
			else
				return Lerp<T>(start.value(), end.value(), progress);
		}
	};
}