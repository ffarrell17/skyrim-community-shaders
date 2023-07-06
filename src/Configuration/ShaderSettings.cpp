#include "ShaderSettings.h"
#include "ConfigurationManager.h"
#include "State.h"

using namespace Configuration;

ShaderSettings::ShaderSettings()
{
	for (int i = 0; i < Feature::GetFeatureList().size(); i++) {
		FeatureSettings.push_back(nullptr);
	}
}

void ShaderSettings::Load(json& o_json, bool isDefault)
{
	FeatureSettings.clear();

	for (auto& feature : Feature::GetFeatureList()) {
			
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
		const auto& feature = Feature::GetFeatureList()[i];
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

	const auto& featureList = Feature::GetFeatureList();

	if (allowAndRemoveFeature) {
		// Creating a combo box for all feature names that this config doesn't have
		// If the user selects one the default config is added

		std::vector<std::string> featureNames;
		std::vector<const char*> featureNamesCStr;

		for (int i = 0; i < FeatureSettings.size(); i++) {
			if (FeatureSettings[i] == nullptr) {
				featureNames.push_back(featureList[i]->GetName());
			}
		}

		for (const std::string& str : featureNames) {
			featureNamesCStr.push_back(str.c_str());
		}

		if (!featureNames.empty()) {
			int selectedItem = -1;
			if (ImGui::Combo("Add Feature Override", &selectedItem, featureNamesCStr.data(), static_cast<int>(featureNamesCStr.size()))) {
				for (int i = 0; i < FeatureSettings.size(); i++) {
					if (featureList[i]->GetName() == featureNames[selectedItem]) {
						
						// TODO: Change from deep copy of default to new config with all optional configs null
						FeatureSettings[i] = featureList[i]->CopyConfig(ConfigurationManager::GetSingleton()->DefaultSettings.FeatureSettings[i]);
					}
				}
			}
		}
	}

	if (ImGui::BeginTable(tabBarName.c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
		ImGui::TableSetupColumn("##ListOfFeatures", 0, 3);
		ImGui::TableSetupColumn("##FeatureConfig", 0, 7);

		static size_t selectedFeature = 0;

		ImGui::TableNextColumn();
		if (ImGui::BeginListBox("##FeatureList", { -FLT_MIN, -FLT_MIN })) {
			for (size_t i = 0; i < featureList.size(); i++)
				if (ImGui::Selectable(featureList[i]->GetName().c_str(), selectedFeature == i))
					selectedFeature = i;
			ImGui::EndListBox();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("##FeatureConfigFrame", { 0, 0 }, true)) {
			bool shownFeature = false;
			for (size_t i = 0; i < FeatureSettings.size(); i++)
				if (i == selectedFeature) {
					shownFeature = true;
					bool featureEnabled = featureList[i]->IsEnabled();

					_updated = FeatureSettings[i]->DrawSettings(featureEnabled, allowOverrides);

					featureList[i]->Enable(featureEnabled);

				}
			if (!shownFeature)
				ImGui::TextDisabled("Please select a feature on the left.");
		}
		ImGui::EndChild();

		ImGui::EndTable();
	}


	/* if (ImGui::BeginTabBar(tabBarName.c_str(), allowOverrides ? ImGuiTabBarFlags_Reorderable : ImGuiTabBarFlags_None)) {
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
	}*/
}


bool ShaderSettings::HasUpdated()
{
	return _updated;
}

void Configuration::ShaderSettings::ResetUpdatedState()
{
	_updated = false;
}
