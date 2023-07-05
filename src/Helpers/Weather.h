#pragma once
#include "PCH.h"


namespace Helpers
{
	class Weather
	{
	public:

		static bool GetCurrentClimate(RE::TESClimate*& climate);
		static bool GetWeatherTransition(float& t);
		static bool GetCurrentWeather(RE::FormID& id);
		static bool GetOutgoingWeather(RE::FormID& id);
		static bool GetSkyMode(RE::FormID& mode);
		static int32_t GetClassification(RE::TESWeather* weather);
		static bool GetCurrentWeatherClassification(int& c);
		static bool GetOutgoingWeatherClassification(int& c);

		static std::string GetWeatherTransitionString();
		static std::string GetCurrentWeatherString();
		static std::string GetOutgoingWeatherString();
	};
}