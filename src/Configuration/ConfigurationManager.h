#pragma once

#include <PCH.h>
#include "ShaderSettings.h"
#include "Weather.h"
#include "Location.h"
#include "TODinfo.h"
#include "Helpers/Location.h"
#include "Helpers/Weather.h"
#include "Helpers/Math.h"
#include "FeatureSettings.h"

namespace Configuration
{

	class ConfigurationManager
	{
	public:
		ShaderSettings CurrentConfig;  // Generated config to be used by features

		ShaderSettings DefaultSettings;                           // General config settings
		std::vector<std::shared_ptr<Weather>> WeatherSettings;    // All weather configs
		std::vector<std::shared_ptr<Location>> LocationSettings;  // All location configs including embedded weather configs

		std::shared_ptr<Location> MatchingCurrentLocationSettings;  // Matching Location
		std::shared_ptr<Weather> MatchingCurrentWeatherSettings;    // Matching current Weather
		std::shared_ptr<Weather> MatchingOutgoingWeatherSettings;   // Matching outgoing weather

		static ConfigurationManager* GetSingleton()
		{
			static ConfigurationManager cm;
			return &cm;
		}

		void Load(json& o_json);
		void Save(json& o_json);
		void Update();

	private:
		RE::FormID _currentWeather = 0;
		RE::FormID _outgoingWeather = 0;
		RE::FormID _currentLocation = 0;
		float _weatherTransition = 1.0f;

		void ApplyCurrentConfig();
		void FindMatchingLocationAndWeathers(RE::FormID newLocation, RE::FormID currentWeather, RE::FormID outgoingWeather);

		void ApplyWeatherTransition(ShaderSettings& targetSettings, const ShaderSettings& outgoingSettings, const ShaderSettings& currentSettings, const float transition);

		void OverrideConfig(ShaderSettings& targetSettings, ShaderSettings& newSettings);
	};
}