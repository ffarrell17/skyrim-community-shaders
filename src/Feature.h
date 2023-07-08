#pragma once
#include "FeatureSettings.h"

class Feature
{
public:
	virtual std::string GetName() = 0;
	virtual std::string GetShortName() = 0;

	std::string GetVersion();

	bool IsEnabled();
	void Enable(bool enable);
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

	virtual std::shared_ptr<FeatureSettings> CreateConfig() = 0;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) = 0;
	void ApplyDefaultConfig();

	// Cat: add all the features in here
	static const std::vector<Feature*>& GetFeatureList();

protected:

	std::string _version;
	bool _enabled = true;
	bool _loaded = false;

private:

	std::string RemoveSpaces(const std::string& str);
};