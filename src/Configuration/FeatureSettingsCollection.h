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

		void Load(json& o_json);
		void Save(json& o_json);

		void ControlNewFeature(Feature* Feature, std::shared_ptr<FeatureSettings> newSettings);

		bool HasUpdated();
		void ResetUpdatedState();

		void Draw();

	private:
		FeatureSettingsType _type;
		bool _updated = false;
		int _selectedFeature = -1;

		void ResetSettings();
	};
}