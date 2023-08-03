#pragma once
#include "PCH.h"
#include "Configuration/FeatureSettingsCollection.h"

namespace Configuration
{
	struct Weather
	{
		std::string Name;
		std::vector<RE::FormID> Ids;
		FeatureSettingsCollection FeatureSettings;

		Weather(bool defaultWeather = false);
		Weather(json& o_json, std::map<Feature*, json> featureConfigMap, bool defaultWeather = false);

		void Load(std::map<Feature*, json>& featureConfigMap);
		void Load(json& o_json, std::map<Feature*, json>& featureConfigMap);
		void Save(std::map<Feature*, json>& featureConfigMap);
		void Save(json& o_json, std::map<Feature*, json>& featureConfigMap);

		void Draw();

		bool HasUpdated();
		bool HasUpdatedIds();
		void ResetUpdatedState();

	private:
		
		void DrawIdList();

		bool _isDefaultWeather = false; // This is the fallback weather config if no matching weather config is found
		bool _updated = false;			// All updates
		bool _updatedIds = false;		// Ids changed updates only
	};
}