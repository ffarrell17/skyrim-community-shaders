#include "FeatureSettingsCollection.h"
#include "ConfigurationManager.h"
#include "State.h"
#include "PCH.h"

using namespace Configuration;

FeatureSettingsCollection::FeatureSettingsCollection(FeatureSettingsType type)
{
	_type = type;
	ResetSettings();
}

void Configuration::FeatureSettingsCollection::ResetSettings()
{
	clear();
	for (const auto& feature : Feature::GetFeatureList()) {
		push_back(FeatureSettingMap(feature));
	}
}

void FeatureSettingsCollection::Load(json& o_json)
{
	ResetSettings();

	for (int i = 0; i < size(); i++) {
		
		auto& settingsMap = at(i);
		json fJson = o_json[settingsMap.GetFeatureName()];

		if (fJson.is_object()) {

			logger::trace("Parsing config for feature [{}]. Json [{}]", settingsMap.GetFeatureName(), fJson.dump());
			try {

				settingsMap.Settings = settingsMap.Feature->CreateNewSettings();
				settingsMap.Settings->FromJson(fJson);
				settingsMap.Settings->SetType(_type);

				auto allSettings = settingsMap.Settings->GetAllSettings();
				for (int j = 0; j < allSettings.size(); j++) {
					
					// If General and dont have a value reset to default
					if (_type == FeatureSettingsType::General) {
						if (!allSettings[j]->HasValue()) {
							allSettings[j]->Copy(settingsMap.Settings->GetAllSettings()[j]);
						}
					}
				}

			} 
			catch (const std::exception& ex)
			{
				logger::error("Exception parsing configuration for feature [{}]. Exception [{}]", settingsMap.GetFeatureName(), ex.what());
				throw ex;
			}
				
			if (_type == FeatureSettingsType::General) {
				if (fJson["Enabled"].is_boolean()) {
					settingsMap.Feature->Enable(fJson["Enabled"]);
				}
			}

		}
	}
}

void FeatureSettingsCollection::Save(json& o_json)
{
	for (auto it = begin(); it != end(); ++it) {
		
		if (it->Settings != nullptr) {
			json fJson;

			logger::info("Save {}", it->GetFeatureName());
			if (_type == FeatureSettingsType::General) {
				it->Settings->ToJson(fJson);
				//settingsMap.Settings->ToJsonOptionals(fJson);
			} else {
				it->Settings->ToJson(fJson);
				//settingsMap.Settings->ToJson(fJson);
			}

			o_json[it->GetFeatureName()] = fJson;
			if (_type == FeatureSettingsType::General)
				o_json[it->GetFeatureName()]["Enabled"] = it->Feature->IsEnabled();
		}
	}
}

void FeatureSettingsCollection::Draw()
{
	_updated = false;

	std::string tableName = "General Settings";
	if (_type != FeatureSettingsType::General)
		tableName = "Override Settings";

	if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
		ImGui::TableSetupColumn("##ListOfFeatures", 0, 3);
		ImGui::TableSetupColumn("##FeatureConfig", 0, 7);

		ImGui::TableNextColumn();
		if (ImGui::BeginListBox("##FeatureList", { -FLT_MIN, ImGui::GetTextLineHeight() * 50 })) {
			for (int i = 0; i < size(); i++)
				if (at(i).Settings != nullptr)
					if (ImGui::Selectable(at(i).GetFeatureName().c_str(), _selectedFeature == i))
						_selectedFeature = i;
			ImGui::EndListBox();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("##FeatureConfigFrame", { 0, ImGui::GetTextLineHeight() * 50 }, true)) {
			bool shownFeature = false;
			for (int i = 0; i < size(); i++)
				if (at(i).Settings != nullptr) {
					if (i == _selectedFeature) {

						shownFeature = true;
						bool featureEnabled = at(i).Feature->IsEnabled();

						/* std::vector<bool> settingsHasValueStates;
						if (_type == FeatureSettingsType::WeatherOverrideDefault) {
							for (auto setting : at(i).Settings->GetAllSettings()) {
								settingsHasValueStates.push_back(setting->HasValue());
							}
						}*/
	

						at(i).Settings->Draw();

						_updated = _updated || at(i).Settings->HasUpdated();

						 /* if (_type == FeatureSettingsType::WeatherOverrideDefault && _updated) {
						
							for (int j = 0; j < settingsHasValueStates.size(); j++) {
								bool newHasValue = at(i).Settings->GetAllSettings()[j]->HasValue();
								if (newHasValue != settingsHasValueStates[j]) {

									for (auto weather : ConfigurationManager::GetSingleton()->WeatherSettings.CustomWeatherSettings) {
										if (newHasValue)
											weather->FeatureSettings[i].Settings->GetAllSettings()[j]->Copy(at(i).Settings->GetAllSettings()[j]);
										else
											weather->FeatureSettings[i].Settings->GetAllSettings()[j]->Release();
									}

								}
							}
						
						
						}*/
	

						at(i).Feature->Enable(featureEnabled);
					}
				}
			if (!shownFeature)
				ImGui::TextDisabled("Please select a feature on the left.");
		}
		ImGui::EndChild();

		ImGui::EndTable();
	}
}

void FeatureSettingsCollection::ControlNewFeature(Feature* feature, std::shared_ptr<FeatureSettings> newSettings)
{
	newSettings->SetType(_type);

	for (auto settingsMapIt = begin(); settingsMapIt != end(); ++settingsMapIt) {
		if (settingsMapIt->Feature == feature) {
			settingsMapIt->Settings = newSettings;
			break;
		}
	}
}

bool FeatureSettingsCollection::HasUpdated()
{
	return _updated;
}

void Configuration::FeatureSettingsCollection::ResetUpdatedState()
{
	_updated = false;
}
