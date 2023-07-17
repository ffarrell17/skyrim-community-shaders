#pragma once
#include "PCH.h"

namespace Helpers
{
	class Time
	{
	public:
		static bool GetDateTime(tm& time);
		static bool GetTime(tm& time);
		static int TimeCmp(const tm& tm1, const tm& tm2);
		static int TimeToMinutes(const tm& time);
		static int TimeToSeconds(const tm& time);
		static void TimeToString(const tm& time, char* buffer, std::size_t bufferSize);

	private:

		static int GetMinutes(RE::Calendar* calendar);
	};
}