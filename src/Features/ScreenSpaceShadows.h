#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;

/* class ScreenSpaceShadows : public Feature
{
public:

	static ScreenSpaceShadows* GetSingleton()
	{
		static ScreenSpaceShadows singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Screen-Space Shadows"; }
	virtual inline std::string GetShortName() { return "ScreenSpaceShadows"; }

	struct ShaderSettings
	{
		fv_uint32 MaxSamples = 24;
		fv_float FarDistanceScale = 0.025f;
		fv_float FarThicknessScale = 0.025f;
		fv_float FarHardness = 8.0f;
		fv_float NearDistance = 16.0f;
		fv_float NearThickness = 2.0f;
		fv_float NearHardness = 32.0f;
		fv_float BlurRadius = 0.5f;
		fv_float BlurDropoff = 0.005f;
	};

	struct alignas(16) PerPass
	{
		uint32_t EnableSSS;
		uint32_t FrameCount;
		uint32_t pad[2];
	};

	struct alignas(16) RaymarchCB
	{
		DirectX::XMFLOAT2 BufferDim;
		DirectX::XMFLOAT2 RcpBufferDim;
		DirectX::XMMATRIX ProjMatrix;
		DirectX::XMMATRIX InvProjMatrix;
		DirectX::XMFLOAT4 DynamicRes;
		DirectX::XMVECTOR InvDirLightDirectionVS;
		float ShadowDistance = 10000;
		ShaderSettings Settings;
	};

	std::shared_ptr<ConfigSettings> configSettings;

	ConstantBuffer* perPass = nullptr;

	ID3D11SamplerState* computeSampler = nullptr;

	Texture2D* screenSpaceShadowsTexture = nullptr;
	Texture2D* screenSpaceShadowsTextureTemp = nullptr;

	ConstantBuffer* raymarchCB = nullptr;
	ID3D11ComputeShader* raymarchProgram = nullptr;

	ID3D11ComputeShader* horizontalBlurProgram = nullptr;
	ID3D11ComputeShader* verticalBlurProgram = nullptr;

	bool renderedScreenCamera = false;

	virtual void SetupResources() override;
	virtual void Reset() override;
	virtual void ClearComputeShader() override;

	void ModifyGrass(const RE::BSShader* shader, const uint32_t descriptor);
	void ModifyDistantTree(const RE::BSShader*, const uint32_t descriptor);

	ID3D11ComputeShader* GetComputeShader();
	ID3D11ComputeShader* GetComputeShaderHorizontalBlur();
	ID3D11ComputeShader* GetComputeShaderVerticalBlur();

	void ModifyLighting(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};
*/