#pragma once
#include "PCH.h"
#include "Feature.h"
#include "FeatureSettings.h"

namespace Configuration
{
	struct FeatureSettingMap
	{
		Feature* Feature;
		std::shared_ptr<FeatureSettings> Settings;

		FeatureSettingMap(::Feature* f)
		{
			Feature = f;
		}

		std::string GetFeatureName()
		{
			return Feature->GetName();
		}
	};

	struct FeatureSettingsCollection : std::vector<FeatureSettingMap>
	{
		FeatureSettingsCollection(FeatureSettingsType type);

		void Load(std::map<Feature*, json>& featureConfigMap);  //(json& o_json);
		void Save(std::map<Feature*, json>& featureConfigMap);

		void ControlNewFeature(FeatureSettingMap& featureSettingsMap);
		void ControlNewFeature(Feature* Feature, std::shared_ptr<FeatureSettings> newSettings);

		bool HasUpdated();
		void ResetUpdatedState();

		void Draw();

		int SelectedFeature = -1;

	private:
		FeatureSettingsType _type;
		bool _updated = false;

		void ResetSettings();
	};
}