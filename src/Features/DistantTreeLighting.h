 #pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"
/*
using namespace Configuration;

class DistantTreeLighting : public Feature
{
public:
	
	static DistantTreeLighting* GetSingleton()
	{
		static DistantTreeLighting singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Tree LOD Lighting"; }
	virtual inline std::string GetShortName() { return "TreeLODLighting"; }

	struct Settings
	{
		fv_uint32 EnableComplexTreeLOD = 1;
		fv_uint32 EnableDirLightFix = 1;
		fv_float SubsurfaceScatteringAmount = 0.5f;
		fv_float FogDimmerAmount = 1.0f;
	};


	struct alignas(16) PerPass
	{
		DirectX::XMFLOAT4	EyePosition;
		DirectX::XMFLOAT3X4	DirectionalAmbient;
		DirectX::XMFLOAT4	DirLightColor;
		DirectX::XMFLOAT4	DirLightDirection;
		float				DirLightScale;
		std::uint32_t		ComplexAtlasTexture;
		Settings Settings;
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
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;

};
*/