#include "Feature.h"

#include "Features/DistantTreeLighting.h"
#include "Features/GrassCollision.h"
#include "Features/GrassLighting.h"
#include "Features/ScreenSpaceShadows.h"
#include "Features/ExtendedMaterials.h"
#include "Features/WaterBlending.h"

std::string Feature::GetNameNoSpaces()
{
	return RemoveSpaces(GetName());
}

std::string Feature::GetVersion()
{
	return _feature;
}

bool Feature::IsEnabled()
{
	return _enabled;
}

void Feature::Enable(bool enable)
{
	_enabled = enable;
}

bool Feature::IsLoaded()
{
	return _loaded;
}

std::string Feature::GetIniPath()
{
	return "Data\\Shaders\\Features\\" + GetNameNoSpaces() + ".ini";
}

void Feature::Init()
{
	std::string iniPath = GetIniPath();

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath.c_str());
	if (auto value = ini.GetValue("Info", "Version")) {
		_loaded = true;
		_feature = value;
		logger::info("{} successfully loaded", iniPath.c_str());
	} else {
		_loaded = false;
		logger::warn("{} not successfully loaded", iniPath.c_str());
	}
}

#pragma warning(suppress: 4100)
std::vector<std::string> Feature::GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType)
{
	return std::vector<std::string>();
}

bool Feature::ValidateCache(CSimpleIniA& a_ini)
{
	logger::info("Validating {}", GetName());

	auto enabledInCache = a_ini.GetBoolValue(GetName().c_str(), "Enabled", false);
	if (enabledInCache && !_loaded) {
		logger::info("Feature was uninstalled");
		return false;
	}
	if (!enabledInCache && _loaded) {
		logger::info("Feature was installed");
		return false;
	}

	if (_loaded) {
		auto versionInCache = a_ini.GetValue(GetName().c_str(), "Version");
		if (strcmp(versionInCache, _feature.c_str()) != 0) {
			logger::info("Change in version detected. Installed {} but {} in Disk Cache", _feature, versionInCache);
			return false;
		} else {
			logger::info("Installed version and cached version match.");
		}
	}

	logger::info("Cached feature is valid");
	return true;
}

void Feature::WriteDiskCacheInfo(CSimpleIniA& a_ini)
{
	a_ini.SetBoolValue(GetName().c_str(), "Enabled", _loaded);
	a_ini.SetValue(GetName().c_str(), "Version", _feature.c_str());
}

std::shared_ptr<FeatureSettings> Feature::LoadAndApplyConfig(json& o_json)
{
	auto config = ParseConfig(o_json);
	if (config)
		ApplyConfig(config);
	return config;
}

std::shared_ptr<FeatureSettings> Feature::CopyConfig(std::shared_ptr<FeatureSettings> settings)
{
	json defaultJson;
	SaveConfig(defaultJson, settings);
	return ParseConfig(defaultJson);
}

std::string Feature::RemoveSpaces(const std::string& str)
{
	std::string result = str;
	result.erase(std::remove_if(result.begin(), result.end(), [](unsigned char c) { return std::isspace(c); }), result.end());
	return result;
}

const std::vector<Feature*>& Feature::GetFeatureList()
{
	// Cat: essentially load order i guess
	static std::vector<Feature*> features = {
		GrassLighting::GetSingleton(),
		DistantTreeLighting::GetSingleton(),
		GrassCollision::GetSingleton(),
		ScreenSpaceShadows::GetSingleton(),
		ExtendedMaterials::GetSingleton(),
		WaterBlending::GetSingleton()
	};
	return features;
}