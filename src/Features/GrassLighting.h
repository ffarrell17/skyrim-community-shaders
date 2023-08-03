#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;

struct GrassLightingSettings : FeatureSettings
{
	fv_float Glossiness = 20.0f;
	fv_float SpecularStrength = 0.5f;
	fv_float SubsurfaceScatteringAmount = 0.5f;
	fv_uint32 EnableDirLightFix = 1;
	fv_uint32 EnablePointLights = 1;

	void Draw();

	FEATURE_SETTINGS(GrassLightingSettings,
		Glossiness,
		SpecularStrength,
		SubsurfaceScatteringAmount,
		EnableDirLightFix,
		EnablePointLights)
};

class GrassLighting : public FeatureWithSettings<GrassLightingSettings>
{
public:

	static GrassLighting* GetSingleton()
	{
		static GrassLighting singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Grass Lighting"; }
	virtual inline std::string GetShortName() { return "GrassLighting"; }

	struct alignas(16) PerFrame
	{
		DirectX::XMFLOAT4 EyePosition;
		DirectX::XMFLOAT3X4 DirectionalAmbient;
		float SunlightScale;

		float Glossiness;
		float SpecularStrength;
		float SubsurfaceScatteringAmount;
		uint32_t EnableDirLightFix;
		uint32_t EnablePointLights;

		float pad0;
		float pad1;
	};

	bool updatePerFrame = false;
	ConstantBuffer* perFrame = nullptr;

	virtual void SetupResources() override;
	virtual void Reset() override;

	void ModifyGrass(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;
};