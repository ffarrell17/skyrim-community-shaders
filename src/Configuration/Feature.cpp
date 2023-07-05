#include "Feature.h"

using namespace Configuration;

std::string Feature::GetName(bool noSpaces)
{
	return noSpaces ? RemoveSpaces(_featureName) : _featureName;
}

std::string Feature::GetVersion()
{
	return _featureVersion;
}

bool Feature::IsEnabled()
{
	return _enabled;
}

void Feature::Enable(bool enable)
{
	_enabled = enable;
}

std::string Feature::GetIniPath()
{
	return "Data\\Shaders\\Features\\" + GetName(true) + ".ini";
}

void Feature::Init()
{
	std::string iniPath = GetIniPath();

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath.c_str());
	if (auto value = ini.GetValue("Info", "Version")) {
		_enabled = true;
		_featureVersion = value;
		logger::info("{} successfully loaded", iniPath.c_str());
	} else {
		_enabled = false;
		logger::warn("{} not successfully loaded", iniPath.c_str());
	}
}

bool Feature::ValidateCache(CSimpleIniA& a_ini)
{
	logger::info("Validating {}", _featureName);

	auto enabledInCache = a_ini.GetBoolValue(_featureName.c_str(), "Enabled", false);
	if (enabledInCache && !_enabled) {
		logger::info("Feature was uninstalled");
		return false;
	}
	if (!enabledInCache && _enabled) {
		logger::info("Feature was installed");
		return false;
	}

	if (_enabled) {
		auto versionInCache = a_ini.GetValue(_featureName.c_str(), "Version");
		if (strcmp(versionInCache, _featureVersion.c_str()) != 0) {
			logger::info("Change in version detected. Installed {} but {} in Disk Cache", _featureVersion, versionInCache);
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
	a_ini.SetBoolValue(_featureName.c_str(), "Enabled", _enabled);
	a_ini.SetValue(_featureName.c_str(), "Version", _featureVersion.c_str());
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