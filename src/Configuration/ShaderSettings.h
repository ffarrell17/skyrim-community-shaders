#pragma once
#include "PCH.h"
#include "Feature.h"
#include "FeatureSettings.h"

namespace Configuration
{
	struct ShaderSettings
	{
		ShaderSettings();

		struct FeatureSettingMap
		{
			Feature* Feature;
			std::shared_ptr<FeatureSettings> Settings = nullptr;

			FeatureSettingMap(::Feature* f) {
				Feature = f;
			}

			std::string GetFeatureName() {
				return Feature->GetName();
			}
		};

		std::vector<FeatureSettingMap> Settings;

		void Load(json& o_json, bool isOptional = false);
		void Save(json& o_json, bool isOptional = false);

		bool HasUpdated();
		void ResetUpdatedState();

		void Draw(std::string tabBarName = "Features", bool allowAndRemoveFeature = false, bool allowOverrides = false);

	private:
		bool _updated = false;
		int _selectedFeature = -1;

		void ClearConfig();
	};
}