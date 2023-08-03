#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;

struct ExtendedMaterialsSettings : FeatureSettings
{
	fv_uint32 EnableComplexMaterial = 1;

	fv_uint32 EnableParallax = 1;
	fv_uint32 EnableTerrain = 0;
	fv_uint32 EnableHighQuality = 0;

	fv_uint32 MaxDistance = 2048;
	fv_float CRPMRange = 0.5f;
	fv_float BlendRange = 0.05f;
	fv_float Height = 0.1f;

	fv_uint32 EnableShadows = 1;
	fv_uint32 ShadowsStartFade = 512;
	fv_uint32 ShadowsEndFade = 1024;

	void Draw();

	FEATURE_SETTINGS(ExtendedMaterialsSettings,
			EnableComplexMaterial,
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

struct ExtendedMaterials : public FeatureWithSettings<ExtendedMaterialsSettings>
{
public:

	static ExtendedMaterials* GetSingleton()
	{
		static ExtendedMaterials singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Complex Parallax Materials"; }
	virtual inline std::string GetShortName() { return "ComplexParallaxMaterials"; }

	struct alignas(16) PerPass
	{
		uint32_t CullingMode = 0;

		uint32_t EnableComplexMaterial;
		uint32_t EnableParallax;
		uint32_t EnableTerrain;
		uint32_t EnableHighQuality;
		uint32_t MaxDistance;
		float CRPMRange;
		float BlendRange;
		float Height;
		uint32_t EnableShadows;
		uint32_t ShadowsStartFade;
		uint32_t ShadowsEndFade;
	};

	std::unique_ptr<Buffer> perPass = nullptr;

	ID3D11SamplerState* terrainSampler = nullptr;
	
	virtual void SetupResources() override;

	void ModifyLighting(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);

};