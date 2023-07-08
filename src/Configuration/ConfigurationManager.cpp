#include "ConfigurationManager.h"
#include "State.h"

using namespace Configuration;

void ConfigurationManager::Load(json& o_json)
{
	DefaultSettings.Load(o_json);

	WeatherSettings.clear();
	if (o_json["Weathers"].is_array()) {
		for (auto& weatherO : o_json["Weathers"]) {
			WeatherSettings.push_back(std::make_shared<Weather>(weatherO));
		}
	}

	LocationSettings.clear();
	if (o_json["Locations"].is_object()) {
		for (auto& locationO : o_json["Locations"]) {
			LocationSettings.push_back(std::make_shared<Location>(locationO));
		}
	}
}

void ConfigurationManager::Save(json& o_json)
{
	DefaultSettings.Save(o_json, true);

	if (!WeatherSettings.empty()) {
		for (const auto& weather : WeatherSettings) {
			json weatherJson;
			weather->Save(weatherJson);
			o_json["Weathers"].push_back(weatherJson);
		}
	}

	// Save location settings
	if (!LocationSettings.empty()) {
		for (const auto& location : LocationSettings) {
			json locationJson;
			location->Save(locationJson);
			o_json["Locations"].push_back(locationJson);
		}
	}
}

void ConfigurationManager::Update()
{
	Configuration::TODInfo::GetSingleton().Update();

	RE::FormID currentWeather = 0;
	RE::FormID outgoingWeather = 0;
	RE::FormID newLocation = 0;
	float weatherTransition = 0;

	Helpers::Weather::GetCurrentWeather(currentWeather);
	Helpers::Weather::GetOutgoingWeather(outgoingWeather);
	Helpers::Location::GetWorldSpaceID(newLocation);
	Helpers::Weather::GetWeatherTransition(weatherTransition);

	bool inWeatherTransition = weatherTransition != 1.0f;

	if (newLocation != _currentLocation)
		logger::trace("Location Changed [{}]", newLocation);
	if (currentWeather != _currentWeather)
		logger::trace("Weather Changed [{}]", currentWeather);
	if (weatherTransition == 0)
		logger::trace("Weather transition begun");
	if (weatherTransition == 1.0f && _weatherTransition != 1.0f)
		logger::trace("Weather transition ended");

	if (newLocation == _currentLocation &&
		currentWeather == _currentWeather &&
		weatherTransition == _weatherTransition) {
		// No change needed unless user updated the settings

		bool updated = DefaultSettings.HasUpdated();

		if (MatchingCurrentWeatherSettings) {
			updated = updated || MatchingCurrentWeatherSettings->HasUpdated();
		}

		if (MatchingOutgoingWeatherSettings && inWeatherTransition) {
			updated = updated || MatchingOutgoingWeatherSettings->HasUpdated();
		}

		// Check any weather settings have changed ids as matching settings may have changed
		for (const auto& weather : WeatherSettings) {
			if (weather->HasUpdatedIds()) {
				for (const auto& weatherId : weather->Ids) {
					if (weatherId == 0)
						continue;
					updated = updated || (weatherId == currentWeather || weatherId == outgoingWeather);
				}
			}
		}

		if (!updated)
			return;

		logger::trace("Detected relevant settings change");	
	}

	logger::trace("Updating Current Configs");

	// Reset all updated statess as the menu updates at a slower rate
	
	DefaultSettings.ResetUpdatedState();
	for (const auto& weather : WeatherSettings) {
		weather->ResetUpdatedState();
	}

	// Config needs to change
	// TODO: Change to only update on transition every e.g. 1/10th?

	try {

		// Check for matching locations and weathers
		FindMatchingLocationAndWeathers(newLocation, currentWeather, outgoingWeather);

		if ((!MatchingCurrentWeatherSettings && !MatchingOutgoingWeatherSettings) ||
			(MatchingOutgoingWeatherSettings && !inWeatherTransition)) {
			// No weather overrides found or only found outgoing after its already transitioned
			// Reset to default

			OverrideConfig(CurrentConfig, DefaultSettings);
		} else {
			// Found either current or outgoing (whilst tranisioning) weather

			if (!inWeatherTransition || MatchingCurrentWeatherSettings == MatchingOutgoingWeatherSettings) {
				// Either not transitioning or current and outgoing are the same config
				// Apply default then apply current over the top if exists

				OverrideConfig(CurrentConfig, DefaultSettings);
				if (MatchingCurrentWeatherSettings)
					OverrideConfig(CurrentConfig, MatchingCurrentWeatherSettings->Settings);

			} else {
				// In a weather transition with either a current or outgoing override or both
				// Create a current and outgoing weather config then merge together

				// Current weather settings
				ShaderSettings currentWeatherSettings;
				OverrideConfig(currentWeatherSettings, DefaultSettings);
				if (MatchingCurrentWeatherSettings)
					OverrideConfig(currentWeatherSettings, MatchingCurrentWeatherSettings->Settings);

				// Outgoing weather settings
				ShaderSettings outgoingWeatherSettings;
				OverrideConfig(outgoingWeatherSettings, DefaultSettings);
				if (MatchingOutgoingWeatherSettings)
					OverrideConfig(outgoingWeatherSettings, MatchingOutgoingWeatherSettings->Settings);

				// Merge into CurrentConfig
				ApplyWeatherTransition(CurrentConfig, outgoingWeatherSettings, currentWeatherSettings, weatherTransition);
			}
		}
	
	} catch (const std::exception& ex) {
		logger::error("Exception updating config on weather/location change. Exception [{}]", ex.what());
		logger::error("LocationId: [{}]; CurrentWeatherId: [{}]; OutgoingWeatherId: [{}]; WeatherTransition: [{}]; ", newLocation, currentWeather, outgoingWeather, weatherTransition);
	}

	ApplyCurrentConfig();

	_currentLocation = newLocation;
	_currentWeather = currentWeather;
	_outgoingWeather = outgoingWeather;
	_weatherTransition = weatherTransition;
	
}

void ConfigurationManager::ApplyWeatherTransition(ShaderSettings& targetSettings, const ShaderSettings& outgoingSettings, const ShaderSettings& currentSettings, const float transition)
{
	for (int i = 0; i < targetSettings.Settings.size(); i++) {
		targetSettings.Settings[i].Settings->Lerp(outgoingSettings.Settings[i].Settings, currentSettings.Settings[i].Settings, transition);
	}
}

void ConfigurationManager::OverrideConfig(ShaderSettings& targetSettings, ShaderSettings& newSettings)
{
	for (int i = 0; i < targetSettings.Settings.size(); i++) {

		if (newSettings.Settings[i].Settings == nullptr)
			continue;

		if (targetSettings.Settings[i].Settings == nullptr)
			targetSettings.Settings[i].Settings = Feature::GetFeatureList()[i]->CreateConfig();

		targetSettings.Settings[i].Settings->Override(newSettings.Settings[i].Settings);
	}
}

void ConfigurationManager::ApplyCurrentConfig()
{
	for (auto& settingsMap : CurrentConfig.Settings) {
		settingsMap.Feature->ApplyConfig(settingsMap.Settings);
	}
}

void ConfigurationManager::FindMatchingLocationAndWeathers(RE::FormID newLocation, RE::FormID currentWeather, RE::FormID outgoingWeather)
{
	MatchingCurrentLocationSettings = nullptr;
	MatchingCurrentWeatherSettings = nullptr;
	MatchingOutgoingWeatherSettings = nullptr;

	// Finding matching location
	for (const auto& location : LocationSettings) {
		for (const RE::FormID& locationId : location->Ids) {
			if (locationId == newLocation) {
				MatchingCurrentLocationSettings = location;
				break;
			}
		}

		if (MatchingCurrentLocationSettings)
			break;
	}

	if (MatchingCurrentLocationSettings) {
		// Finding matching weather in location

		for (const auto& weather : MatchingCurrentLocationSettings->Weathers) {
			for (const RE::FormID& weatherId : weather->Ids) {
				if (weatherId == currentWeather) {
					MatchingCurrentWeatherSettings = weather;
				} else if (weatherId == outgoingWeather) {
					MatchingOutgoingWeatherSettings = weather;
				}

				if (MatchingCurrentWeatherSettings && MatchingOutgoingWeatherSettings)
					break;
			}

			if (MatchingCurrentWeatherSettings && MatchingOutgoingWeatherSettings)
				break;
		}
	}

	if (!MatchingCurrentWeatherSettings || !MatchingOutgoingWeatherSettings) {
		// Location doesnt have both matching weathers in weather settings

		for (const auto& weather : WeatherSettings) {
			for (const RE::FormID& weatherId : weather->Ids) {
				if (weatherId == currentWeather && !MatchingCurrentWeatherSettings) {
					MatchingCurrentWeatherSettings = weather;
				} else if (weatherId == outgoingWeather && !MatchingOutgoingWeatherSettings) {
					MatchingOutgoingWeatherSettings = weather;
				}

				if (MatchingCurrentWeatherSettings && MatchingOutgoingWeatherSettings)
					break;
			}

			if (MatchingCurrentWeatherSettings && MatchingOutgoingWeatherSettings)
				break;
		}
	}

	// Dont match when there is no outgoing weather as it confuses the user
	if (MatchingOutgoingWeatherSettings && outgoingWeather == 0) {
		MatchingOutgoingWeatherSettings = nullptr;
	}
}