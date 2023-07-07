#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

class GrassLighting : public Feature
{
public:

	static GrassLighting* GetSingleton()
	{
		static GrassLighting singleton;
		return &singleton;
	}

	virtual std::string GetName() override
	{
		return "Grass Lighting";
	}

	struct ShaderSettings
	{
		float Glossiness;
		float SpecularStrength;
		float SubsurfaceScatteringAmount;
		std::uint32_t EnableDirLightFix;
		std::uint32_t EnablePointLights;
	};

	struct ConfigSettings : FeatureSettings
	{
		std::optional<TODValue<float>> Glossiness = 20.0f;
		std::optional<TODValue<float>> SpecularStrength = 0.5f;
		std::optional<TODValue<float>> SubsurfaceScatteringAmount = 0.5f;
		uint32_t EnableDirLightFix = 1;
		uint32_t EnablePointLights = 1;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride, std::shared_ptr<FeatureSettings> defaultSettings) override;

		FEATURE_SETTINGS_OPTIONALS(
			ConfigSettings,
			Glossiness,
			SpecularStrength,
			SubsurfaceScatteringAmount)

		FEATURE_SETTINGS_ALL(
			ConfigSettings,
			Glossiness,
			SpecularStrength,
			SubsurfaceScatteringAmount,
			EnableDirLightFix,
			EnablePointLights)
	};

	struct alignas(16) PerFrame
	{
		DirectX::XMFLOAT4 EyePosition;
		DirectX::XMFLOAT3X4 DirectionalAmbient;
		float SunlightScale;
		ShaderSettings Settings;
		float pad0;
		float pad1;
	};

	std::shared_ptr<ConfigSettings> configSettings;

	bool updatePerFrame = false;
	ConstantBuffer* perFrame = nullptr;

	virtual void SetupResources() override;
	virtual void Reset() override;

	void ModifyGrass(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};
