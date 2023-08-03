 #pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;


struct DistantTreeLightingSettings : FeatureSettings
{
	fv_uint32 EnableComplexTreeLOD = 1;
	fv_uint32 EnableDirLightFix = 1;
	fv_float SubsurfaceScatteringAmount = 0.5f;
	fv_float FogDimmerAmount = 1.0f;

	void Draw();

	FEATURE_SETTINGS(DistantTreeLightingSettings,
		EnableComplexTreeLOD,
		EnableDirLightFix,
		SubsurfaceScatteringAmount,
		FogDimmerAmount)
};

class DistantTreeLighting : public FeatureWithSettings<DistantTreeLightingSettings>
{
public:
	
	static DistantTreeLighting* GetSingleton()
	{
		static DistantTreeLighting singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Tree LOD Lighting"; }
	virtual inline std::string GetShortName() { return "TreeLODLighting"; }

	struct alignas(16) PerPass
	{
		DirectX::XMFLOAT4	EyePosition;
		DirectX::XMFLOAT3X4	DirectionalAmbient;
		DirectX::XMFLOAT4	DirLightColor;
		DirectX::XMFLOAT4	DirLightDirection;
		float				DirLightScale;
		std::uint32_t		ComplexAtlasTexture;
		uint32_t EnableComplexTreeLOD;
		uint32_t EnableDirLightFix;
		float SubsurfaceScatteringAmount ;
		float FogDimmerAmount;
		float pad0;
		float pad1;
	};

	ConstantBuffer* perPass = nullptr;

	RE::TESWorldSpace* lastWorldSpace = nullptr;
	bool complexAtlasTexture = false;

	virtual void SetupResources() override;

	void ModifyDistantTree(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;
};