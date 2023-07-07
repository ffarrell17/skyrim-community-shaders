#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/TODValue.h"

using namespace Configuration;

class GrassCollision : public Feature
{
public:

	static GrassCollision* GetSingleton()
	{
		static GrassCollision singleton;
		return &singleton;
	}

	virtual std::string GetName() override
	{
		return "Grass Collision";
	}

	struct ShaderSettings
	{
		std::uint32_t EnableGrassCollision;
		float RadiusMultiplier = 2;
		float DisplacementMultiplier = 16;
	};

	struct ConfigSettings : FeatureSettings
	{
		std::optional<TODValue<float>> RadiusMultiplier = 2.0f;
		std::optional<TODValue<float>> DisplacementMultiplier = 8.0f;

		ShaderSettings ToShaderSettings();
		virtual bool DrawSettings(bool& featureEnabled, bool isConfigOverride, std::shared_ptr<FeatureSettings> defaultSettings) override;

		FEATURE_SETTINGS_OPTIONALS(ConfigSettings, RadiusMultiplier, DisplacementMultiplier)
		FEATURE_SETTINGS_ALL(ConfigSettings, RadiusMultiplier, DisplacementMultiplier)
	};

	struct alignas(16) PerFrame
	{
		DirectX::XMFLOAT3 boundCentre;
		float boundRadius;
		ShaderSettings Settings;
		float pad0;
	};

	struct CollisionSData
	{
		DirectX::XMFLOAT3 centre;
		float radius;
	};

	std::unique_ptr<Buffer> collisions = nullptr;

	std::shared_ptr<ConfigSettings> configSettings;

	bool updatePerFrame = false;
	ConstantBuffer* perFrame = nullptr;

	virtual void SetupResources() override;
	virtual void Reset() override;

	void UpdateCollisions();
	void ModifyGrass(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	virtual std::shared_ptr<FeatureSettings> CreateConfig() override;
	virtual void ApplyConfig(std::shared_ptr<FeatureSettings> config) override;
};
