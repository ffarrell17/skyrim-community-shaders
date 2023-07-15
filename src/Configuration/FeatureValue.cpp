#include "FeatureValue.h"
#include "ConfigurationManager.h"

bool Configuration::FeatureValueGeneric::IsWeatherOverrideEnabled()
{
	return Configuration::ConfigurationManager::GetSingleton()->UseWeatherOverrides;
}