/* #pragma once

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

	struct Settings
	{
		fv_uint32 EnableComplexMaterial;

		fv_uint32 EnableParallax;
		fv_uint32 EnableTerrain;
		fv_uint32 EnableHighQuality;

		fv_uint32 MaxDistance;
		fv_float CRPMRange ;
		fv_float BlendRange;
		fv_float Height;

		fv_uint32 EnableShadows;
		fv_uint32 ShadowsStartFade;
		fv_uint32 ShadowsEndFade;
	};

	struct alignas(16) PerPass
	{
		uint32_t CullingMode = 0;
		Settings settings;
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
};*/