#include "Feature.h"

#include "Features/DistantTreeLighting.h"
#include "Features/GrassCollision.h"
#include "Features/GrassLighting.h"
#include "Features/ScreenSpaceShadows.h"
#include "Features/ExtendedMaterials.h"
#include "Features/WaterBlending.h"


std::string Feature::GetVersion()
{
	return _version;
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
	return "Data\\Shaders\\Features\\" + GetShortName() + ".ini";
}

void Feature::Init()
{
	ApplyDefaultConfig();

	std::string iniPath = GetIniPath();

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath.c_str());
	if (auto value = ini.GetValue("Info", "Version")) {
		_loaded = true;
		_version = value;
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
	auto name = GetName();
	auto ini_name = GetShortName();

	logger::info("Validating {}", name);

	auto enabledInCache = a_ini.GetBoolValue(ini_name.c_str(), "Enabled", false);
	if (enabledInCache && !_loaded) {
		logger::info("Feature was uninstalled");
		return false;
	}
	if (!enabledInCache && _loaded) {
		logger::info("Feature was installed");
		return false;
	}

	if (_loaded) {
		auto versionInCache = a_ini.GetValue(ini_name.c_str(), "Version");
		if (strcmp(versionInCache, _version.c_str()) != 0) {
			logger::info("Change in version detected. Installed {} but {} in Disk Cache", _version, versionInCache);
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
	auto ini_name = GetShortName();
	a_ini.SetBoolValue(ini_name.c_str(), "Enabled", _loaded);
	a_ini.SetValue(ini_name.c_str(), "Version", _version.c_str());
}

void Feature::ApplyDefaultConfig()
{
	ApplyConfig(CreateConfig());
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