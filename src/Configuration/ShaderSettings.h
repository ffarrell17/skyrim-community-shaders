#pragma once
#include "PCH.h"
#include "FeatureSettings.h"

namespace Configuration
{
	struct ShaderSettings
	{
		ShaderSettings();

		std::vector<std::shared_ptr<FeatureSettings>> FeatureSettings;

		void Load(json& o_json, bool isDefault);
		void Save(json& o_json, bool isDefault);

		bool HasUpdated();
		void ResetUpdatedState();

		void Draw(std::string tabBarName = "Features", bool allowAndRemoveFeature = false, bool allowOverrides = false);

	private:
		bool _updated = false;
	};
}