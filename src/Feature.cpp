#include "Feature.h"

#include "Features/DistantTreeLighting.h"
#include "Features/ExtendedMaterials.h"
#include "Features/GrassCollision.h"
#include "Features/GrassLighting.h"
#include "Features/ScreenSpaceShadows.h"
#include "Features/WaterBlending.h"

std::string Feature::GetVersion()
{
	return version;
}

bool Feature::IsLoaded()
{
	return loaded;
}

std::string Feature::GetIniPath()
{
	return "Data\\Shaders\\Features\\" + GetShortName() + ".ini";
}

void Feature::Init()
{
	std::string iniPath = GetIniPath();

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath.c_str());
	if (auto value = ini.GetValue("Info", "Version")) {
		loaded = true;
		version = value;
		logger::info("{} successfully loaded", iniPath.c_str());
	} else {
		loaded = false;
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
	if (enabledInCache && !loaded) {
		logger::info("Feature was uninstalled");
		return false;
	}
	if (!enabledInCache && loaded) {
		logger::info("Feature was installed");
		return false;
	}

	if (loaded) {
		auto versionInCache = a_ini.GetValue(ini_name.c_str(), "Version");
		if (strcmp(versionInCache, version.c_str()) != 0) {
			logger::info("Change in version detected. Installed {} but {} in Disk Cache", version, versionInCache);
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
	a_ini.SetBoolValue(ini_name.c_str(), "Enabled", loaded);
	a_ini.SetValue(ini_name.c_str(), "Version", version.c_str());
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

	static std::vector<Feature*> featuresVR = {
		GrassLighting::GetSingleton(),
		ExtendedMaterials::GetSingleton(),
	};

	return REL::Module::IsVR() ? featuresVR : features;
}