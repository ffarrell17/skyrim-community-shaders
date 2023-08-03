#include "ConfigurationManager.h"
#include "State.h"

using namespace Configuration;

Configuration::ConfigurationManager::ConfigurationManager() :
	GeneralSettings(FeatureSettingsType::General),
	CurrentSettings(FeatureSettingsType::WeatherOverride)
{ 
	
}

void ConfigurationManager::Load(json& general_json)
{
	logger::trace("Loading individual feature json configs");
	std::map<Feature*, json> featureConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {

		json settings;
		std::string configPath = "Data\\SKSE\\Plugins\\" + feature->GetShortName() + ".json";

		std::ifstream i(configPath);
		if (!i.is_open()) {
			// config doesn't exist

			if (!general_json[feature->GetName()].is_null())
				settings["General"] = general_json[feature->GetName()];
			
		} else {
			try {
				i >> settings;
			} catch (const nlohmann::json::parse_error& e) {
				logger::error("Error parsing json config file ({}) : {}\n", configPath, e.what());
				//continue;
			}
		}

		featureConfigMap.insert(std::make_pair(feature, settings));
	}

	logger::trace("Loading General Settings");
	std::map<Feature*, json> generalFeatureConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {
		json generalSettings;
		if (!featureConfigMap[feature].is_null() && !featureConfigMap[feature]["General"].is_null()) {
			generalSettings = featureConfigMap[feature]["General"];
		}
		generalFeatureConfigMap.insert(std::make_pair(feature, generalSettings));
	}
	GeneralSettings.Load(generalFeatureConfigMap);

	logger::trace("Loading Weather Settings");
	WeatherSettings.Load(featureConfigMap);

	logger::trace("Loaded. Updating applied config");
	Update(true);

	for (int i = 0; i < CurrentSettings.size(); i++) {
		for (int j = 0; j < GeneralSettings[i].Settings->GetAllSettings().size(); j++) {
			GeneralSettings[i].Settings->GetAllSettings()[j]->SetAsOverrideVal(CurrentSettings[i].Settings->GetAllSettings()[j]);
		}
	}
}

void ConfigurationManager::Save()
{
	std::map<Feature*, json> featureConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {
		featureConfigMap[feature] = json();
	}

	std::map<Feature*, json> generalConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {
		generalConfigMap[feature] = json();
	}

	GeneralSettings.Save(generalConfigMap);

	for (Feature* feature : Feature::GetFeatureList()) {
		featureConfigMap[feature]["General"] = generalConfigMap[feature];
	}

	WeatherSettings.Save(featureConfigMap);

	for (Feature* feature : Feature::GetFeatureList()) {

		std::string configPath = "Data\\SKSE\\Plugins\\" + feature->GetShortName() + ".json";
		std::ofstream o(configPath);
		o << featureConfigMap[feature].dump(1);
	}
}

void ConfigurationManager::Update(bool force)
{
	Configuration::TODInfo::GetSingleton()->Update();
	force = force;
	RE::FormID currentWeather = 0;
	RE::FormID outgoingWeather = 0;
	float weatherTransition = 0;
	bool inWeatherTransition = false;

	/* if (UseWeatherOverrides) {
		Helpers::Weather::GetCurrentWeather(currentWeather);
		Helpers::Weather::GetOutgoingWeather(outgoingWeather);
		Helpers::Weather::GetWeatherTransition(weatherTransition);

		inWeatherTransition = weatherTransition != 1.0f;

		if (currentWeather != _currentWeather)
			logger::trace("Weather Changed [{}]", currentWeather);
		if (weatherTransition == 0)
			logger::trace("Weather transition begun");
		if (weatherTransition == 1.0f && _weatherTransition != 1.0f)
			logger::trace("Weather transition ended");

		if (currentWeather == _currentWeather &&
			weatherTransition == _weatherTransition && 
			!force) {
			// No change needed unless user updated the settings

			bool updated = GeneralSettings.HasUpdated();

			if (CurrentWeatherSettings) {
				updated = updated || CurrentWeatherSettings->HasUpdated();
			}

			if (OutgoingWeatherSettings && inWeatherTransition) {
				updated = updated || OutgoingWeatherSettings->HasUpdated();
			}

			// Check any weather settings have changed ids as matching settings may have changed
			updated = updated || WeatherSettings.HasAnyUpdatedWeatherIds();

			if (!updated)
				return;

			logger::trace("Detected relevant settings change");
		}

		CurrentWeatherSettings = WeatherSettings.GetWeatherFromId(currentWeather);
		OutgoingWeatherSettings = outgoingWeather > 0 ? WeatherSettings.GetWeatherFromId(outgoingWeather) : nullptr;

	} else {
		if (!GeneralSettings.HasUpdated() && !force)
			return;
	}

	logger::trace("Updating Current Configs");*/

	if (UseWeatherOverrides) {
		Helpers::Weather::GetCurrentWeather(currentWeather);
		Helpers::Weather::GetOutgoingWeather(outgoingWeather);
		Helpers::Weather::GetWeatherTransition(weatherTransition);

		inWeatherTransition = weatherTransition != 1.0f;

		CurrentWeatherSettings = WeatherSettings.GetWeatherFromId(currentWeather);
		OutgoingWeatherSettings = outgoingWeather > 0 ? WeatherSettings.GetWeatherFromId(outgoingWeather) : nullptr;
	}

	// Reset all updated statess as the menu updates at a slower rate

	GeneralSettings.ResetUpdatedState();
	WeatherSettings.ResetUpdatedStates();

	// Config needs to change

	try {

		if (!UseWeatherOverrides || (!CurrentWeatherSettings && !OutgoingWeatherSettings) ||
			(OutgoingWeatherSettings && !inWeatherTransition)) {
			// Not using weathers, no weather overrides found 
			// or only found outgoing after its already transitioned
			// Reset to default

			OverrideConfig(CurrentSettings, GeneralSettings);
		} else {
			// Found either current or outgoing (whilst tranisioning) weather

			if (!inWeatherTransition || CurrentWeatherSettings == OutgoingWeatherSettings) {
				// Either not transitioning or current and outgoing are the same config
				// Apply default then apply current over the top if exists

				OverrideConfig(CurrentSettings, GeneralSettings);
				if (CurrentWeatherSettings)
					OverrideConfig(CurrentSettings,CurrentWeatherSettings->FeatureSettings, true);

			} else {
				// In a weather transition with either a current or outgoing override or both
				// Create a current and outgoing weather config then merge together

				// Current weather settings
				FeatureSettingsCollection currentWeatherSettings(FeatureSettingsType::WeatherOverride);
				OverrideConfig(currentWeatherSettings, GeneralSettings);
				if (CurrentWeatherSettings)
					OverrideConfig(currentWeatherSettings, CurrentWeatherSettings->FeatureSettings);

				// Outgoing weather settings
				FeatureSettingsCollection outgoingWeatherSettings(FeatureSettingsType::WeatherOverride);
				OverrideConfig(outgoingWeatherSettings, GeneralSettings);
				if (OutgoingWeatherSettings)
					OverrideConfig(outgoingWeatherSettings, OutgoingWeatherSettings->FeatureSettings);

				// Merge into CurrentConfig
				ApplyWeatherTransition(CurrentSettings, outgoingWeatherSettings, currentWeatherSettings, weatherTransition);
			}
		}
	
	} catch (const std::exception& ex) {
		logger::error("Exception updating config on weather/location change. Exception [{}]", ex.what());
		logger::error("CurrentWeatherId: [{}]; OutgoingWeatherId: [{}]; WeatherTransition: [{}]; ", currentWeather, outgoingWeather, weatherTransition);
	}

	ApplyCurrentConfig();

	_currentWeather = currentWeather;
	_outgoingWeather = outgoingWeather;
	_weatherTransition = weatherTransition;
}

#pragma warning(suppress: 4100)
void ConfigurationManager::OverrideConfig(FeatureSettingsCollection& targetSettings, FeatureSettingsCollection& newSettings, bool isConfigOverride)
{
	for (int i = 0; i < targetSettings.size(); i++) {

		if (newSettings[i].Settings == nullptr)
			continue;

		if (targetSettings[i].Settings == nullptr)
			targetSettings[i].Settings = targetSettings[i].Feature->CreateNewSettings();

		auto targetSettingList = targetSettings[i].Settings->GetAllSettings();
		auto newSettingList = newSettings[i].Settings->GetAllSettings();

		for (int j = 0; j < targetSettingList.size(); j++) {
			if (newSettingList[j]->HasValue()) {

				targetSettingList[j]->Copy(newSettingList[j]);
			}
		}
	}
}

#pragma warning(suppress: 4100)
void ConfigurationManager::ApplyWeatherTransition(FeatureSettingsCollection& targetSettings, const FeatureSettingsCollection& outgoingSettings, const FeatureSettingsCollection& currentSettings, const float transition)
{
	/* for (int i = 0; i < targetSettings.Settings.size(); i++) {
		targetSettings.Settings[i].Settings->Lerp(outgoingSettings.Settings[i].Settings, currentSettings.Settings[i].Settings, transition);
	}*/

	for (int i = 0; i < targetSettings.size(); i++) {
		if (outgoingSettings[i].Settings == nullptr || currentSettings[i].Settings == nullptr)
			continue;

		if (targetSettings[i].Settings == nullptr)
			targetSettings[i].Settings = targetSettings[i].Feature->CreateNewSettings();

		auto targetSettingList = targetSettings[i].Settings->GetAllSettings();
		auto currentSettingList = currentSettings[i].Settings->GetAllSettings();
		auto outgoingSettingList = outgoingSettings[i].Settings->GetAllSettings();

		for (int j = 0; j < targetSettingList.size(); j++) {
			if (currentSettingList[j]->HasValue() && outgoingSettingList[j]->HasValue()) {

				targetSettingList[j]->SetAsLerp(outgoingSettingList[j], currentSettingList[j], transition);
			}
		}
	}
}

void ConfigurationManager::ApplyCurrentConfig()
{
	for (auto& settingsMap : CurrentSettings) {
		settingsMap.Feature->SetSettings(*settingsMap.Settings);
	}
}