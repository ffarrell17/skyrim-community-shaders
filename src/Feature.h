#pragma once
#include "FeatureSettings.h"

class Feature
{
public:
	virtual std::string GetName() = 0;
	virtual std::string GetShortName() = 0;

	std::string GetVersion();

	bool IsLoaded();

	std::string GetIniPath();

	virtual void Init();
	virtual void SetupResources() = 0;
	virtual void Reset() {}
	virtual void ClearComputeShader() {}

	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) = 0;

	virtual std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);

	virtual bool ValidateCache(CSimpleIniA& a_ini);
	virtual void WriteDiskCacheInfo(CSimpleIniA& a_ini);

	virtual std::shared_ptr<FeatureSettings> CreateNewSettings() = 0;
	virtual void SetSettings(FeatureSettings& newSettings) = 0;

	// Cat: add all the features in here
	static const std::vector<Feature*>& GetFeatureList();

protected:

	std::string version;
	bool loaded = false;
};


template <typename SettingsT>
class FeatureWithSettings : public Feature
{
protected:
	SettingsT settings;

public:
	virtual std::shared_ptr<FeatureSettings> CreateNewSettings() override
	{
		return std::make_shared<SettingsT>();
	}
	
	void SetSettings(FeatureSettings& newSettings)
	{
		settings = dynamic_cast<SettingsT&>(newSettings);
	}
};
