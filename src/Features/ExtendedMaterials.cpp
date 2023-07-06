#include "ExtendedMaterials.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
	ExtendedMaterials::ConfigSettings,
	EnableParallax,
	EnableTerrain,
	EnableComplexMaterial,
	EnableHighQuality,
	MaxDistance,
	CRPMRange,
	BlendRange,
	Height,
	EnableShadows,
	ShadowsStartFade,
	ShadowsEndFade)

bool ExtendedMaterials::ConfigSettings::DrawSettings(bool& featureEnabled, bool isConfigOverride)
{
	bool updated = false;

	featureEnabled = featureEnabled; //to hide warning

	if (!isConfigOverride && ImGui::TreeNodeEx("Complex Material", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextWrapped(
			"Enables support for the Complex Material specification which makes use of the environment mask.\n"
			"This includes parallax, as well as more realistic metals and specular reflections.\n"
			"May lead to some warped textures on modded content which have an invalid alpha channel in their environment mask.");
		updated = updated || ImGui::Checkbox("Enable Complex Material", (bool*)&EnableComplexMaterial);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Contact Refinement Parallax Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
		
		if (!isConfigOverride) {
			ImGui::TextWrapped("Enables parallax on standard meshes made for parallax.");
			updated = updated || ImGui::Checkbox("Enable Parallax", (bool*)&EnableParallax);

			ImGui::TextWrapped(
				"Enables terrain parallax using the alpha channel of each landscape texture.\n"
				"Therefore, all landscape textures must support parallax for this effect to work properly.");
			updated = updated || ImGui::Checkbox("Enable Terrain", (bool*)&EnableTerrain);

			ImGui::TextWrapped(
				"Doubles the sample count and approximates the intersection point using Parallax Occlusion Mapping.\n"
				"Significantly improves the quality and removes aliasing.\n"
				"TAA or the Skyrim Upscaler is recommended when using this option due to CRPM artifacts.");
			updated = updated || ImGui::Checkbox("Enable High Quality", (bool*)&EnableHighQuality);
		}

		ImGui::TextWrapped("The furthest distance from the camera which uses parallax.");
		updated = updated || MaxDistance->DrawSliderScalar("Max Distance", ImGuiDataType_U32, 0, 4096);

		ImGui::TextWrapped("The percentage of the max distance which uses CRPM.");
		updated = updated || CRPMRange->DrawSliderScalar("CRPM Range", ImGuiDataType_Float, 0.0f, 1.0f);

		ImGui::TextWrapped(
			"The range that parallax blends from POM to bump mapping, and bump mapping to nothing.\n"
			"This value should be set as low as possible due to the performance impact of blending POM and relief mapping.");
		updated = updated || BlendRange->DrawSliderScalar("Blend Range", ImGuiDataType_Float, 0.0f, CRPMRange.value());

		ImGui::TextWrapped("The range between the highest and lowest point a surface can be offset by.");
		updated = updated || Height->DrawSliderScalar("Height", ImGuiDataType_Float, 0, 0.2f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Approximate Soft Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
		
		
		if (!isConfigOverride) {
			ImGui::TextWrapped(
				"Enables cheap soft shadows when using parallax.\n"
				"This applies to all directional and point lights.");
			ImGui::Checkbox("Enable Shadows", (bool*)&EnableShadows);
		}

		ImGui::TextWrapped("Modifying the start and end fade can improve performance and hide obvious texture tiling.");
		updated = updated || ShadowsStartFade->DrawSliderScalar("Max Distance", ImGuiDataType_U32, 0, ShadowsEndFade.value());
		updated = updated || ShadowsEndFade->DrawSliderScalar("Max Distance", ImGuiDataType_U32, ShadowsStartFade.value(), 4096);

		ImGui::TreePop();
	}

	return updated;
}

ExtendedMaterials::ShaderSettings ExtendedMaterials::ConfigSettings::ToShaderSettings()
{
	ShaderSettings settings;

	settings.EnableComplexMaterial = EnableComplexMaterial;

	settings.EnableParallax = EnableParallax;
	settings.EnableTerrain = EnableTerrain;
	settings.EnableHighQuality = EnableHighQuality;

	settings.MaxDistance = MaxDistance->Get();
	settings.CRPMRange = CRPMRange->Get();
	settings.BlendRange = BlendRange->Get();
	settings.Height = Height->Get();

	settings.EnableShadows = EnableShadows;
	settings.ShadowsStartFade = ShadowsStartFade->Get();
	settings.ShadowsEndFade = ShadowsEndFade->Get();
	return settings;
}

void ExtendedMaterials::ModifyLighting(const RE::BSShader*, const uint32_t)
{
	auto context = RE::BSGraphics::Renderer::GetSingleton()->GetRuntimeData().context;

	{
		PerPass data{};
		data.CullingMode = RE::BSGraphics::RendererShadowState::GetSingleton()->GetRuntimeData().rasterStateCullMode;
		data.settings = configSettings->ToShaderSettings();

		D3D11_MAPPED_SUBRESOURCE mapped;
		DX::ThrowIfFailed(context->Map(perPass->resource.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
		size_t bytes = sizeof(PerPass);
		memcpy_s(mapped.pData, bytes, &data, bytes);
		context->Unmap(perPass->resource.get(), 0);
	}

	context->PSSetSamplers(1, 1, &terrainSampler);

	ID3D11ShaderResourceView* views[1]{};
	views[0] = perPass->srv.get();
	context->PSSetShaderResources(30, 1, views);
}

void ExtendedMaterials::Draw(const RE::BSShader* shader, const uint32_t descriptor)
{
	switch (shader->shaderType.get()) {
	case RE::BSShader::Type::Lighting:
		ModifyLighting(shader, descriptor);
		break;
	}
}

void ExtendedMaterials::SetupResources()
{
	D3D11_BUFFER_DESC sbDesc{};
	sbDesc.Usage = D3D11_USAGE_DYNAMIC;
	sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = sizeof(PerPass);
	sbDesc.ByteWidth = sizeof(PerPass);
	perPass = std::make_unique<Buffer>(sbDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = 1;
	perPass->CreateSRV(srvDesc);

	logger::info("Creating terrain parallax sampler state");

	auto renderer = RE::BSGraphics::Renderer::GetSingleton();
	auto device = renderer->GetRuntimeData().forwarder;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &terrainSampler));
}

std::vector<std::string> ExtendedMaterials::GetAdditionalRequiredShaderDefines(RE::BSShader::Type shaderType)
{
	std::vector<std::string> defines;

	switch (shaderType) {
	case RE::BSShader::Type::Lighting:
		defines.push_back("COMPLEX_PARALLAX_MATERIALS");
		break;
	}

	return defines;
}

std::shared_ptr<FeatureSettings> ExtendedMaterials::CreateConfig()
{
	return std::make_shared<ExtendedMaterials::ConfigSettings>();
}

std::shared_ptr<FeatureSettings> ExtendedMaterials::ParseConfig(json& o_json)
{
	auto emConfig = std::dynamic_pointer_cast<ExtendedMaterials::ConfigSettings>(CreateConfig());
	*emConfig = o_json;
	return emConfig;
}

void ExtendedMaterials::SaveConfig(json& o_json, std::shared_ptr<FeatureSettings> config)
{
	auto emConfig = std::dynamic_pointer_cast<ExtendedMaterials::ConfigSettings>(config);
	if (emConfig) {
		o_json = *emConfig;
	}
}

void ExtendedMaterials::ApplyConfig(std::shared_ptr<FeatureSettings> config)
{
	configSettings = std::dynamic_pointer_cast<ExtendedMaterials::ConfigSettings>(config);
}