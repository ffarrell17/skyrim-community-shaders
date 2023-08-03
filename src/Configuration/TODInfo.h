#pragma once

#include "Helpers/Time.h"
#include "Helpers/Weather.h"
#include "Helpers/Location.h"

namespace Configuration
{

	class TODInfo
	{
	public:
		static TODInfo* GetSingleton()
		{
			static TODInfo instance;
			return &instance;
		}

		enum TimePeriod
		{
			NightToSunriseStart,
			SunriseStartToSunriseEnd,
			SunriseEndToDay,
			DayToSunsetStart,
			SunsetStartToSunsetEnd,
			SunsetEndToNight
		};

		// TODValue Info
		bool Valid = false;
		tm Time;
		tm SunriseBeginTime;
		tm SunriseEndTime;
		tm SunsetBeginTime;
		tm SunsetEndTime;
		TimePeriod TimePeriodType;
		float TimePeriodPercentage;
		bool Exterior = true;

		void Update();

		std::string GetTimePeriodStr();

	private:
		float GetWindowRatio(const tm& startTime, const tm& endTime, const tm& currentTime);

		
		const std::tm _midday = { 0, 0, 12, 0, 0, 0, 0, 0, 0 };
		const std::tm _midnightAM = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		const std::tm _midnightPM = { 0, 0, 24, 0, 0, 0, 0, 0, 0 };
	};
}