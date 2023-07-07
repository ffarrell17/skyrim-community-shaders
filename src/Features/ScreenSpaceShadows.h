#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

class ScreenSpaceShadows : public Feature
{
public:

	static ScreenSpaceShadows* GetSingleton()
	{
		static ScreenSpaceShadows singleton;
		return &singleton;
	}

	virtual std::string GetName() override
	{
		return "Screen-Space Shadows";
	}

	struct ShaderSettings
	{
		uint32_t MaxSamples = 24;
		float FarDistanceScale = 0.025f;
		float FarThicknessScale = 0.025f;
		float FarHardness = 8.0f;
		float NearDistance = 16.0f;
		float NearThickness = 2.0f;
		float NearHardness = 32.0f;
		float BlurRadius = 0.5f;
		float BlurDropoff = 0.005f;
	};

	struct ConfigSettings : FeatureSettings
	{
		std::optional<uint32_t> MaxSamples = 24;
		std::optional<TODValue<float>> FarDistanceScale = 0.025f;
		std::optional<TODValue<float>> FarThicknessScale = 0.025f;
		std::optional<TODValue<float>> FarHardness = 8.0f;
		std::optional<TODValue<float>> NearDistance = 16.0f;
		std::optional<TODValue<float>> NearThickness = 2.0f;
		std::optional<TODValue<float>> NearHardness = 32.0f;
		std::optional<TODValue<float>> BlurRadius = 0.5f;
		std::optional<TODValue<float>> BlurDropoff = 0.005f;

		FEATURE_SETTINGS_OPTIONALS(
			ConfigSettings,
			MaxSamples,
			FarDistanceScale,
			FarThicknessScale,
			FarHardness,
			NearDistance,
			NearThickness,
			NearHardness,
			BlurRadius,
			BlurDropoff)

		FEATURE_SETTINGS_ALL(
			ConfigSettings,
			MaxSamples,
			FarDistanceScale,
			FarThicknessScale,
			FarHardness,
			NearDistance,
			NearThickness,
			NearHardness,
			BlurRadius,
			BlurDropoff)

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride) override;
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
