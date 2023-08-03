#include "FeatureValue.h"
#include "ConfigurationManager.h"

using namespace Configuration;


bool FeatureValueGeneric::IsWeatherOverrideEnabled()
{
	return Configuration::ConfigurationManager::GetSingleton()->UseWeatherOverrides;
}

bool FeatureValueGeneric::HasValue() const
{
	return *_hasValue;
}

void FeatureValueGeneric::Release()
{
	*_hasValue = false;
	_isTOD = false;
}

bool FeatureValueGeneric::IsTODValue() const
{
	return _isTOD;
}

void FeatureValueGeneric::SetIsTODValue(bool todVal)
{
	_isTOD = todVal;
}

void FeatureValueGeneric::CopyAndLink(FeatureValueGeneric* fv)
{
	Copy(fv);
	Link(fv);
}

bool FeatureValueGeneric::HasUpdated() const
{
	return _updated;
}

void FeatureValueGeneric::ResetUpdated()
{
	_updated = false;
}

void FeatureValueGeneric::SetType(FeatureSettingsType type)
{
	_type = type;
}

FeatureSettingsType FeatureValueGeneric::GetType()
{
	return _type;
}

std::shared_ptr<bool> FeatureValueGeneric::GetIsOverwrittenPtr()
{
	return _isOverwritten;
}

void FeatureValueGeneric::SetIsOverwrittenPtr(std::shared_ptr<bool> isOverwritten)
{
	_isOverwritten = isOverwritten;
}

std::shared_ptr<bool> FeatureValueGeneric::GetHasValuePtr()
{
	return _hasValue;
}

void FeatureValueGeneric::SetHasValuePtr(std::shared_ptr<bool> hasValue)
{
	_hasValue = hasValue;
}

std::string FeatureValueGeneric::GenerateGuidAsString()
{
	GUID guid;
	CoCreateGuid(&guid);

	return GuidToString(guid);
}

std::string FeatureValueGeneric::GuidToString(GUID guid)
{
	char guid_cstr[39];
	snprintf(guid_cstr, sizeof(guid_cstr),
		"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return std::string(guid_cstr);
}