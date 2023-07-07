#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

class DistantTreeLighting : public Feature
{
public:
	
	static DistantTreeLighting* GetSingleton()
	{
		static DistantTreeLighting singleton;
		return &singleton;
	}

	virtual std::string GetName() override
	{
		return "Tree LOD Lighting";
	}

	struct ShaderSettings
	{
		std::uint32_t EnableComplexTreeLOD;
		std::uint32_t EnableDirLightFix;
		float SubsurfaceScatteringAmount;
		float FogDimmerAmount;
	};

	struct ConfigSettings : FeatureSettings
	{
		uint32_t EnableComplexTreeLOD = 1;
		uint32_t EnableDirLightFix = 1;
		std::optional<TODValue<float>> SubsurfaceScatteringAmount= 0.5f;
		std::optional<TODValue<float>> FogDimmerAmount = 1.0f;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride) override;

		FEATURE_SETTINGS_OVERRIDES(
			ConfigSettings,
			SubsurfaceScatteringAmount,
			FogDimmerAmount)
	};	

	struct alignas(16) PerPass
	{
		DirectX::XMFLOAT4	EyePosition;
		DirectX::XMFLOAT3X4	DirectionalAmbient;
		DirectX::XMFLOAT4	DirLightColor;
		DirectX::XMFLOAT4	DirLightDirection;
		float				DirLightScale;
		std::uint32_t		ComplexAtlasTexture;
		ShaderSettings			Settings;
		float pad0;
		float pad1;
	};

	std::shared_ptr<ConfigSettings> configSettings;
	ConstantBuffer* perPass = nullptr;

	RE::TESWorldSpace* lastWorldSpace = nullptr;
	bool complexAtlasTexture = false;

	virtual void SetupResources() override;

	void ModifyDistantTree(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual std::shared_ptr<FeatureSettings> ParseConfig(json& o_json) override;
	virtual void SaveConfig(json& o_json, std::shared_ptr<FeatureSettings> config) override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;

};
