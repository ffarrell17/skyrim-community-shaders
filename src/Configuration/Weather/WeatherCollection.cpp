#include "WeathersCollection.h"
#include "Configuration/ConfigurationManager.h"

using namespace Configuration;

WeathersCollection::WeathersCollection()
{
	WeatherDefaultSettings = std::make_shared<Weather>(true);
}

void WeathersCollection::Load(json& o_json)
{
	CustomWeatherSettings.clear();

	if (o_json["Default"].is_object()) {
		WeatherDefaultSettings->Load(o_json["Default"]);
	}

	if (o_json["Overrides"].is_array()) {
		for (auto& weatherO : o_json["Overrides"]) {
			CustomWeatherSettings.push_back(std::make_shared<Weather>(weatherO));
		}
	}
}

void WeathersCollection::Save(json& o_json)
{
	json wdefault;
	WeatherDefaultSettings->Save(wdefault);
	o_json["Default"] = wdefault;

	if (!CustomWeatherSettings.empty()) {
		for (const auto& weather : CustomWeatherSettings) {
			json weatherJson;
			weather->Save(weatherJson);
			o_json["Overrides"].push_back(weatherJson);
		}
	}
}

void Configuration::WeathersCollection::Draw()
{
	std::vector<std::shared_ptr<Weather>> allWeathers = CustomWeatherSettings;
	allWeathers.insert(allWeathers.begin(), WeatherDefaultSettings);
	int count = static_cast<int>(allWeathers.size());

	if (ImGui::Button("Add Weather")) {
		AddNewWeather();
		_selectedWeatherIndex = count - 1;
		logger::trace("Added new weather config");
	}

	if (_selectedWeatherIndex <= 0) ImGui::BeginDisabled();
	if (ImGui::Button("Remove Weather")) {
		// TODO: add warning
		RemoveWeather(allWeathers[_selectedWeatherIndex]);
		_selectedWeatherIndex = -1;
	}
	if (_selectedWeatherIndex <= 0) ImGui::EndDisabled();

	// Display the list box of weather items
	std::vector<const char*> weatherNames;
	for (const auto& weather : allWeathers) {
		weatherNames.push_back(weather->Name.c_str());
	}

	int preSelectedIndex = _selectedWeatherIndex;
	if (ImGui::ListBox("Weather List", &_selectedWeatherIndex, weatherNames.data(), count)) {
		if (preSelectedIndex == _selectedWeatherIndex)
			preSelectedIndex = -1;  //deselect
	}

	ImGui::Spacing();

	ImGui::Indent();
	// Display the selected weather item's settings
	if (_selectedWeatherIndex >= 0 && _selectedWeatherIndex < count) {
		DrawWeather(allWeathers[_selectedWeatherIndex]);
	}
	ImGui::Unindent();
}

void Configuration::WeathersCollection::AddNewWeather()
{
	auto newWeather = std::make_shared<Configuration::Weather>();

	for (int i = 0; i < WeatherDefaultSettings->FeatureSettings.size(); i++) {
		if (WeatherDefaultSettings->FeatureSettings[i].Settings != nullptr) {

			auto newFS = WeatherDefaultSettings->FeatureSettings[i].Feature->CreateNewSettings();
			newFS->Copy(*WeatherDefaultSettings->FeatureSettings[i].Settings);
			newWeather->FeatureSettings.ControlNewFeature(WeatherDefaultSettings->FeatureSettings[i].Feature, newFS);
		}
	}

	CustomWeatherSettings.push_back(newWeather);
}

void Configuration::WeathersCollection::RemoveWeather(std::shared_ptr<Weather> weather)
{
	CustomWeatherSettings.erase(std::remove(CustomWeatherSettings.begin(), CustomWeatherSettings.end(), weather), CustomWeatherSettings.end());
}

std::shared_ptr<Weather> WeathersCollection::GetWeatherFromId(RE::FormID id)
{
	for (const auto& weather : CustomWeatherSettings) {
		for (const RE::FormID& weatherId : weather->Ids) {
			if (weatherId == id) {
				return weather;
			}
		}
	}

	return WeatherDefaultSettings;
}

void Configuration::WeathersCollection::ResetUpdatedStates()
{
	for (const auto& weather : CustomWeatherSettings) {
		weather->ResetUpdatedState();
	}
	WeatherDefaultSettings->ResetUpdatedState();
}

bool Configuration::WeathersCollection::HasAnyUpdatedWeatherIds()
{
	for (const auto& weather : CustomWeatherSettings) {
		if (weather->HasUpdatedIds())
			return true;
	}
	return WeatherDefaultSettings->HasUpdatedIds();
}



void Configuration::WeathersCollection::DrawWeather(std::shared_ptr<Weather> weather)
{
	bool isDeafult = weather == WeatherDefaultSettings;

	if (isDeafult) {
		// Creating a combo box for all feature names that this config doesn't have
		// If the user selects one the default config is added

		std::vector<std::string> featureNames;
		std::vector<const char*> featureNamesCStr;

		for (auto& settingsMap : WeatherDefaultSettings->FeatureSettings) {
			if (settingsMap.Settings == nullptr) {
				featureNames.push_back(settingsMap.GetFeatureName());
			}
		}

		for (const std::string& str : featureNames) {
			featureNamesCStr.push_back(str.c_str());
		}

		if (!featureNames.empty()) {
			int selectedItem = -1;
			if (ImGui::Combo("Add Feature Override", &selectedItem, featureNamesCStr.data(), static_cast<int>(featureNamesCStr.size()))) {
				for (int i = 0; i < WeatherDefaultSettings->FeatureSettings.size(); i++) {
					if (WeatherDefaultSettings->FeatureSettings[i].GetFeatureName() == featureNames[selectedItem]) {
						ControlNewFeatureSettings(i);
						//_selectedFeature = i;
					}
				}
			}
		}
	}

	weather->Draw();
}

void Configuration::WeathersCollection::ControlNewFeatureSettings(int featureIndex)
{
	auto configManager = Configuration::ConfigurationManager::GetSingleton();
	auto featureSettingsToControl = WeatherDefaultSettings->FeatureSettings[featureIndex];

	auto newDefaultFS = featureSettingsToControl.Feature->CreateNewSettings();
	WeatherDefaultSettings->FeatureSettings.ControlNewFeature(featureSettingsToControl.Feature, newDefaultFS);
	newDefaultFS->Copy(*configManager->GeneralSettings[featureIndex].Settings);

	// Add to all custom weathers also
	for (auto weather : CustomWeatherSettings) {
		auto newFS = featureSettingsToControl.Feature->CreateNewSettings();
		weather->FeatureSettings.ControlNewFeature(featureSettingsToControl.Feature, newFS);
		newFS->Copy(*newDefaultFS);
	}

	/*
	auto* configManager = Configuration::ConfigurationManager::GetSingleton();

	auto featureSettingsToControl = WeatherDefaultSettings->Settings.Settings[featureIndex];

	auto newFS = configManager->DefaultSettings.Settings[featureIndex].Settings->Clone();
	WeatherDefaultSettings->Settings.Settings[featureIndex].Settings = newFS;

	// Add to all custom weathers also
	for (auto weather : CustomWeatherSettings) {
		weather->Settings.Settings[featureIndex].Settings = newFS->Clone();
	}

	*/
}
