#include "WeathersCollection.h"
#include "Configuration/ConfigurationManager.h"

using namespace Configuration;

WeathersCollection::WeathersCollection()
{
	WeatherDefaultSettings = std::make_shared<Weather>(true);
}

void WeathersCollection::Load(std::map<Feature*, json>& featureConfigMap)//(json& o_json)
{
	CustomWeatherSettings.clear();

	WeatherDefaultSettings->Load(featureConfigMap);

	json weathersJson;
	std::string configPath = "Data\\SKSE\\Plugins\\Weathers.json";

	std::ifstream i(configPath);
	if (i.is_open()) {
		try {
			i >> weathersJson;
		} catch (const nlohmann::json::parse_error& e) {
			logger::error("Error parsing json config file ({}) : {}\n", configPath, e.what());
			return;
		}

		if (weathersJson.is_array()) {
			for (auto& weatherO : weathersJson) {
				CustomWeatherSettings.push_back(std::make_shared<Weather>(weatherO, featureConfigMap));
			}
		}
	}

	
}

void WeathersCollection::Save(std::map<Feature*, json>& featureConfigMap)
{
	std::ofstream o(L"Data\\SKSE\\Plugins\\Weathers.json");

	WeatherDefaultSettings->Save(featureConfigMap);

	json weathersJson;
	if (!CustomWeatherSettings.empty()) {
		for (const auto& weather : CustomWeatherSettings) {
			json weatherJson;
			weather->Save(weatherJson, featureConfigMap);
			weathersJson.push_back(weatherJson);
		}
	}

	o << weathersJson.dump(1);
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

			auto& defaultSettingsMap = WeatherDefaultSettings->FeatureSettings[i];

			auto newFS = defaultSettingsMap.Feature->CreateNewSettings();
			newWeather->FeatureSettings.ControlNewFeature(defaultSettingsMap.Feature, newFS);
			newFS->CopyAndLink(*defaultSettingsMap.Settings);
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
			if (ImGui::Combo("Add", &selectedItem, featureNamesCStr.data(), static_cast<int>(featureNamesCStr.size()))) {
				for (int i = 0; i < WeatherDefaultSettings->FeatureSettings.size(); i++) {
					if (WeatherDefaultSettings->FeatureSettings[i].GetFeatureName() == featureNames[selectedItem]) {
						ControlNewFeatureSettings(i);
						weather->FeatureSettings.SelectedFeature = -1;
					}
				}
			}
		}

		if (weather->FeatureSettings.SelectedFeature >= 0) {
			std::string buttonLabel = "Remove:" + weather->FeatureSettings[weather->FeatureSettings.SelectedFeature].GetFeatureName();
			if (ImGui::Button(buttonLabel.c_str())) {
				weather->FeatureSettings[weather->FeatureSettings.SelectedFeature].Settings = nullptr;
				weather->FeatureSettings.SelectedFeature = -1;
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
	newDefaultFS->CopyAndLink(*configManager->GeneralSettings[featureIndex].Settings);
	newDefaultFS->ReleaseAll();

	// Add to all custom weathers also
	for (auto weather : CustomWeatherSettings) {
		auto newFS = featureSettingsToControl.Feature->CreateNewSettings();
		weather->FeatureSettings.ControlNewFeature(featureSettingsToControl.Feature, newFS);
		newFS->CopyAndLink(*newDefaultFS);
	}
}
