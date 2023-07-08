#pragma once
#include "PCH.h"
#include "Macros.h"
#include "Configuration\TODValue.h"


struct FeatureSettings
{
	/// <summary>
	/// Draw the settings using ImGui
	/// </summary>
	/// <param name="featureEnabled">Reference to toggle the feature enabled state</param>
	/// <param name="isConfigOverride">Whether the config is overriding the default config</param>
	/// <param name="defaultSettings">The default settings config for this feature type. For use when range based overrides are null</param>
	/// <returns>If the settings have been updated by the user</returns>
	virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride = false, std::shared_ptr<FeatureSettings> defaultSettings = nullptr) = 0;

	
	// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
	virtual void Lerp(const std::shared_ptr<FeatureSettings> start, const std::shared_ptr<FeatureSettings> end, double t) = 0;

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
	virtual void FromJson(const nlohmann::json& nlohmann_json_j) = 0;
};

namespace nlohmann
{
	template <typename T>
	struct adl_serializer<std::optional<T>>
	{
		static void to_json(nlohmann::json& j, const std::optional<T>& opt)
		{
			if (opt.has_value()) {
				j = opt.value();
			}
		}

		static void from_json(const nlohmann::json& j, std::optional<T>& opt)
		{
			if (j.is_null()) {
				opt = std::nullopt;
			} else {
				opt = j.get<T>();
			}
		}
	};

} 


#define FEATURE_SETTINGS_OPTIONALS(StructType, ...)																\
	void Lerp(const std::shared_ptr<FeatureSettings> start,														\
		const std::shared_ptr<FeatureSettings> end, double t)													\
	{																											\
		const std::shared_ptr<StructType> startSettings = dynamic_pointer_cast<StructType>(start);				\
		const std::shared_ptr<StructType> endSettings = dynamic_pointer_cast<StructType>(end);					\
		APPLY_TO_STRUCT_MEMBERS(GENERATE_LERP_MEMBER, __VA_ARGS__)												\
	}																											\
																												\
	void Override(const std::shared_ptr<FeatureSettings> overrideSettings)										\
	{																											\
		const std::shared_ptr<StructType> newSettings = dynamic_pointer_cast<StructType>(overrideSettings);     \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_OVERRIDE_MEMBER, __VA_ARGS__)											\
	}																											\
																												\
	void ResetOptionals()																						\
	{																											\
		APPLY_TO_STRUCT_MEMBERS(GENERATE_RESET_MEMBER, __VA_ARGS__)												\
	}																											\
																												\
	void ToJsonOptionals(nlohmann::json& nlohmann_json_j)                       \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;     \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                            \
		DELETE_NULL_JSON(nlohmann_json_j)												\
	}                                                                                                       \
\
	void FromJsonOptionals(const nlohmann::json& nlohmann_json_j)                     \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;\
		std::shared_ptr<StructType> tempPtr = std::make_shared<StructType>(); \
		tempPtr->ResetOptionals();\
		StructType& nlohmann_json_default_obj = *tempPtr;                                                                     \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__))             \
	} 

#define FEATURE_SETTINGS_ALL(StructType, ...)                                                         \
	void ToJson(nlohmann::json& nlohmann_json_j)                                                   \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;                                                                \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                            \
	}                                                                                                       \
                                                                                                            \
	void FromJson(const nlohmann::json& nlohmann_json_j)                                           \
	{                                                                                                       \
		StructType& nlohmann_json_t = *this;                                                                \
		std::shared_ptr<StructType> tempPtr = std::make_shared<StructType>();                               \
		StructType& nlohmann_json_default_obj = *tempPtr;                                                   \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__))             \
	}