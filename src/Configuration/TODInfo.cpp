#include "TODInfo.h"

using namespace Configuration;

void TODInfo::Update()
{
	Valid = false;

	RE::TESClimate* climate = nullptr;

	if (!Helpers::Time::GetDateTime(Time))
		return;

	if (!Helpers::Weather::GetCurrentClimate(climate))
		return;

	Valid = true;

	Exterior = !Helpers::Location::IsValidInterior();

	SunriseBeginTime = climate->timing.sunrise.GetBeginTime();
	SunriseEndTime = climate->timing.sunrise.GetEndTime();
	SunsetBeginTime = climate->timing.sunset.GetBeginTime();
	SunsetEndTime = climate->timing.sunset.GetEndTime();

	if (Helpers::Time::TimeCmp(Time, SunriseBeginTime) <= 0) {
		TimePeriodType = TimePeriod::NightToSunriseStart;
		TimePeriodPercentage = GetWindowRatio(_midnightAM, SunriseBeginTime, Time);

	} else if (Helpers::Time::TimeCmp(Time, SunriseBeginTime) == 1 && Helpers::Time::TimeCmp(Time, SunriseEndTime) <= 0) {
		TimePeriodType = TimePeriod::SunriseStartToSunriseEnd;
		TimePeriodPercentage = GetWindowRatio(SunriseBeginTime, SunriseEndTime, Time);

	} else if (Helpers::Time::TimeCmp(Time, SunriseEndTime) == 1 && Helpers::Time::TimeCmp(Time, _midday) <= 0) {
		TimePeriodType = TimePeriod::SunriseEndToDay;
		TimePeriodPercentage = GetWindowRatio(SunriseEndTime, _midday, Time);

	} else if (Helpers::Time::TimeCmp(Time, _midday) == 1 && Helpers::Time::TimeCmp(Time, SunsetBeginTime) <= 0) {
		TimePeriodType = TimePeriod::DayToSunsetStart;
		TimePeriodPercentage = GetWindowRatio(_midday, SunsetBeginTime, Time);

	} else if (Helpers::Time::TimeCmp(Time, SunsetBeginTime) == 1 && Helpers::Time::TimeCmp(Time, SunsetEndTime) <= 0) {
		TimePeriodType = TimePeriod::SunsetStartToSunsetEnd;
		TimePeriodPercentage = GetWindowRatio(SunsetBeginTime, SunsetEndTime, Time);

	} else {
		TimePeriodType = TimePeriod::SunsetEndToNight;
		TimePeriodPercentage = GetWindowRatio(SunsetEndTime, _midnightPM, Time);
	}
}

std::string Configuration::TODInfo::GetTimePeriodStr()
{
	if (Valid) {
		switch (TimePeriodType) {
		case SunriseStartToSunriseEnd:
			return "Dawn To Sunrise";
		case DayToSunsetStart:
			return "Day To Sunset";
		case SunsetEndToNight:
			return "Dusk To Night";
		case NightToSunriseStart:
			return "Night To Dawn";
		case SunriseEndToDay:
			return "Sunrise To Day";
		case SunsetStartToSunsetEnd:
			return "Sunset To Dusk";
		}
	}
	return "-";
}

float TODInfo::GetWindowRatio(const tm& startTime, const tm& endTime, const tm& currentTime)
{
	float windowStartMinutes = (float)Helpers::Time::TimeToMinutes(startTime);
	float windowEndMinutes = (float)Helpers::Time::TimeToMinutes(endTime);
	float currentTimeMinutes = (float)Helpers::Time::TimeToMinutes(currentTime);

	return (currentTimeMinutes - windowStartMinutes) / (windowEndMinutes - windowStartMinutes);
}