#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

struct WaterBlending : Feature
{
public:

	static WaterBlending* GetSingleton()
	{
		static WaterBlending singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Water Blending"; }
	virtual inline std::string GetShortName() { return "WaterBlending"; }

	struct ShaderSettings
	{
		uint32_t EnableWaterBlending;
		uint32_t EnableWaterBlendingSSR;
		float WaterBlendRange;
		float SSRBlendRange;
	};

	struct ConfigSettings : FeatureSettings
	{
		bool EnableWaterBlendingSSR = 1;
		std::optional<TODValue<float>> WaterBlendRange = 1;
		std::optional<TODValue<float>> SSRBlendRange = 1;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride, std::shared_ptr<FeatureSettings> defaultSettings) override;

		FEATURE_SETTINGS_OPTIONALS(
			ConfigSettings,
			WaterBlendRange,
			SSRBlendRange)

		FEATURE_SETTINGS_ALL(
			ConfigSettings,
			EnableWaterBlendingSSR,
			WaterBlendRange,
			SSRBlendRange)
	};	

	struct alignas(16) PerPass
	{
		float waterHeight;
		ShaderSettings settings;
		float pad[3];
	};

	std::shared_ptr<ConfigSettings> configSettings;

	std::unique_ptr<Buffer> perPass = nullptr;

	
	virtual void SetupResources();

	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};

