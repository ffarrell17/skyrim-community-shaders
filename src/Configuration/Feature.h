#pragma once
#include "FeatureSettings.h"

namespace Configuration
{
	struct Feature
	{
	public:
		std::string GetName(bool noSpaces = false);

		std::string GetVersion();

		bool IsEnabled();
		void Enable(bool enable);

		std::string GetIniPath();

		virtual void Init();
		virtual void SetupResources() = 0;
		virtual void Reset() {}
		virtual void ClearComputeShader() {}

		virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) = 0;

		virtual bool ValidateCache(CSimpleIniA& a_ini);
		virtual void WriteDiskCacheInfo(CSimpleIniA& a_ini);

		virtual std::shared_ptr<FeatureSettings> CreateConfig() = 0;
		virtual std::shared_ptr<FeatureSettings> ParseConfig(json& o_json) = 0;
		virtual void SaveConfig(json& o_json, std::shared_ptr<FeatureSettings> config) = 0;
		virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) = 0;
		std::shared_ptr<FeatureSettings> LoadAndApplyConfig(json& o_json);
		std::shared_ptr<FeatureSettings> CopyConfig(std::shared_ptr<FeatureSettings> settings);

	protected:

		std::string _featureName;
		std::string _featureVersion;
		bool _enabled = true;

	private:

		std::string RemoveSpaces(const std::string& str);
	};
}