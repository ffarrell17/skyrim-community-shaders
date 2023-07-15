#pragma once

#include <PCH.h>
#include "FeatureSettingsCollection.h"
#include "Weather/WeathersCollection.h"
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

		static ConfigurationManager* GetSingleton()
		{
			static ConfigurationManager cm;
			return &cm;
		}

		FeatureSettingsCollection CurrentSettings;	// Generated config to be used by features
		FeatureSettingsCollection GeneralSettings;	// Basic General config settings
		
		WeathersCollection WeatherSettings;			// Collection of all weather settings
		
		std::shared_ptr<Weather> CurrentWeatherSettings;   // Matching current Weather
		std::shared_ptr<Weather> OutgoingWeatherSettings;  // Matching outgoing weather

		ConfigurationManager();

		void Load(json& o_json);
		void Save(json& o_json);
		void Update(bool force = false);

		bool UseWeatherOverrides = false;
	private:
		RE::FormID _currentWeather = 0;
		RE::FormID _outgoingWeather = 0;
		float _weatherTransition = 1.0f;

		void ApplyCurrentConfig();

		void ApplyWeatherTransition(FeatureSettingsCollection& targetSettings, const FeatureSettingsCollection& outgoingSettings, const FeatureSettingsCollection& currentSettings, const float transition);

		void OverrideConfig(FeatureSettingsCollection& targetSettings, FeatureSettingsCollection& newSettings, bool isConfigOverride = false);
	};
}