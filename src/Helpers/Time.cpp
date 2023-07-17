#include "Time.h"
using namespace Helpers;

bool Time::GetDateTime(tm& time)
{
	if (const auto& calendar = RE::Calendar::GetSingleton(); calendar) {

		time = calendar->GetTime();
		time.tm_min = GetMinutes(calendar);
		return true;
	}

	return false;
}

bool Time::GetTime(tm& time)
{
	if (const auto& calendar = RE::Calendar::GetSingleton(); calendar) {
		
		time.tm_hour = static_cast<int>(calendar->GetHour());
		time.tm_min = GetMinutes(calendar);
		return true;
	}

	return false;
}

int Time::GetMinutes(RE::Calendar* calendar)
{
	return static_cast<int>(calendar->gameHour->value * 60.0f) % 60;
}

// -1 less <; 0 equal =; 1 more >
int Time::TimeCmp(const tm& tm1, const tm& tm2)
{
	int time1Mins = TimeToMinutes(tm1);
	int time2Mins = TimeToMinutes(tm2);

	if (time1Mins < time2Mins)
		return -1;
	else if (time1Mins == time2Mins)
		return 0;
	return 1;
}

int Time::TimeToMinutes(const tm& time)
{
	return (time.tm_hour * 60) + time.tm_min;
}

int Helpers::Time::TimeToSeconds(const tm& time)
{
	return (time.tm_hour * 60 * 60) + (time.tm_min * 60) + time.tm_sec;
}

void Helpers::Time::TimeToString(const tm& time, char* buffer, std::size_t bufferSize)
{
	strftime(buffer, bufferSize, "%H:%M", &time);
}
