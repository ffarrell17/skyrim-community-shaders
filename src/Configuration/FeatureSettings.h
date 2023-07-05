#pragma once
#include "Macros.h"
#include "TODValue.h"
#include <optional>

namespace Configuration
{
	struct FeatureSettings
	{
		/// <summary>
		/// Draw the settings using ImGui
		/// </summary>
		/// <param name="featureEnabled">Reference to toggle the feature enabled state</param>
		/// <param name="isConfigOverride">Whether the config is overriding the default config</param>
		/// <returns>If the settings have been updated by the user</returns>
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride = false) = 0;

		// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
		virtual void Lerp(const std::shared_ptr<Configuration::FeatureSettings> start, const std::shared_ptr<Configuration::FeatureSettings> end, double t) = 0;
		// Can be auto generated with FEATURE_SETTINGS_OVERRIDES macro
		virtual void Override(const std::shared_ptr<Configuration::FeatureSettings> overrideSettings) = 0;
	};
}


#define FEATURE_SETTINGS_OVERRIDES(StructType, ...)                                               \
	void Lerp(const std::shared_ptr<Configuration::FeatureSettings> start,            \
		const std::shared_ptr<Configuration::FeatureSettings> end, double t)                    \
	{                                                                                                 \
		const std::shared_ptr<StructType> startSettings = dynamic_pointer_cast<StructType>(start); \
		const std::shared_ptr<StructType> endSettings = dynamic_pointer_cast<StructType>(end); \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_LERP_MEMBER, __VA_ARGS__)                                    \
	}                                                                                                 \
	void Override(const std::shared_ptr<Configuration::FeatureSettings> overrideSettings)            \
	{                                                                                                 \
		const std::shared_ptr<StructType> newSettings = dynamic_pointer_cast<StructType>(overrideSettings);      \
		APPLY_TO_STRUCT_MEMBERS(GENERATE_OVERRIDE_MEMBER, __VA_ARGS__)                                \
	}

#define FEATURE_SETTINGS_JSON_AND_OVERRIDES(structName, ...)       \
	FEATURE_SETTINGS_OVERRIDES(structName, __VA_ARGS__); \
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(structName, __VA_ARGS__)
