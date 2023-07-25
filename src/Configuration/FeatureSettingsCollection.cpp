#include "FeatureSettingsCollection.h"
#include "ConfigurationManager.h"
#include "State.h"
#include "PCH.h"
#include <IconsMaterialDesign.h>

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

void FeatureSettingsCollection::Load(std::map<Feature*, json>& featureConfigMap)  //(json& o_json)
{
	ResetSettings();

	for (int i = 0; i < size(); i++) {
		auto& settingsMap = at(i);

		if (!featureConfigMap[settingsMap.Feature].is_null() && featureConfigMap[settingsMap.Feature].is_object()) {
			json& fJson = featureConfigMap[settingsMap.Feature];

			try {
					
				settingsMap.Settings = settingsMap.Feature->CreateNewSettings();
				settingsMap.Settings->SetType(_type);

				settingsMap.Settings->ReleaseAll();

				settingsMap.Settings->FromJson(fJson);
				settingsMap.Settings->SetType(_type);

				if (_type == FeatureSettingsType::WeatherOverrideDefault) {
					settingsMap.Settings->Link(*ConfigurationManager::GetSingleton()->GeneralSettings[i].Settings);
				} else if (_type == FeatureSettingsType::WeatherOverride) {
					settingsMap.Settings->Link(*ConfigurationManager::GetSingleton()->WeatherSettings.WeatherDefaultSettings->FeatureSettings[i].Settings);
				}
				
				// If General and dont have a value reset to default
				if (_type == FeatureSettingsType::General) {
					auto allSettings = settingsMap.Settings->GetAllSettings();
					for (int j = 0; j < allSettings.size(); j++) {
						if (!allSettings[j]->HasValue()) {
							allSettings[j]->Copy(settingsMap.Settings->GetAllSettings()[j]);
						}
					}
				}				

			} catch (const std::exception& ex) {
				logger::error("Exception parsing configuration for feature [{}]. Exception [{}]", settingsMap.GetFeatureName(), ex.what());
				throw ex;
			}
		} else if (_type == FeatureSettingsType::General) {
			settingsMap.Settings = settingsMap.Feature->CreateNewSettings();
			settingsMap.Settings->SetType(_type);
		}
	}
}

void removeNullElements(json& jsonObj)
{
	if (jsonObj.is_object()) {
		for (auto it = jsonObj.begin(); it != jsonObj.end();) {
			if (it.value().is_null()) {
				it = jsonObj.erase(it);
			} else {
				++it;
			}
		}
	}
}

void FeatureSettingsCollection::Save(std::map<Feature*, json>& featureConfigMap)
{
	for (auto it = begin(); it != end(); ++it) {
		
		if (it->Settings != nullptr) {
			json fJson;

			it->Settings->ToJson(fJson);
			if (_type != FeatureSettingsType::General) {
				removeNullElements(fJson);
			}

			featureConfigMap[it->Feature] = fJson;
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
					if (ImGui::Selectable(at(i).GetFeatureName().c_str(), SelectedFeature == i))
						SelectedFeature = i;
			ImGui::EndListBox();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("##FeatureConfigFrame", { 0, ImGui::GetTextLineHeight() * 50 }, true)) {
			bool shownFeature = false;
			for (int i = 0; i < size(); i++)
				if (at(i).Settings != nullptr) {
					if (i == SelectedFeature) {
						shownFeature = true;
						auto& settingsMap = at(i);

						if (_type == FeatureSettingsType::General && settingsMap.Feature->AllowEnableDisable()) {
							std::string label = "Enable " + settingsMap.GetFeatureName();
							bool enabled = settingsMap.Feature->IsEnabled();
							if (ImGui::Checkbox(label.c_str(), &enabled))
								settingsMap.Feature->Enable(enabled);
						}

						settingsMap.Settings->TODUpdateAll();
						settingsMap.Settings->Draw();

						_updated = _updated || settingsMap.Settings->HasUpdated();
					}
				}
			if (!shownFeature)
				ImGui::TextDisabled("Please select a feature on the left.");
		}
		ImGui::EndChild();

		ImGui::EndTable();
	}
}

void Configuration::FeatureSettingsCollection::ControlNewFeature(FeatureSettingMap& featureSettingsMap)
{
	ControlNewFeature(featureSettingsMap.Feature, featureSettingsMap.Settings);
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
