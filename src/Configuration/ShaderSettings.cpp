#include "ShaderSettings.h"
#include "ConfigurationManager.h"
#include "State.h"
#include "PCH.h"
#include "FeatureSettings.h"

using namespace Configuration;

ShaderSettings::ShaderSettings()
{
	ClearConfig();
}

void Configuration::ShaderSettings::ClearConfig()
{
	Settings.clear();
	for (const auto& feature : Feature::GetFeatureList()) {
		Settings.push_back(FeatureSettingMap(feature));
	}
}

void ShaderSettings::Load(json& o_json, bool isOptional)
{
	ClearConfig();

	for (auto& settingsMap : Settings) {
		
		json featureJson = o_json[settingsMap.GetFeatureName()];

		if (featureJson.is_object()) {

			logger::trace("Parsing config for feature [{}]. Json [{}]", settingsMap.GetFeatureName(), featureJson.dump());
			try {

				settingsMap.Settings = settingsMap.Feature->CreateConfig();

				if (isOptional) {
					settingsMap.Settings->FromJsonOptionals(featureJson);
				} else {
					settingsMap.Settings->FromJson(featureJson);
				}

			} 
			catch (const std::exception& ex)
			{
				logger::error("Exception parsing configuration for feature [{}]. Exception [{}]", settingsMap.GetFeatureName(), ex.what());
				throw ex;
			}
				
			if (!isOptional) {
				if (featureJson["Enabled"].is_boolean()) {
					settingsMap.Feature->Enable(featureJson["Enabled"]);
				}
			}

		}
	}
}

void ShaderSettings::Save(json& o_json, bool isOptional)
{
	for (auto& settingsMap : Settings) {
		
		if (settingsMap.Settings != nullptr) {
			json fJson;

			logger::info("Save {}", settingsMap.GetFeatureName());
			if (isOptional) {
				settingsMap.Settings->ToJsonOptionals(fJson);
			} else {
				settingsMap.Settings->ToJson(fJson);
			}

			o_json[settingsMap.GetFeatureName()] = fJson;
			if (!isOptional)
				o_json[settingsMap.GetFeatureName()]["Enabled"] = settingsMap.Feature->IsEnabled();
		}
	}
}

void ShaderSettings::Draw(std::string tabBarName, bool isOptional)
{
	_updated = false;

	if (isOptional) {
		// Creating a combo box for all feature names that this config doesn't have
		// If the user selects one the default config is added

		std::vector<std::string> featureNames;
		std::vector<const char*> featureNamesCStr;

		for (auto& settingsMap : Settings) {
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
				for (int i = 0; i < Settings.size(); i++) {
					if (Settings[i].GetFeatureName() == featureNames[selectedItem]) {
						
						Settings[i].Settings = Settings[i].Feature->CreateConfig();
						Settings[i].Settings->ResetOptionals();
						_selectedFeature = i;
					}
				}
			}
		}
	}

	if (ImGui::BeginTable(tabBarName.c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
		ImGui::TableSetupColumn("##ListOfFeatures", 0, 3);
		ImGui::TableSetupColumn("##FeatureConfig", 0, 7);

		ImGui::TableNextColumn();
		if (ImGui::BeginListBox("##FeatureList", { -FLT_MIN, -FLT_MIN })) {
			for (int i = 0; i < Settings.size(); i++)
				if (Settings[i].Settings != nullptr)
					if (ImGui::Selectable(Settings[i].GetFeatureName().c_str(), _selectedFeature == i))
						_selectedFeature = i;
			ImGui::EndListBox();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("##FeatureConfigFrame", { 0, 0 }, true)) {
			bool shownFeature = false;
			for (int i = 0; i < Settings.size(); i++)
				if (Settings[i].Settings != nullptr) {
					if (i == _selectedFeature) {
						shownFeature = true;
						bool featureEnabled = Settings[i].Feature->IsEnabled();

						auto defaults = ConfigurationManager::GetSingleton()->DefaultSettings.Settings[i].Settings;
						_updated = Settings[i].Settings->DrawSettings(featureEnabled, isOptional, defaults);

						Settings[i].Feature->Enable(featureEnabled);
					}
				}
			if (!shownFeature)
				ImGui::TextDisabled("Please select a feature on the left.");
		}
		ImGui::EndChild();

		ImGui::EndTable();
	}
}


bool ShaderSettings::HasUpdated()
{
	return _updated;
}

void Configuration::ShaderSettings::ResetUpdatedState()
{
	_updated = false;
}
