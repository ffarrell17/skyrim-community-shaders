#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

struct ExtendedMaterials : Feature
{
public:

	static ExtendedMaterials* GetSingleton()
	{
		static ExtendedMaterials singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Complex Parallax Materials"; }
	virtual inline std::string GetShortName() { return "ComplexParallaxMaterials"; }

	struct ShaderSettings
	{
		uint32_t EnableComplexMaterial;

		uint32_t EnableParallax;
		uint32_t EnableTerrain;
		uint32_t EnableHighQuality;

		uint32_t MaxDistance;
		float CRPMRange ;
		float BlendRange;
		float Height;

		uint32_t EnableShadows;
		uint32_t ShadowsStartFade;
		uint32_t ShadowsEndFade;
	};

	struct ConfigSettings : FeatureSettings
	{
		uint32_t EnableParallax = 1;
		uint32_t EnableTerrain = 0;
		uint32_t EnableHighQuality = 0;

		std::optional<TODValue<uint32_t>> MaxDistance = 2048;
		std::optional<TODValue<float>> CRPMRange = 0.5f;
		std::optional<TODValue<float>> BlendRange = 0.05f;
		std::optional<TODValue<float>> Height = 0.1f;

		uint32_t EnableShadows = 1;
		std::optional<TODValue<uint32_t>> ShadowsStartFade = 512;
		std::optional<TODValue<uint32_t>> ShadowsEndFade = 1024;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride, std::shared_ptr<FeatureSettings> defaultSettings) override;

		FEATURE_SETTINGS_OPTIONALS(
			ConfigSettings,
			MaxDistance,
			CRPMRange,
			BlendRange,
			Height,
			ShadowsStartFade,
			ShadowsEndFade)

		FEATURE_SETTINGS_ALL(
			ConfigSettings,
			EnableParallax,
			EnableTerrain,
			EnableHighQuality,
			MaxDistance,
			CRPMRange,
			BlendRange,
			Height,
			EnableShadows,
			ShadowsStartFade,
			ShadowsEndFade)
	};

	struct alignas(16) PerPass
	{
		uint32_t CullingMode = 0;
		ShaderSettings settings;
	};

	std::shared_ptr<ConfigSettings> configSettings;

	std::unique_ptr<Buffer> perPass = nullptr;

	ID3D11SamplerState* terrainSampler = nullptr;
	
	virtual void SetupResources() override;

	void ModifyLighting(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};

