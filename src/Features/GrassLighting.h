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
		bool EnableDirLightFix = true;
		bool EnablePointLights = true;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride) override;

		FEATURE_SETTINGS_OVERRIDES(
			ConfigSettings,
			Glossiness,
			SpecularStrength,
			SubsurfaceScatteringAmount)
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
	virtual std::shared_ptr<FeatureSettings> ParseConfig(json& o_json) override;
	virtual void SaveConfig(json& o_json, std::shared_ptr<FeatureSettings> config) override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};
