#include "ShaderSettings.h"
#include "ConfigurationManager.h"
#include "State.h"

ShaderSettings::ShaderSettings()
{
	for (int i = 0; i < State::GetSingleton()->Features.size(); i++) {
		FeatureSettings.push_back(nullptr);
	}
}

void ShaderSettings::Load(json& o_json, bool isDefault)
{
	FeatureSettings.clear();

	for (auto& feature : State::GetSingleton()->Features) {
			
		json& featureJson = o_json[feature->GetName()];

		if (featureJson.is_object()) {
			FeatureSettings.push_back(feature->ParseConfig(featureJson));
			
			if (isDefault) {
				if (featureJson["Enabled"].is_boolean()) {
					feature->Enable(featureJson["Enabled"]);
				}
			}

		} 
		else
			FeatureSettings.push_back(nullptr);
	}
}

void ShaderSettings::Save(json& o_json, bool isDefault)
{
	for (int i = 0; i < FeatureSettings.size(); i++) {
		const auto& feature = State::GetSingleton()->Features[i];
		json fJson;
		feature->SaveConfig(fJson, FeatureSettings[i]);
		o_json[feature->GetName()] = fJson;
		if (isDefault)
			o_json[feature->GetName()]["Enabled"] = feature->IsEnabled();
	}
}

void ShaderSettings::Draw(std::string tabBarName, bool allowAndRemoveFeature, bool allowOverrides)
{
	_updated = false;
	auto state = State::GetSingleton();

	if (allowAndRemoveFeature) {
		// Creating a combo box for all feature names that this config doesn't have
		// If the user selects one the default config is added

		std::vector<std::string> featureNames;
		std::vector<const char*> featureNamesCStr;

		for (int i = 0; i < FeatureSettings.size(); i++) {
			if (FeatureSettings[i] == nullptr) {
				featureNames.push_back(state->Features[i]->GetName());
			}
		}

		for (const std::string& str : featureNames) {
			featureNamesCStr.push_back(str.c_str());
		}

		if (!featureNames.empty()) {
			int selectedItem = -1;
			if (ImGui::Combo("Add Feature Override", &selectedItem, featureNamesCStr.data(), static_cast<int>(featureNamesCStr.size()))) {
				for (int i = 0; i < FeatureSettings.size(); i++) {
					if (state->Features[i]->GetName() == featureNames[selectedItem]) {
						
						// TODO: Change from deep copy of default to new config with all optional configs null
						FeatureSettings[i] = state->Features[i]->CopyConfig(ConfigurationManager::GetSingleton()->DefaultSettings.FeatureSettings[i]);
					}
				}
			}
		}
	}

	if (ImGui::BeginTabBar(tabBarName.c_str(), allowOverrides ? ImGuiTabBarFlags_Reorderable : ImGuiTabBarFlags_None)) {
		for (int i = 0; i < FeatureSettings.size(); i++) {
			if (FeatureSettings[i]) {
				std::string featureName = state->Features[i]->GetName();

				bool tabOpen = true;
				if (!allowAndRemoveFeature && ImGui::BeginTabItem(featureName.c_str()) ||
					allowAndRemoveFeature && ImGui::BeginTabItem(featureName.c_str(), &tabOpen)) {

					bool featureEnabled = state->Features[i]->IsEnabled();

					_updated = FeatureSettings[i]->DrawSettings(featureEnabled, allowOverrides);

					state->Features[i]->Enable(featureEnabled);

					ImGui::EndTabItem();
				}

				if (!tabOpen) {  // Tab closed. Remove config
					FeatureSettings[i].reset();
				}
			}
		}

		ImGui::EndTabBar();
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
