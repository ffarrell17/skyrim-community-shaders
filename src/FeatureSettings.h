#pragma once
#include "PCH.h"
#include "Macros.h"
#include "Configuration\FeatureValue.h"


struct FeatureSettings
{	

#pragma warning(suppress: 4100)
	virtual void Draw()
	{ }

	virtual std::vector<Configuration::fv_any*> GetAllSettings()
	{
		return {};
	}

	bool HasUpdated()
	{
		for (Configuration::fv_any* val : GetAllSettings()) {
			if (val->HasUpdated())
				return true;
		}
		return false;
	}

	void Copy(FeatureSettings& copy)
	{
		auto thisSettings = GetAllSettings();
		auto copySettings = copy.GetAllSettings();

		if (thisSettings.size() == copySettings.size()) {
			for (int i = 0; i < thisSettings.size(); i++) {
				thisSettings[i]->Copy(copySettings[i]);
			}
		}
	}

	void SetType(Configuration::FeatureSettingsType type)
	{
		for (Configuration::fv_any* val : GetAllSettings()) {
			val->SetType(type);
		}
	}

	void ReleaseAll() {
		for (Configuration::fv_any* val : GetAllSettings()) {
			val->Release();
		}
	}

	void TODUpdateAll() {
		for (Configuration::fv_any* val : GetAllSettings()) {
			val->TODUpdate();
		}
	}

#pragma warning(suppress: 4100)
	virtual void ToJson(nlohmann::json& nlohmann_json_j)
	{}
#pragma warning(suppress: 4100)
	virtual void FromJson(const nlohmann::json& nlohmann_json_j) {}

	
	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	/*virtual void Lerp(const std::shared_ptr<FeatureSettings> start, const std::shared_ptr<FeatureSettings> end, double t) = 0;

	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void Override(const std::shared_ptr<FeatureSettings> overrideSettings) = 0;

	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void ResetOptionals() = 0;

	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void ToJsonOptionals(nlohmann::json& nlohmann_json_j) = 0;
	
	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void FromJsonOptionals(const nlohmann::json& nlohmann_json_j) = 0;

	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void ToJson(nlohmann::json& nlohmann_json_j) = 0;

	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void FromJson(const nlohmann::json& nlohmann_json_j) = 0;*/


};

/*
#define FEATURE_SETTINGS_OPTIONALS(StructType, ...)                                                         \
	void Lerp(const std::shared_ptr<FeatureSettings> start,                                                 \
		const std::shared_ptr<FeatureSettings> end, double t)                                               \
	{                                                                                                       \
		const std::shared_ptr<StructType> startSettings = dynamic_pointer_cast<StructType>(start);          \
		const std::shared_ptr<StructType> endSettings = dynamic_pointer_cast<StructType>(end);              \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_LERP_MEMBER, __VA_ARGS__)                                          \
	}                                                                                                       \
                                                                                                            \
	void Override(const std::shared_ptr<FeatureSettings> overrideSettings)                                  \
	{                                                                                                       \
		const std::shared_ptr<StructType> newSettings = dynamic_pointer_cast<StructType>(overrideSettings); \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_OVERRIDE_MEMBER, __VA_ARGS__)                                      \
	}                                                                                                       \
                                                                                                            \
	void ResetOptionals()                                                                                   \
	{                                                                                                       \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_RESET_MEMBER, __VA_ARGS__)                                         \
	}                                                                                                       \
                                                                                                            \
	void ToJsonOptionals(nlohmann::json& nlohmann_json_j)                                                   \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;                                                                \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                            \
		DELETE_NULL_JSON(nlohmann_json_j)                                                                   \
	}                                                                                                       \
                                                                                                            \
	void FromJsonOptionals(const nlohmann::json& nlohmann_json_j)                                           \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;                                                                \
		std::shared_ptr<StructType> tempPtr = std::make_shared<StructType>();                               \
		tempPtr->ResetOptionals();                                                                          \
		StructType& nlohmann_json_default_obj = *tempPtr;                                                   \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__))             \
	}

#define FEATURE_SETTINGS_ALL(StructType, ...)                                                   \
	void ToJson(nlohmann::json& nlohmann_json_j)                                                \
	{                                                                                           \
		StructType& nlohmann_json_t = *this;                                                    \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                \
	}                                                                                           \
                                                                                                \
	void FromJson(const nlohmann::json& nlohmann_json_j)                                        \
	{                                                                                           \
		StructType& nlohmann_json_t = *this;                                                    \
		std::shared_ptr<StructType> tempPtr = std::make_shared<StructType>();                   \
		StructType& nlohmann_json_default_obj = *tempPtr;                                       \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) \ 
	}*/

#define FEATURE_SETTINGS(StructType, ...)                                                       \
	void ToJson(nlohmann::json& nlohmann_json_j)                                                \
	{                                                                                           \
		StructType& nlohmann_json_t = *this;                                                    \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                \
	}                                                                                           \
                                                                                                \
	void FromJson(const nlohmann::json& nlohmann_json_j)                                        \
	{                                                                                           \
		StructType& nlohmann_json_t = *this;                                                    \
		std::shared_ptr<StructType> tempPtr = std::make_shared<StructType>();                   \
		StructType& nlohmann_json_default_obj = *tempPtr;                                       \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) \
	}                                                                                           \
                                                                                                \
	std::vector<Configuration::fv_any*> GetAllSettings()                                        \
	{                                                                                           \
		return {                                                                                \
			APPLY_TO_STRUCT_MEMBERS(GENERATE_MEMBER_REF, __VA_ARGS__)                           \
		};                                                                                      \
	}