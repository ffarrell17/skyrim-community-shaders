#pragma once

#include "Buffer.h"
#include "Feature.h"
#include "Configuration/FeatureValue.h"

using namespace Configuration;

struct GrassCollisionSettings : FeatureSettings
{
	fv_uint32 EnableGrassCollision = 1;
	fv_float RadiusMultiplier = 2;
	fv_float DisplacementMultiplier = 16;

	void Draw();

	FEATURE_SETTINGS(GrassCollisionSettings,
		EnableGrassCollision,
		RadiusMultiplier,
		DisplacementMultiplier)
};

class GrassCollision : public FeatureWithSettings<GrassCollisionSettings>
{
public:

	static GrassCollision* GetSingleton()
	{
		static GrassCollision singleton;
		return &singleton;
	}

	virtual inline std::string GetName() { return "Grass Collision"; }
	virtual inline std::string GetShortName() { return "GrassCollision"; }

	struct alignas(16) PerFrame
	{
		DirectX::XMFLOAT3 boundCentre;
		float boundRadius;

		uint32_t EnableGrassCollision;
		float RadiusMultiplier;
		float DisplacementMultiplier;

		float pad0;
	};

	struct CollisionSData
	{
		DirectX::XMFLOAT3 centre;
		float radius;
	};

	std::unique_ptr<Buffer> collisions = nullptr;

	bool updatePerFrame = false;
	ConstantBuffer* perFrame = nullptr;

	virtual void SetupResources() override;
	virtual void Reset() override;

	void UpdateCollisions();
	void ModifyGrass(const RE::BSShader* shader, const uint32_t descriptor);
	virtual void Draw(const RE::BSShader* shader, const uint32_t descriptor) override;

	std::vector<std::string> GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType);
};