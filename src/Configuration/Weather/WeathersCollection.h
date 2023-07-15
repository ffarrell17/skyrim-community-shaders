#pragma once
#include <PCH.h>
#include "Weather.h"

namespace Configuration
{
	struct WeathersCollection
	{
		std::shared_ptr<Weather> WeatherDefaultSettings;              // Default weather override config for when no matching weather is present
		std::vector<std::shared_ptr<Weather>> CustomWeatherSettings;  // All user created configs

		WeathersCollection();

		void Load(json& o_json);
		void Save(json& o_json);
		void Draw();

		void AddNewWeather();
		void RemoveWeather(std::shared_ptr<Weather> weather);

		std::shared_ptr<Weather> GetWeatherFromId(RE::FormID id);

		void ResetUpdatedStates();
		bool HasAnyUpdatedWeatherIds();

	private:
		int _selectedWeatherIndex = -1;

		void DrawWeather(std::shared_ptr<Weather> weather);
		void ControlNewFeatureSettings(int featureIndex);
	};
}