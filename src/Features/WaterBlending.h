#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;

struct WaterBlendingSettings : FeatureSettings
{
	fv_uint32 EnableWaterBlending = 1;
	fv_uint32 EnableWaterBlendingSSR = 1;
	fv_float WaterBlendRange = 1;
	fv_float SSRBlendRange = 1;

	void Draw();

	FEATURE_SETTINGS(WaterBlendingSettings, EnableWaterBlending, EnableWaterBlendingSSR, WaterBlendRange, SSRBlendRange)
};


struct WaterBlending : FeatureWithSettings<WaterBlendingSettings>
{
public:

	static WaterBlending* GetSingleton()
	{
		static WaterBlending singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Water Blending"; }
	virtual inline std::string GetShortName() { return "WaterBlending"; }

	struct alignas(16) PerPass
	{
		float waterHeight;
		WaterBlendingSettings settings;
		float pad[3];
	};

	std::unique_ptr<Buffer> perPass = nullptr;

	virtual void SetupResources();

	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);
};

