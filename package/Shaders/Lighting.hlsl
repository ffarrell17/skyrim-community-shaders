#include "Common/FrameBuffer.hlsl"
#include "Common/MotionBlur.hlsl"

#if (defined(TREE_ANIM) || defined(LANDSCAPE)) && !defined(VC)
#	define VC
#endif  // TREE_ANIM || LANDSCAPE || !VC

#if defined(SPECULAR) || defined(AMBIENT_SPECULAR) || defined(ENVMAP) || defined(RIM_LIGHTING) || defined(PARALLAX) || defined(MULTI_LAYER_PARALLAX) || defined(FACEGEN) || defined(FACEGEN_RGB_TINT) || defined(SNOW_FLAG) || defined(EYE) || defined(PBR)
#	define HAS_VIEW_VECTOR
#endif  // defined(SPECULAR) || defined(AMBIENT_SPECULAR) || defined(ENVMAP) || defined(RIM_LIGHTING) || defined(PARALLAX) || defined(MULTI_LAYER_PARALLAX) || defined(FACEGEN) || defined(FACEGEN_RGB_TINT) || defined(SNOW_FLAG) || defined(EYE) || defined(PBR)

struct VS_INPUT
{
	float4 Position : POSITION0;
	float2 TexCoord0 : TEXCOORD0;
#if !defined(MODELSPACENORMALS)
	float4 Normal : NORMAL0;
	float4 Bitangent : BINORMAL0;
#endif  // !MODELSPACENORMALS

#if defined(VC)
	float4 Color : COLOR0;
#	if defined(LANDSCAPE)
	float4 LandBlendWeights1 : TEXCOORD2;
	float4 LandBlendWeights2 : TEXCOORD3;
#	endif  // LANDSCAPE
#endif      // VC
#if defined(SKINNED)
	float4 BoneWeights : BLENDWEIGHT0;
	float4 BoneIndices : BLENDINDICES0;
#endif  // SKINNED
#if defined(EYE)
	float EyeParameter : TEXCOORD2;
#endif  // EYE
#if defined(VR)
	uint InstanceID : SV_INSTANCEID;
#endif  // VR
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION0;
#if (defined(PROJECTED_UV) && !defined(SKINNED)) || defined(LANDSCAPE)
	float4
#else
	float2
#endif  // (defined (PROJECTED_UV) && !defined(SKINNED)) || defined(LANDSCAPE)
		TexCoord0 : TEXCOORD0;
#if defined(ENVMAP)
	precise
#endif  // ENVMAP
		float3 InputPosition : TEXCOORD4;
#if defined(SKINNED) || !defined(MODELSPACENORMALS)
	float3 TBN0 : TEXCOORD1;
	float3 TBN1 : TEXCOORD2;
	float3 TBN2 : TEXCOORD3;
#endif  // defined(SKINNED) || !defined(MODELSPACENORMALS)
#if defined(HAS_VIEW_VECTOR)
	float3 ViewVector : TEXCOORD5;
#endif  // HAS_VIEW_VECTOR
#if defined(EYE)
	float3 EyeNormal : TEXCOORD6;
#elif defined(LANDSCAPE)
	float4 LandBlendWeights1 : TEXCOORD6;
	float4 LandBlendWeights2 : TEXCOORD7;
#elif defined(PROJECTED_UV) && !defined(SKINNED)
	float3 TexProj : TEXCOORD7;
#endif  // EYE
	float3 ScreenNormalTransform0 : TEXCOORD8;
	float3 ScreenNormalTransform1 : TEXCOORD9;
	float3 ScreenNormalTransform2 : TEXCOORD10;
	// #if !defined(VR)  // Position is normally not in VR, but perhaps we can use
	// it.
	float4 WorldPosition : POSITION1;
	float4 PreviousWorldPosition : POSITION2;
	// #endif // !VR
	float4 Color : COLOR0;
	float4 FogParam : COLOR1;
#if defined(VR)
	float ClipDistance : SV_ClipDistance0;  // o11
	float CullDistance : SV_CullDistance0;  // p11
#endif                                      // VR
};
#ifdef VSHADER

cbuffer PerTechnique : register(b0)
{
#	if !defined(VR)
	float4 HighDetailRange[1] : packoffset(c0);  // loaded cells center in xy, size in zw
	float4 FogParam : packoffset(c1);
	float4 FogNearColor : packoffset(c2);
	float4 FogFarColor : packoffset(c3);
#	else
	float4 HighDetailRange[2] : packoffset(c0);  // loaded cells center in xy, size in zw
	float4 FogParam : packoffset(c2);
	float4 FogNearColor : packoffset(c3);
	float4 FogFarColor : packoffset(c4);
#	endif  // VR
};

cbuffer PerMaterial : register(b1)
{
	float4 LeftEyeCenter : packoffset(c0);
	float4 RightEyeCenter : packoffset(c1);
	float4 TexcoordOffset : packoffset(c2);
};

cbuffer PerGeometry : register(b2)
{
#	if !defined(VR)
	row_major float3x4 World[1] : packoffset(c0);
	row_major float3x4 PreviousWorld[1] : packoffset(c3);
	float4 EyePosition[1] : packoffset(c6);
	float4 LandBlendParams : packoffset(c7);  // offset in xy, gridPosition in yw
	float4 TreeParams : packoffset(c8);       // wind magnitude in y, amplitude in z, leaf frequency in w
	float2 WindTimers : packoffset(c9);
	row_major float3x4 TextureProj[1] : packoffset(c10);
	float IndexScale : packoffset(c13);
	float4 WorldMapOverlayParameters : packoffset(c14);
#	else   // VR has 49 vs 30 entries
	row_major float3x4 World[2] : packoffset(c0);
	row_major float3x4 PreviousWorld[2] : packoffset(c6);
	float4 EyePosition[2] : packoffset(c12);
	float4 LandBlendParams : packoffset(c14);  // offset in xy, gridPosition in yw
	float4 TreeParams : packoffset(c15);       // wind magnitude in y, amplitude in z, leaf frequency in w
	float2 WindTimers : packoffset(c16);
	row_major float3x4 TextureProj[2] : packoffset(c17);
	float IndexScale : packoffset(c23);
	float4 WorldMapOverlayParameters : packoffset(c24);
#	endif  // VR
};

#	if defined(SKINNED)
cbuffer PreviousBonesBuffer : register(b9)
{
	float4 PreviousBones[240] : packoffset(c0);
}

cbuffer BonesBuffer : register(b10) { float4 Bones[240] : packoffset(c0); }
#	endif

cbuffer VS_PerFrame : register(b12)
{
#	if !defined(VR)
	row_major float3x3 ScreenProj[1] : packoffset(c0);
	row_major float4x4 ViewProj[1] : packoffset(c8);
#		if defined(SKINNED)
	float3 BonesPivot[1] : packoffset(c40);
	float3 PreviousBonesPivot[1] : packoffset(c41);
#		endif  // SKINNED
#	else
	row_major float3x3 ScreenProj[2] : packoffset(c0);
	row_major float4x4 ViewProj[2] : packoffset(c16);
#		if defined(SKINNED)
	float3 BonesPivot[2] : packoffset(c80);
	float3 PreviousBonesPivot[2] : packoffset(c82);
#		endif  // SKINNED
#	endif      // VR
};

#	ifdef VR
cbuffer cb13 : register(b13)
{
	float4 cb13[3];
}
#	endif  // VR

const static float4x4 M_IdentityMatrix = {
	{ 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 }
};

#	if defined(TREE_ANIM)
float2 GetTreeShiftVector(float4 position, float4 color)
{
	precise float4 tmp1 = (TreeParams.w * TreeParams.y).xxxx * WindTimers.xxyy;
	precise float4 tmp2 = float4(0.1, 0.25, 0.1, 0.25) * tmp1 + dot(position.xyz, 1.0.xxx).xxxx;
	precise float4 tmp3 = abs(-1.0.xxxx + 2.0.xxxx * frac(0.5.xxxx + tmp2.xyzw));
	precise float4 tmp4 = (tmp3 * tmp3) * (3.0.xxxx - 2.0.xxxx * tmp3);
	return (tmp4.xz + 0.1.xx * tmp4.yw) * (TreeParams.z * color.w).xx;
}
#	endif

#	if defined(SKINNED)
float3x4 GetBoneMatrix(float4 bones[240], int4 actualIndices, float3 pivot, float4 weights)
{
	/*float3x4 result;
for (int rowIndex = 0; rowIndex < 3; ++rowIndex)
{
float4 pivotRow = float4(0, 0, 0, pivot[rowIndex]);
result[rowIndex] = 0.0.xxxx;
for (int boneIndex = 0; boneIndex < 4; ++boneIndex)
{
result[rowIndex] += (bones[actualIndices[boneIndex] +
rowIndex] - pivotRow) * weights[boneIndex];
}
}
return result;*/

	float3x4 pivotMatrix = transpose(float4x3(0.0.xxx, 0.0.xxx, 0.0.xxx, pivot));

	float3x4 boneMatrix1 =
		float3x4(bones[actualIndices.x], bones[actualIndices.x + 1], bones[actualIndices.x + 2]);
	float3x4 boneMatrix2 =
		float3x4(bones[actualIndices.y], bones[actualIndices.y + 1], bones[actualIndices.y + 2]);
	float3x4 boneMatrix3 =
		float3x4(bones[actualIndices.z], bones[actualIndices.z + 1], bones[actualIndices.z + 2]);
	float3x4 boneMatrix4 =
		float3x4(bones[actualIndices.w], bones[actualIndices.w + 1], bones[actualIndices.w + 2]);

	float3x4 unitMatrix = float3x4(1.0.xxxx, 1.0.xxxx, 1.0.xxxx);
	float3x4 weightMatrix1 = unitMatrix * weights.x;
	float3x4 weightMatrix2 = unitMatrix * weights.y;
	float3x4 weightMatrix3 = unitMatrix * weights.z;
	float3x4 weightMatrix4 = unitMatrix * weights.w;

	return (boneMatrix1 - pivotMatrix) * weightMatrix1 +
	       (boneMatrix2 - pivotMatrix) * weightMatrix2 +
	       (boneMatrix3 - pivotMatrix) * weightMatrix3 +
	       (boneMatrix4 - pivotMatrix) * weightMatrix4;
}

float3x3 GetBoneRSMatrix(float4 bones[240], int4 actualIndices, float4 weights)
{
	float3x3 result;
	for (int rowIndex = 0; rowIndex < 3; ++rowIndex) {
		result[rowIndex] = weights.xxx * bones[actualIndices.x + rowIndex].xyz +
		                   weights.yyy * bones[actualIndices.y + rowIndex].xyz +
		                   weights.zzz * bones[actualIndices.z + rowIndex].xyz +
		                   weights.www * bones[actualIndices.w + rowIndex].xyz;
	}
	return result;
}
#	endif  // SKINNED

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT vsout;

	precise float4 inputPosition = float4(input.Position.xyz, 1.0);

#	if !defined(VR)
	uint eyeIndex = 0;
	uint eyeIndexX3 = 0;
	uint eyeIndexX4 = 0;
#	else   // VR
	uint eyeIndex = cb13[0].y * (input.InstanceID.x & 1);
	uint eyeIndexX3 = eyeIndex * 3;
	uint eyeIndexX4 = eyeIndex << 2;
#	endif  // VR

#	if defined(LODLANDNOISE) || defined(LODLANDSCAPE)
	float4 rawWorldPosition = float4(mul(World[eyeIndex], inputPosition), 1);
	float worldXShift = rawWorldPosition.x - HighDetailRange[eyeIndex].x;
	float worldYShift = rawWorldPosition.y - HighDetailRange[eyeIndex].y;
	if ((abs(worldXShift) < HighDetailRange[eyeIndex].z) && (abs(worldYShift) < HighDetailRange[eyeIndex].w)) {
		inputPosition.z -= (230 + rawWorldPosition.z / 1e9);
	}
#	endif  // defined(LODLANDNOISE) || defined(LODLANDSCAPE)                                                                   \

	precise float4 previousInputPosition = inputPosition;

#	if defined(TREE_ANIM)
	precise float2 treeShiftVector = GetTreeShiftVector(input.Position, input.Color);
	float3 normal = -1.0.xxx + 2.0.xxx * input.Normal.xyz;

	inputPosition.xyz += normal.xyz * treeShiftVector.x;
	previousInputPosition.xyz += normal.xyz * treeShiftVector.y;
#	endif

#	if defined(SKINNED)
	precise int4 actualIndices = 765.01.xxxx * input.BoneIndices.xyzw;

	float3x4 previousWorldMatrix =
		GetBoneMatrix(PreviousBones, actualIndices, PreviousBonesPivot[eyeIndex], input.BoneWeights);
	precise float4 previousWorldPosition =
		float4(mul(inputPosition, transpose(previousWorldMatrix)), 1);

	float3x4 worldMatrix = GetBoneMatrix(Bones, actualIndices, BonesPivot[eyeIndex], input.BoneWeights);
	precise float4 worldPosition = float4(mul(inputPosition, transpose(worldMatrix)), 1);

	float4 viewPos = mul(ViewProj[eyeIndex], worldPosition);
#	else   // !SKINNED
	precise float4 previousWorldPosition = float4(mul(PreviousWorld[eyeIndex], inputPosition), 1);
	precise float4 worldPosition = float4(mul(World[eyeIndex], inputPosition), 1);
	precise float4x4 world4x4 = float4x4(World[eyeIndex][0], World[eyeIndex][1], World[eyeIndex][2], float4(0, 0, 0, 1));
	precise float4x4 modelView = mul(ViewProj[eyeIndex], world4x4);
	float4 viewPos = mul(modelView, inputPosition);
#	endif  // SKINNED

#	if defined(OUTLINE) && !defined(MODELSPACENORMALS)
	float3 normal = normalize(-1.0.xxx + 2.0.xxx * input.Normal.xyz);
	float outlineShift = min(max(viewPos.z / 150, 1), 50);
	inputPosition.xyz += outlineShift * normal.xyz;
	previousInputPosition.xyz += outlineShift * normal.xyz;

#		if defined(SKINNED)
	previousWorldPosition =
		float4(mul(inputPosition, transpose(previousWorldMatrix)), 1);
	worldPosition = float4(mul(inputPosition, transpose(worldMatrix)), 1);
	viewPos = mul(ViewProj[eyeIndex], worldPosition);
#		else   // !SKINNED
	previousWorldPosition = float4(mul(PreviousWorld[eyeIndex], inputPosition), 1);
	worldPosition = float4(mul(World[eyeIndex], inputPosition), 1);
	viewPos = mul(modelView, inputPosition);
#		endif  // SKINNED
#	endif      // defined(OUTLINE) && !defined(MODELSPACENORMALS)

	vsout.Position = viewPos;

#	if defined(LODLANDNOISE) || defined(LODLANDSCAPE)
	vsout.Position.z += min(1, 1e-4 * max(0, viewPos.z - 70000)) * 0.5;
#	endif

	float2 uv = input.TexCoord0.xy * TexcoordOffset.zw + TexcoordOffset.xy;
#	if defined(LANDSCAPE)
	vsout.TexCoord0.zw = (uv * 0.010416667.xx + LandBlendParams.xy) * float2(1, -1) + float2(0, 1);
#	elif defined(PROJECTED_UV) && !defined(SKINNED)
	vsout.TexCoord0.z = mul(TextureProj[eyeIndex][0], inputPosition);
	vsout.TexCoord0.w = mul(TextureProj[eyeIndex][1], inputPosition);
#	endif
	vsout.TexCoord0.xy = uv;

#	if defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(SKINNED)
	vsout.InputPosition.xyz = worldPosition.xyz;
#	elif defined(WORLD_MAP)
	vsout.InputPosition.xyz = WorldMapOverlayParameters.xyz + worldPosition.xyz;
#	else
	vsout.InputPosition.xyz = inputPosition.xyz;
#	endif

#	if defined(SKINNED)
	float3x3 boneRSMatrix = GetBoneRSMatrix(Bones, actualIndices, input.BoneWeights);
#	endif

#	if !defined(MODELSPACENORMALS)
	float3x3 tbn = float3x3(
		float3(input.Position.w, input.Normal.w * 2 - 1, input.Bitangent.w * 2 - 1),
		input.Bitangent.xyz * 2.0.xxx + -1.0.xxx,
		input.Normal.xyz * 2.0.xxx + -1.0.xxx);
	float3x3 tbnTr = transpose(tbn);

#		if defined(SKINNED)
	float3x3 worldTbnTr = transpose(mul(transpose(tbnTr), transpose(boneRSMatrix)));
	float3x3 worldTbnTrTr = transpose(worldTbnTr);
	worldTbnTrTr[0] = normalize(worldTbnTrTr[0]);
	worldTbnTrTr[1] = normalize(worldTbnTrTr[1]);
	worldTbnTrTr[2] = normalize(worldTbnTrTr[2]);
	worldTbnTr = transpose(worldTbnTrTr);
	vsout.TBN0.xyz = worldTbnTr[0];
	vsout.TBN1.xyz = worldTbnTr[1];
	vsout.TBN2.xyz = worldTbnTr[2];
#		elif defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX)
	vsout.TBN0.xyz = mul(tbn, World[eyeIndex][0].xyz);
	vsout.TBN1.xyz = mul(tbn, World[eyeIndex][1].xyz);
	vsout.TBN2.xyz = mul(tbn, World[eyeIndex][2].xyz);
	float3x3 tempTbnTr = transpose(float3x3(vsout.TBN0.xyz, vsout.TBN1.xyz, vsout.TBN2.xyz));
	tempTbnTr[0] = normalize(tempTbnTr[0]);
	tempTbnTr[1] = normalize(tempTbnTr[1]);
	tempTbnTr[2] = normalize(tempTbnTr[2]);
	tempTbnTr = transpose(tempTbnTr);
	vsout.TBN0.xyz = tempTbnTr[0];
	vsout.TBN1.xyz = tempTbnTr[1];
	vsout.TBN2.xyz = tempTbnTr[2];
#		else
	vsout.TBN0.xyz = tbnTr[0];
	vsout.TBN1.xyz = tbnTr[1];
	vsout.TBN2.xyz = tbnTr[2];
#		endif
#	elif defined(SKINNED)
	float3x3 boneRSMatrixTr = transpose(boneRSMatrix);
	float3x3 worldTbnTr = transpose(float3x3(normalize(boneRSMatrixTr[0]),
		normalize(boneRSMatrixTr[1]), normalize(boneRSMatrixTr[2])));

	vsout.TBN0.xyz = worldTbnTr[0];
	vsout.TBN1.xyz = worldTbnTr[1];
	vsout.TBN2.xyz = worldTbnTr[2];
#	endif

#	if defined(LANDSCAPE)
	vsout.LandBlendWeights1 = input.LandBlendWeights1;

	float2 gridOffset = LandBlendParams.zw - input.Position.xy;
	vsout.LandBlendWeights2.w = 1 - saturate(0.000375600968 * (9625.59961 - length(gridOffset)));
	vsout.LandBlendWeights2.xyz = input.LandBlendWeights2.xyz;
#	elif defined(PROJECTED_UV) && !defined(SKINNED)
	vsout.TexProj = TextureProj[eyeIndex][2].xyz;
#	endif

#	if defined(HAS_VIEW_VECTOR)
#		if defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(SKINNED)
	vsout.ViewVector = EyePosition[eyeIndex].xyz - worldPosition.xyz;
#		else
	vsout.ViewVector = EyePosition[eyeIndex].xyz - input.Position.xyz;
#		endif
#	endif  // HAS_VIEW_VECTOR

#	if defined(EYE)
	precise float4 modelEyeCenter = float4(LeftEyeCenter.xyz + input.EyeParameter.xxx * (RightEyeCenter.xyz - LeftEyeCenter.xyz), 1);
	vsout.EyeNormal.xyz = normalize(worldPosition.xyz - mul(modelEyeCenter, transpose(worldMatrix)));
#	endif  // EYE

#	if defined(SKINNED)
	float3x3 ScreenNormalTransform = mul(ScreenProj[eyeIndex], worldTbnTr);

	vsout.ScreenNormalTransform0.xyz = ScreenNormalTransform[0];
	vsout.ScreenNormalTransform1.xyz = ScreenNormalTransform[1];
	vsout.ScreenNormalTransform2.xyz = ScreenNormalTransform[2];
#	else
	float3x4 transMat = mul(ScreenProj[eyeIndex], World[eyeIndex]);

#		if defined(MODELSPACENORMALS)
	vsout.ScreenNormalTransform0.xyz = transMat[0].xyz;
	vsout.ScreenNormalTransform1.xyz = transMat[1].xyz;
	vsout.ScreenNormalTransform2.xyz = transMat[2].xyz;
#		else
	vsout.ScreenNormalTransform0.xyz = mul(transMat[0].xyz, transpose(tbn));
	vsout.ScreenNormalTransform1.xyz = mul(transMat[1].xyz, transpose(tbn));
	vsout.ScreenNormalTransform2.xyz = mul(transMat[2].xyz, transpose(tbn));
#		endif  // MODELSPACENORMALS
#	endif      // SKINNED

	vsout.WorldPosition = worldPosition;
	vsout.PreviousWorldPosition = previousWorldPosition;

#	if defined(VC)
	vsout.Color = input.Color;
#	else
	vsout.Color = 1.0.xxxx;
#	endif  // VC

	float fogColorParam = min(FogParam.w,
		exp2(FogParam.z * log2(saturate(length(viewPos.xyz) * FogParam.y - FogParam.x))));

	vsout.FogParam.xyz = lerp(FogNearColor.xyz, FogFarColor.xyz, fogColorParam);
	vsout.FogParam.w = fogColorParam;

#	ifdef VR
	float4 r0;
	float4 projSpacePosition = viewPos;
	r0.xyzw = 0;
	if (0 < cb13[0].y) {
		r0.yz = dot(projSpacePosition, cb13[eyeIndex + 1].xyzw);
	} else {
		r0.yz = float2(1, 1);
	}

	r0.w = 2 + -cb13[0].y;
	r0.x = dot(cb13[0].zw, M_IdentityMatrix[eyeIndex + 0].xy);
	r0.xw = r0.xw * projSpacePosition.wx;
	r0.x = cb13[0].y * r0.x;

	vsout.Position.x = r0.w * 0.5 + r0.x;
	vsout.Position.yzw = projSpacePosition.yzw;

	vsout.ClipDistance.x = r0.z;
	vsout.CullDistance.x = r0.y;
#	endif  // VR
	return vsout;
}
#endif  // VSHADER

typedef VS_OUTPUT PS_INPUT;

struct PS_OUTPUT
{
	float4 Albedo : SV_Target0;
	float4 MotionVectors : SV_Target1;
	float4 ScreenSpaceNormals : SV_Target2;
#if defined(SNOW)
	float2 SnowParameters : SV_Target3;
#endif
};

#ifdef PSHADER

SamplerState SampTerrainParallaxSampler : register(s1);

#	if defined(LANDSCAPE)

SamplerState SampColorSampler : register(s0);

#		define SampLandColor2Sampler SampColorSampler
#		define SampLandColor3Sampler SampColorSampler
#		define SampLandColor4Sampler SampColorSampler
#		define SampLandColor5Sampler SampColorSampler
#		define SampLandColor6Sampler SampColorSampler
#		define SampNormalSampler SampColorSampler
#		define SampLandNormal2Sampler SampColorSampler
#		define SampLandNormal3Sampler SampColorSampler
#		define SampLandNormal4Sampler SampColorSampler
#		define SampLandNormal5Sampler SampColorSampler
#		define SampLandNormal6Sampler SampColorSampler

// SamplerState SampLandColor2Sampler : register(s1);
// SamplerState SampLandColor3Sampler : register(s2);
// SamplerState SampLandColor4Sampler : register(s3);
// SamplerState SampLandColor5Sampler : register(s4);
// SamplerState SampLandColor6Sampler : register(s5);
// SamplerState SampNormalSampler : register(s7);
// SamplerState SampLandNormal2Sampler : register(s8);
// SamplerState SampLandNormal3Sampler : register(s9);
// SamplerState SampLandNormal4Sampler : register(s10);
// SamplerState SampLandNormal5Sampler : register(s11);
// SamplerState SampLandNormal6Sampler : register(s12);

#	else
SamplerState SampColorSampler : register(s0);

#		define SampNormalSampler SampColorSampler
// SamplerState SampNormalSampler : register(s1);

#		if defined(MODELSPACENORMALS) && !defined(LODLANDNOISE)
SamplerState SampSpecularSampler : register(s2);
#		endif
#		if defined(FACEGEN)
SamplerState SampTintSampler : register(s3);
SamplerState SampDetailSampler : register(s4);
#		elif defined(PARALLAX)
SamplerState SampParallaxSampler : register(s3);
#		elif defined(PROJECTED_UV) && !defined(SPARKLE)
SamplerState SampProjDiffuseSampler : register(s3);
#		endif
#		if defined(PBR)
SamplerState SampRMAOSampler : register(s6);
#		endif
#		if (defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(SNOW_FLAG) || defined(EYE)) && !defined(FACEGEN)
SamplerState SampEnvSampler : register(s4);
SamplerState SampEnvMaskSampler : register(s5);
#		endif
#		if defined(GLOWMAP)
SamplerState SampGlowSampler : register(s6);
#		endif
#		if defined(MULTI_LAYER_PARALLAX)
SamplerState SampLayerSampler : register(s8);
#		elif defined(PROJECTED_UV) && !defined(SPARKLE)
SamplerState SampProjNormalSampler : register(s8);
#		endif
#		if defined(BACK_LIGHTING)
SamplerState SampBackLightSampler : register(s9);
#		endif
#		if defined(PROJECTED_UV)
SamplerState SampProjDetailSampler : register(s10);
#		endif
#		if defined(CHARACTER_LIGHT)
SamplerState SampCharacterLightSampler : register(s11);
#		elif defined(PROJECTED_UV)
SamplerState SampProjNoiseSampler : register(s11);
#		endif
#		if defined(SOFT_LIGHTING) || defined(RIM_LIGHTING)
SamplerState SampRimSoftLightSampler : register(s12);
#		elif defined(WORLD_MAP)
SamplerState SampWorldMapOverlaySampler : register(s12);
#		endif
#		if defined(WORLD_MAP) && (defined(LODLANDSCAPE) || defined(LODLANDNOISE))
SamplerState SampWorldMapOverlaySnowSampler : register(s13);
#		endif

#	endif

#	if defined(LOD_LAND_BLEND)
SamplerState SampLandLodBlend1Sampler : register(s13);
SamplerState SampLandLodBlend2Sampler : register(s15);
#	elif defined(LODLANDNOISE)
SamplerState SampLandLodNoiseSampler : register(s15);
#	endif
#	if defined(DEFSHADOW)
SamplerState SampShadowMaskSampler : register(s14);
#	endif

#	if defined(LANDSCAPE)

Texture2D<float4> TexColorSampler : register(t0);
Texture2D<float4> TexLandColor2Sampler : register(t1);
Texture2D<float4> TexLandColor3Sampler : register(t2);
Texture2D<float4> TexLandColor4Sampler : register(t3);
Texture2D<float4> TexLandColor5Sampler : register(t4);
Texture2D<float4> TexLandColor6Sampler : register(t5);
Texture2D<float4> TexNormalSampler : register(t7);
Texture2D<float4> TexLandNormal2Sampler : register(t8);
Texture2D<float4> TexLandNormal3Sampler : register(t9);
Texture2D<float4> TexLandNormal4Sampler : register(t10);
Texture2D<float4> TexLandNormal5Sampler : register(t11);
Texture2D<float4> TexLandNormal6Sampler : register(t12);

#	else

Texture2D<float4> TexColorSampler : register(t0);
Texture2D<float4> TexNormalSampler : register(t1);  // normal in xyz, glossiness in w if not modelspacenormal

#		if defined(MODELSPACENORMALS) && !defined(LODLANDNOISE)
Texture2D<float4> TexSpecularSampler : register(t2);
#		endif
#		if defined(FACEGEN)
Texture2D<float4> TexTintSampler : register(t3);
Texture2D<float4> TexDetailSampler : register(t4);
#		elif defined(PARALLAX)
Texture2D<float4> TexParallaxSampler : register(t3);
#		elif defined(PROJECTED_UV) && !defined(SPARKLE)
Texture2D<float4> TexProjDiffuseSampler : register(t3);
#		endif
#		if defined(PBR)
Texture2D<float4> TexRMAOSampler : register(t6);
#		endif
#		if (defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(SNOW_FLAG) || defined(EYE)) && !defined(FACEGEN)
TextureCube<float4> TexEnvSampler : register(t4);
Texture2D<float4> TexEnvMaskSampler : register(t5);
#		endif
#		if defined(GLOWMAP)
Texture2D<float4> TexGlowSampler : register(t6);
#		endif
#		if defined(MULTI_LAYER_PARALLAX)
Texture2D<float4> TexLayerSampler : register(t8);
#		elif defined(PROJECTED_UV) && !defined(SPARKLE)
Texture2D<float4> TexProjNormalSampler : register(t8);
#		endif
#		if defined(BACK_LIGHTING)
Texture2D<float4> TexBackLightSampler : register(t9);
#		endif
#		if defined(PROJECTED_UV)
Texture2D<float4> TexProjDetail : register(t10);
#		endif
#		if defined(CHARACTER_LIGHT)
Texture2D<float4> TexCharacterLightSampler : register(t11);
#		elif defined(PROJECTED_UV)
Texture2D<float4> TexProjNoiseSampler : register(t11);
#		endif
#		if defined(SOFT_LIGHTING) || defined(RIM_LIGHTING)
Texture2D<float4> TexRimSoftLightSampler : register(t12);
#		elif defined(WORLD_MAP)
Texture2D<float4> TexWorldMapOverlaySampler : register(t12);
#		endif
#		if defined(WORLD_MAP) && (defined(LODLANDSCAPE) || defined(LODLANDNOISE))
Texture2D<float4> TexWorldMapOverlaySnowSampler : register(t13);
#		endif

#	endif

#	if defined(LOD_LAND_BLEND)
Texture2D<float4> TexLandLodBlend1Sampler : register(t13);
Texture2D<float4> TexLandLodBlend2Sampler : register(t15);
#	elif defined(LODLANDNOISE)
Texture2D<float4> TexLandLodNoiseSampler : register(t15);
#	endif
#	if defined(DEFSHADOW)
Texture2D<float4> TexShadowMaskSampler : register(t14);
#	endif

cbuffer PerTechnique : register(b0)
{
	float4 FogColor : packoffset(c0);           // Color in xyz, invFrameBufferRange in w
	float4 ColourOutputClamp : packoffset(c1);  // fLightingOutputColourClampPostLit in x, fLightingOutputColourClampPostEnv in y, fLightingOutputColourClampPostSpec in z
	float4 VPOSOffset : packoffset(c2);         // ???
};

cbuffer PerMaterial : register(b1)
{
	float4 LODTexParams : packoffset(c0);  // TerrainTexOffset in xy, LodBlendingEnabled in z
	float4 TintColor : packoffset(c1);
	float4 EnvmapData : packoffset(c2);  // fEnvmapScale in x, 1 or 0 in y depending of if has envmask
	float4 ParallaxOccData : packoffset(c3);
	float4 SpecularColor : packoffset(c4);  // Shininess in w, color in xyz
	float4 SparkleParams : packoffset(c5);
	float4 MultiLayerParallaxData : packoffset(c6);  // Layer thickness in x, refraction scale in y, uv scale in zw
	float4 LightingEffectParams : packoffset(c7);    // fSubSurfaceLightRolloff in x, fRimLightPower in y
	float4 IBLParams : packoffset(c8);
	float4 LandscapeTexture1to4IsSnow : packoffset(c9);
	float4 LandscapeTexture5to6IsSnow : packoffset(c10);  // bEnableSnowMask in z, inverse iLandscapeMultiNormalTilingFactor in w
	float4 LandscapeTexture1to4IsSpecPower : packoffset(c11);
	float4 LandscapeTexture5to6IsSpecPower : packoffset(c12);
	float4 SnowRimLightParameters : packoffset(c13);  // fSnowRimLightIntensity in x, fSnowGeometrySpecPower in y, fSnowNormalSpecPower in z, bEnableSnowRimLighting in w
	float4 CharacterLightParams : packoffset(c14);
	// VR is [9] instead of [15]
};

cbuffer PerGeometry : register(b2)
{
#	if !defined(VR)
	float3 DirLightDirection : packoffset(c0);
	float3 DirLightColor : packoffset(c1);
	float4 ShadowLightMaskSelect : packoffset(c2);
	float4 MaterialData : packoffset(c3);  // envmapLODFade in x, specularLODFade in y, alpha in z
	float AlphaTestRef : packoffset(c4);
	float3 EmitColor : packoffset(c4.y);
	float4 ProjectedUVParams : packoffset(c6);
	float4 SSRParams : packoffset(c7);
	float4 WorldMapOverlayParametersPS : packoffset(c8);
	float4 ProjectedUVParams2 : packoffset(c9);
	float4 ProjectedUVParams3 : packoffset(c10);  // fProjectedUVDiffuseNormalTilingScale in x, fProjectedUVNormalDetailTilingScale in y, EnableProjectedNormals in w
	row_major float3x4 DirectionalAmbient : packoffset(c11);
	float4 AmbientSpecularTintAndFresnelPower : packoffset(c14);  // Fresnel power in z, color in xyz
	float4 PointLightPosition[7] : packoffset(c15);               // point light radius in w
	float4 PointLightColor[7] : packoffset(c22);
	float2 NumLightNumShadowLight : packoffset(c29);
#	else
	// VR is [49] instead of [30]
	float3 DirLightDirection : packoffset(c0);
	float4 UnknownPerGeometry[12] : packoffset(c1);
	float3 DirLightColor : packoffset(c13);
	float4 ShadowLightMaskSelect : packoffset(c14);
	float4 MaterialData : packoffset(c15);  // envmapLODFade in x, specularLODFade in y, alpha in z
	float AlphaTestRef : packoffset(c16);
	float3 EmitColor : packoffset(c16.y);
	float4 ProjectedUVParams : packoffset(c18);
	float4 SSRParams : packoffset(c19);
	float4 WorldMapOverlayParametersPS : packoffset(c20);
	float4 ProjectedUVParams2 : packoffset(c21);
	float4 ProjectedUVParams3 : packoffset(c22);  // fProjectedUVDiffuseNormalTilingScale in x,	fProjectedUVNormalDetailTilingScale in y, EnableProjectedNormals in w
	row_major float3x4 DirectionalAmbient : packoffset(c23);
	float4 AmbientSpecularTintAndFresnelPower : packoffset(c26);  // Fresnel power in z, color in xyz
	float4 PointLightPosition[14] : packoffset(c27);              // point light radius in w
	float4 PointLightColor[7] : packoffset(c41);
	float2 NumLightNumShadowLight : packoffset(c48);
#	endif  // VR
};

cbuffer AlphaTestRefBuffer : register(b11)
{
	float AlphaThreshold : packoffset(c0);
}

#	ifdef VR
cbuffer cb13 : register(b13)
{
	float4 cb13[3];
}
#	endif  // VR

float GetSoftLightMultiplier(float angle)
{
	float softLightParam = saturate((LightingEffectParams.x + angle) / (1 + LightingEffectParams.x));
	float arg1 = (softLightParam * softLightParam) * (3 - 2 * softLightParam);
	float clampedAngle = saturate(angle);
	float arg2 = (clampedAngle * clampedAngle) * (3 - 2 * clampedAngle);
	float softLigtMul = saturate(arg1 - arg2);
	return softLigtMul;
}

float GetRimLightMultiplier(float3 L, float3 V, float3 N)
{
	float NdotV = saturate(dot(N, V));
	return exp2(LightingEffectParams.y * log2(1 - NdotV)) * saturate(dot(V, -L));
}

float ProcessSparkleColor(float color)
{
	return exp2(SparkleParams.y * log2(min(1, abs(color))));
}

float3 GetLightSpecularInput(PS_INPUT input, float3 L, float3 V, float3 N, float3 lightColor, float shininess, float2 uv)
{
	float3 H = normalize(V + L);
#	if defined(ANISO_LIGHTING)
	float3 AN = normalize(N * 0.5 + float3(input.TBN0.z, input.TBN1.z, input.TBN2.z));
	float LdotAN = dot(AN, L);
	float HdotAN = dot(AN, H);
	float HdotN = 1 - min(1, abs(LdotAN - HdotAN));
#	else
	float HdotN = saturate(dot(H, N));
#	endif
#	if defined(SPECULAR)
	float lightColorMultiplier = exp2(shininess * log2(HdotN));
#	elif defined(SPARKLE)
	float lightColorMultiplier = 0;
#	else
	float lightColorMultiplier = HdotN;
#	endif
#	if defined(ANISO_LIGHTING)
	lightColorMultiplier *= 0.7 * max(0, L.z);
#	endif
#	if defined(SPARKLE) && !defined(SNOW)
	float3 sparkleUvScale = exp2(float3(1.3, 1.6, 1.9) * log2(abs(SparkleParams.x)).xxx);

	float sparkleColor1 = TexProjDetail.Sample(SampProjDetailSampler, uv * sparkleUvScale.xx).z;
	float sparkleColor2 = TexProjDetail.Sample(SampProjDetailSampler, uv * sparkleUvScale.yy).z;
	float sparkleColor3 = TexProjDetail.Sample(SampProjDetailSampler, uv * sparkleUvScale.zz).z;
	float sparkleColor = ProcessSparkleColor(sparkleColor1) + ProcessSparkleColor(sparkleColor2) + ProcessSparkleColor(sparkleColor3);
	float VdotN = dot(V, N);
	V += N * -(2 * VdotN);
	float sparkleMultiplier = exp2(SparkleParams.w * log2(saturate(dot(V, -L)))) * (SparkleParams.z * sparkleColor);
	sparkleMultiplier = sparkleMultiplier >= 0.5 ? 1 : 0;
	lightColorMultiplier += sparkleMultiplier * HdotN;
#	endif
	return lightColor * lightColorMultiplier.xxx;
}

float3 TransformNormal(float3 normal)
{
	return normal * 2 + -1.0.xxx;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1 - F0) * pow(saturate(1 - cosTheta), 5);
}

static const float PI = 3.14159265;

float DistributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1) + 1);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float cosTheta, float roughness)
{
	float r = (roughness + 1);
	float k = (r * r) / 8;

	float num = cosTheta;
	float denom = cosTheta * (1 - k) + k;

	return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggxV = GeometrySchlickGGX(NdotV, roughness);
	float ggxL = GeometrySchlickGGX(NdotL, roughness);

	return ggxV * ggxL;
}

float OrenNayarDiffuseCoefficient(float roughness, float3 N, float3 L, float3 V, float NdotL, float NdotV)
{
	float gamma = dot(V - N * NdotV, L - N * NdotL);
	float a = roughness * roughness;
	float A = 1 - 0.5 * (a / (a + 0.57));
	float B = 0.45 * (a / (a + 0.09));
	float C = sqrt((1 - NdotV * NdotV) * (1 - NdotL * NdotL)) / max(NdotV, NdotL);
	return (A + B * max(0.0f, gamma) * C) / PI;
}

float3 GetLightRadiance(float3 N, float3 L, float3 V, float3 F0, float3 originalRadiance, float3 albedo, float roughness, float metallic)
{
	float3 radiance = PI * originalRadiance;

	float3 H = normalize(V + L);
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);
	float HdotV = dot(H, V);
	float NdotH = dot(N, H);

	float NDF = DistributionGGX(saturate(NdotH), roughness);
	float G = GeometrySmith(saturate(NdotV), saturate(NdotL), roughness);
	float3 F = fresnelSchlick(saturate(HdotV), F0);

	float3 kD = (1 - F) * (1 - metallic);

	float3 numerator = NDF * G * F;
	float denominator = 4 * saturate(NdotV) * saturate(NdotL) + 0.0001;
	float3 specular = numerator / denominator;

	float diffuseValue = OrenNayarDiffuseCoefficient(roughness, N, L, V, NdotL, NdotV);
	return (kD * diffuseValue * albedo + specular) * saturate(NdotL) * radiance;
}

float GetLodLandBlendParameter(float3 color)
{
	float result = saturate(1.6666666 * (dot(color, 0.55.xxx) - 0.4));
	result = ((result * result) * (3 - result * 2));
#	if !defined(WORLD_MAP)
	result *= 0.8;
#	endif
	return result;
}

float GetLodLandBlendMultiplier(float parameter, float mask)
{
	return 0.8333333 * (parameter * (0.37 - mask) + mask) + 0.37;
}

float GetLandSnowMaskValue(float alpha)
{
	return alpha * LandscapeTexture5to6IsSnow.z + (1 + -LandscapeTexture5to6IsSnow.z);
}

float3 GetLandNormal(float landSnowMask, float3 normal, float2 uv, SamplerState sampNormal, Texture2D<float4> texNormal)
{
	float3 landNormal = TransformNormal(normal);
#	if defined(SNOW)
	if (landSnowMask > 1e-5 && LandscapeTexture5to6IsSnow.w != 1.0) {
		float3 snowNormal =
			float3(-1, -1, 1) *
			TransformNormal(texNormal.Sample(sampNormal, LandscapeTexture5to6IsSnow.ww * uv).xyz);
		landNormal.z += 1;
		float normalProjection = dot(landNormal, snowNormal);
		snowNormal = landNormal * normalProjection.xxx - snowNormal * landNormal.z;
		return normalize(snowNormal);
	} else {
		return landNormal;
	}
#	else
	return landNormal;
#	endif
}

#	if defined(SNOW)
float3 GetSnowSpecularColor(PS_INPUT input, float3 modelNormal, float3 viewDirection)
{
	if (SnowRimLightParameters.w > 1e-5) {
#		if defined(MODELSPACENORMALS) && !defined(SKINNED)
#			if defined(HAS_VIEW_VECTOR)
		float3 modelGeometryNormal = float3(0, 0, 1);
#			else
		float3 modelGeometryNormal = 0.333333333.xxx;
#			endif
#		else
		float3 modelGeometryNormal = normalize(float3(input.TBN0.z, input.TBN1.z, input.TBN2.z));
#		endif
		float normalFactor = 1 - saturate(dot(modelNormal, viewDirection));
		float geometryNormalFactor = 1 - saturate(dot(modelGeometryNormal, viewDirection));
		return (SnowRimLightParameters.x * (exp2(SnowRimLightParameters.y * log2(geometryNormalFactor)) * exp2(SnowRimLightParameters.z * log2(normalFactor)))).xxx;
	} else {
		return 0.0.xxx;
	}
}
#	endif

#	if defined(FACEGEN)
float3 GetFacegenBaseColor(float3 rawBaseColor, float2 uv)
{
	float3 detailColor = TexDetailSampler.Sample(SampDetailSampler, uv).xyz;
	detailColor = float3(3.984375, 3.984375, 3.984375) * (float3(0.00392156886, 0, 0.00392156886) + detailColor);
	float3 tintColor = TexTintSampler.Sample(SampTintSampler, uv).xyz;
	tintColor = tintColor * rawBaseColor * 2.0.xxx;
	tintColor = tintColor - tintColor * rawBaseColor;
	return (rawBaseColor * rawBaseColor + tintColor) * detailColor;
}
#	endif

#	if defined(FACEGEN_RGB_TINT)
float3 GetFacegenRGBTintBaseColor(float3 rawBaseColor, float2 uv)
{
	float3 tintColor = TintColor.xyz * rawBaseColor * 2.0.xxx;
	tintColor = tintColor - tintColor * rawBaseColor;
	return float3(1.01171875, 0.99609375, 1.01171875) * (rawBaseColor * rawBaseColor + tintColor);
}
#	endif

#	if defined(WORLD_MAP)
float3 GetWorldMapNormal(PS_INPUT input, float3 rawNormal, float3 baseColor)
{
	float3 normal = normalize(rawNormal);
#		if defined(MODELSPACENORMALS)
	float3 worldMapNormalSrc = normal.xyz;
#		else
	float3 worldMapNormalSrc = float3(input.TBN0.z, input.TBN1.z, input.TBN2.z);
#		endif
	float3 worldMapNormal = 7.0.xxx * (-0.2.xxx + abs(normalize(worldMapNormalSrc)));
	worldMapNormal = max(0.01.xxx, worldMapNormal * worldMapNormal * worldMapNormal);
	worldMapNormal /= dot(worldMapNormal, 1.0.xxx);
	float3 worldMapColor1 = TexWorldMapOverlaySampler.Sample(SampWorldMapOverlaySampler, WorldMapOverlayParametersPS.xx * input.InputPosition.yz).xyz;
	float3 worldMapColor2 = TexWorldMapOverlaySampler.Sample(SampWorldMapOverlaySampler, WorldMapOverlayParametersPS.xx * input.InputPosition.xz).xyz;
	float3 worldMapColor3 = TexWorldMapOverlaySampler.Sample(SampWorldMapOverlaySampler, WorldMapOverlayParametersPS.xx * input.InputPosition.xy).xyz;
#		if defined(LODLANDNOISE) || defined(LODLANDSCAPE)
	float3 worldMapSnowColor1 = TexWorldMapOverlaySnowSampler.Sample(SampWorldMapOverlaySnowSampler, WorldMapOverlayParametersPS.ww * input.InputPosition.yz).xyz;
	float3 worldMapSnowColor2 = TexWorldMapOverlaySnowSampler.Sample(SampWorldMapOverlaySnowSampler, WorldMapOverlayParametersPS.ww * input.InputPosition.xz).xyz;
	float3 worldMapSnowColor3 = TexWorldMapOverlaySnowSampler.Sample(SampWorldMapOverlaySnowSampler, WorldMapOverlayParametersPS.ww * input.InputPosition.xy).xyz;
#		endif
	float3 worldMapColor = worldMapNormal.xxx * worldMapColor1 + worldMapNormal.yyy * worldMapColor2 + worldMapNormal.zzz * worldMapColor3;
#		if defined(LODLANDNOISE) || defined(LODLANDSCAPE)
	float3 worldMapSnowColor = worldMapSnowColor1 * worldMapNormal.xxx + worldMapSnowColor2 * worldMapNormal.yyy + worldMapSnowColor3 * worldMapNormal.zzz;
	float snowMultiplier = GetLodLandBlendParameter(baseColor);
	worldMapColor = snowMultiplier * (worldMapSnowColor - worldMapColor) + worldMapColor;
#		endif
	worldMapColor = normalize(2.0.xxx * (-0.5.xxx + (worldMapColor)));
#		if defined(LODLANDNOISE) || defined(LODLANDSCAPE)
	float worldMapLandTmp = saturate(19.9999962 * (rawNormal.z - 0.95));
	worldMapLandTmp = saturate(-(worldMapLandTmp * worldMapLandTmp) * (worldMapLandTmp * -2 + 3) + 1.5);
	float3 worldMapLandTmp1 = normalize(normal.zxy * float3(1, 0, 0) - normal.yzx * float3(0, 0, 1));
	float3 worldMapLandTmp2 = normalize(worldMapLandTmp1.yzx * normal.zxy - worldMapLandTmp1.zxy * normal.yzx);
	float3 worldMapLandTmp3 = normalize(worldMapColor.xxx * worldMapLandTmp1 + worldMapColor.yyy * worldMapLandTmp2 + worldMapColor.zzz * normal.xyz);
	float worldMapLandTmp4 = dot(worldMapLandTmp3, worldMapLandTmp3);
	if (worldMapLandTmp4 > 0.999 && worldMapLandTmp4 < 1.001) {
		normal.xyz = worldMapLandTmp * (worldMapLandTmp3 - normal.xyz) + normal.xyz;
	}
#		else
	normal.xyz = normalize(
		WorldMapOverlayParametersPS.zzz * (rawNormal.xyz - worldMapColor.xyz) + worldMapColor.xyz);
#		endif
	return normal;
}

float3 GetWorldMapBaseColor(float3 originalBaseColor, float3 rawBaseColor, float texProjTmp)
{
#		if defined(LODOBJECTS) && !defined(PROJECTED_UV)
	return rawBaseColor;
#		endif
#		if defined(LODLANDSCAPE) || defined(LODOBJECTSHD) || defined(LODLANDNOISE)
	float lodMultiplier = GetLodLandBlendParameter(originalBaseColor.xyz);
#		elif defined(LODOBJECTS) && defined(PROJECTED_UV)
	float lodMultiplier = saturate(10 * texProjTmp);
#		else
	float lodMultiplier = 1;
#		endif
#		if defined(LODOBJECTS)
	float4 lodColorMul = lodMultiplier.xxxx * float4(0.269999981, 0.281000018, 0.441000015, 0.441000015) + float4(0.0780000091, 0.09799999, -0.0349999964, 0.465000004);
	float4 lodColor = lodColorMul.xyzw * 2.0.xxxx;
	bool useLodColorZ = lodColorMul.w > 0.5;
	lodColor.xyz = max(lodColor.xyz, rawBaseColor.xyz);
	lodColor.w = useLodColorZ ? lodColor.z : min(lodColor.w, rawBaseColor.z);
	return (0.5 * lodMultiplier).xxx * (lodColor.xyw - rawBaseColor.xyz) + rawBaseColor;
#		else
	float4 lodColorMul = lodMultiplier.xxxx * float4(0.199999988, 0.441000015, 0.269999981, 0.281000018) + float4(0.300000012, 0.465000004, 0.0780000091, 0.09799999);
	float3 lodColor = lodColorMul.zwy * 2.0.xxx;
	lodColor.xy = max(lodColor.xy, rawBaseColor.xy);
	lodColor.z = lodColorMul.y > 0.5 ? max((lodMultiplier * 0.441 + -0.0349999964) * 2, rawBaseColor.z) : min(lodColor.z, rawBaseColor.z);
	return lodColorMul.xxx * (lodColor - rawBaseColor.xyz) + rawBaseColor;
#		endif
}
#	endif

float GetSnowParameterY(float texProjTmp, float alpha)
{
#	if defined(BASE_OBJECT_IS_SNOW)
	return min(1, texProjTmp + alpha);
#	else
	return texProjTmp;
#	endif
}

#	if defined(COMPLEX_PARALLAX_MATERIALS) && (defined(PARALLAX) || defined(LANDSCAPE) || defined(ENVMAP))
#		define CPM_AVAILABLE
#	endif

#	if defined(CPM_AVAILABLE)
#		include "ComplexParallaxMaterials/ComplexParallaxMaterials.hlsli"
#	endif

#	if defined(SCREEN_SPACE_SHADOWS)
#		include "ScreenSpaceShadows/ShadowsPS.hlsli"
#	endif

#	if defined(WATER_BLENDING)
#		include "WaterBlending/WaterBlending.hlsli"
#	endif

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT psout;

#	if !defined(VR)
	uint eyeIndex = 0;
	uint eyeIndexX3 = 0;
	uint eyeIndexX4 = 0;
#	else
	// this code appears in parallax code in the PShader,
	// https://github.com/alandtse/SSEShaderTools/commit/450a0d62d01b0cbdfeb86b4eba46c3528833c897?diff=split#diff-1927ad1d541f8de9480b08bd5e6878a9a56b64d06068e1724e4a5792c663c87bR71

	// calculate eyeindex
	// r0.xy = HPosition.xy * cb0[2].xy + cb0[2].zw;
	// r0.x = cb12[86].x * r0.x;
	// r0.z = cmp(r0.x >= 0.5);
	// r0.z = r0.z ? 1.000000 : 0;  // eyeIndex
	// r0.w = (uint)cb13[0].y;
	// r0.z = (int)r0.w * (int)r0.z;
	// r1.x = (uint)r0.z;
	// r1.y = -r1.x * 0.5 + r0.x;
	// r1.y = r1.y + r1.y;
	// r2.x = r0.w ? r1.y : r0.x;
	// r2.y = -r0.y * cb12[86].y + 1;
	// r2.xy = r2.xy * float2(2, 2) + float2(-1, -1);
	// r0.x = (uint)r0.z << 2;
	// r2.z = v0.z;
	// r2.w = 1;
	// r3.x = dot(cb12[r0.x + 64].xyzw, r2.xyzw);
	// r3.y = dot(cb12[r0.x + 65].xyzw, r2.xyzw);
	// r3.z = dot(cb12[r0.x + 66].xyzw, r2.xyzw);
	// r0.y = dot(cb12[r0.x + 67].xyzw, r2.xyzw);
	// r2.xyz = r3.xyz / r0.yyy;
	// r0.y = (int)r0.z * 3;  // should be fixed at 0 to avoid specular lightin issues
	// r2.w = 1;
	// r3.x = dot(cb2[r0.y + 1].xyzw, r2.xyzw);
	// r3.y = dot(cb2[r0.y + 2].xyzw, r2.xyzw);
	// r3.z = dot(cb2[r0.y + 3].xyzw, r2.xyzw);
	// r3.w = 1;
	// r4.x = dot(cb2[r0.y + 7].xyzw, r3.xyzw);
	// r4.y = dot(cb2[r0.y + 8].xyzw, r3.xyzw);
	// r4.z = dot(cb2[r0.y + 9].xyzw, r3.xyzw);
	// see also
	// r0.x = v0.x * cb0[2].x + cb0[2].z;
	// r0.x = cb12[86].x * r0.x;
	// r0.x = cmp(r0.x >= 0.5);
	// r0.x = r0.x ? 0.000000 : 0; // note this is broken VR code, it needs to
	// return 1 on true.
	// r0.y = (uint)cb13[0].y; r0.x = (int)r0.y * (int)r0.x;
	//
	float4 r0, r1, r3, stereoUV;
	stereoUV.xy = input.Position.xy * VPOSOffset.xy + VPOSOffset.zw;
	stereoUV.x = DynamicResolutionParams2.x * stereoUV.x;
	stereoUV.x = (stereoUV.x >= 0.5);
	uint eyeIndex =
		(uint)(((int)((uint)cb13[0].y)) *
			   (int)stereoUV.x);  // this may be eyeOffset from RunGrass.hsl
	uint eyeIndexX3 = eyeIndex * 3;
	uint eyeIndexX4 = eyeIndex << 2;

	// In VR, there is no worldPosition or PreviousWorldPosition as an input. This code is used to determine position
	// float4 worldPositionVR;
	// worldPositionVR.x =
	// 	(uint)cb13[0].y ? 2 * (-eyeIndex * 0.5 + stereoUV.x) : stereoUV.x;
	// worldPositionVR.y = -stereoUV.y * DynamicResolutionParams2.y + 1;
	// worldPositionVR.xy = worldPositionVR.xy * float2(2, 2) + float2(-1, -1);
	// worldPositionVR.z = input.Position.z;
	// worldPositionVR.w = 1;
	// r3.x = dot(CameraViewProjInverse[eyeIndex][0].xyzw, worldPositionVR.xyzw);
	// r3.y = dot(CameraViewProjInverse[eyeIndex][1].xyzw, worldPositionVR.xyzw);
	// r3.z = dot(CameraViewProjInverse[eyeIndex][2].xyzw, worldPositionVR.xyzw);
	// r0.y = dot(CameraViewProjInverse[eyeIndex][3].xyzw, worldPositionVR.xyzw);
	// worldPositionVR.xyz = r3.xyz / r0.yyy;
	// worldPositionVR.w = 1;
	// note in RE this was eyeIndexX3 for accessing UnknownPerEGeomtry;
	// but that was causing a specular bug in the right eye. Resolved with
	// https://github.com/alandtse/SSEShaderTools/commit/731f1d126319eecaf0ed9178c8b77e22926cf3f9
	// r3.x = dot(UnknownPerGeometry[0].xyzw, worldPositionVR.xyzw);
	// r3.y = dot(UnknownPerGeometry[1].xyzw, worldPositionVR.xyzw);
	// r3.z = dot(UnknownPerGeometry[2].xyzw, worldPositionVR.xyzw);
	// r3.w = 1;
	// float4 PreviousWorldPositionVR;
	// PreviousWorldPositionVR.x = dot(UnknownPerGeometry[6].xyzw, r3.xyzw);
	// PreviousWorldPositionVR.y = dot(UnknownPerGeometry[7].xyzw, r3.xyzw);
	// PreviousWorldPositionVR.z = dot(UnknownPerGeometry[8].xyzw, r3.xyzw);
	// PreviousWorldPositionVR.w = 1;
	// end
#	endif

#	if defined(SKINNED) || !defined(MODELSPACENORMALS)
	float3x3 tbn = float3x3(input.TBN0.xyz, input.TBN1.xyz, input.TBN2.xyz);
#	endif  // defined (SKINNED) || !defined (MODELSPACENORMALS)

#	if defined(LANDSCAPE)
	float shininess = dot(input.LandBlendWeights1, LandscapeTexture1to4IsSpecPower) + input.LandBlendWeights2.x * LandscapeTexture5to6IsSpecPower.x + input.LandBlendWeights2.y * LandscapeTexture5to6IsSpecPower.y;
#	else
	float shininess = SpecularColor.w;
#	endif  // defined (LANDSCAPE)

	float3 viewPosition = mul(CameraView[eyeIndex], float4(input.WorldPosition.xyz, 1)).xyz;
#	if defined(CPM_AVAILABLE)
	float parallaxShadowQuality = 1 - smoothstep(perPassParallax[0].ShadowsStartFade, perPassParallax[0].ShadowsEndFade, viewPosition.z);
#	endif

#	if defined(HAS_VIEW_VECTOR)
	// #if defined (SKINNED)
	//	float3 viewDirection = normalize(mul(tbn, input.ViewVector.xyz));
	// #else
	float3 viewDirection = normalize(input.ViewVector.xyz);
// #endif
#	else
	float3 viewDirection = 0.57735026.xxx;
#	endif  // HAS_VIEW_VECTOR

	float2 uv = input.TexCoord0.xy;
	float2 uvOriginal = uv;

#	if defined(LANDSCAPE)
	float mipLevel[6];
	float sh0[6];
#	else
	float mipLevel;
	float sh0;
#	endif  // LANDSCAPE

#	if defined(CPM_AVAILABLE)
#		if defined(PARALLAX)
	if (perPassParallax[0].EnableParallax) {
		mipLevel = GetMipLevel(uv, TexParallaxSampler);
		uv = GetParallaxCoords(viewPosition.z, uv, mipLevel, viewDirection, tbn, TexParallaxSampler, SampParallaxSampler, 0);
		if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
			sh0 = TexParallaxSampler.SampleLevel(SampParallaxSampler, uv, 0).x;
	}
#		endif  // PARALLAX

#		if defined(ENVMAP)
	bool complexMaterial = false;
	bool complexMaterialParallax = false;
	float4 complexMaterialColor = 1;

	if (perPassParallax[0].EnableComplexMaterial) {
		float envMaskTest = TexEnvMaskSampler.SampleLevel(SampEnvMaskSampler, uv, 15).w;
		complexMaterial = envMaskTest < (1.0f - (4.0f / 255.0f));

		if (complexMaterial) {
			if (envMaskTest > (4.0f / 255.0f)) {
				complexMaterialParallax = true;
				mipLevel = GetMipLevel(uv, TexEnvMaskSampler);
				uv = GetParallaxCoords(viewPosition.z, uv, mipLevel, viewDirection, tbn, TexEnvMaskSampler, SampTerrainParallaxSampler, 3);
				if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
					sh0 = TexEnvMaskSampler.SampleLevel(SampEnvMaskSampler, uv, 0).w;
			}

			complexMaterialColor = TexEnvMaskSampler.Sample(SampEnvMaskSampler, uv);
		}
	}
#		endif  // ENVMAP
#	endif      // CPM_AVAILABLE

#	if defined(SNOW)
	bool useSnowSpecular = true;
#	else
	bool useSnowSpecular = false;
#	endif  // SNOW

#	if defined(SPARKLE) || !defined(PROJECTED_UV)
	bool useSnowDecalSpecular = true;
#	else
	bool useSnowDecalSpecular = false;
#	endif  // defined(SPARKLE) || !defined(PROJECTED_UV)

	float2 diffuseUv = uv;
#	if defined(SPARKLE)
	diffuseUv = ProjectedUVParams2.yy * input.TexCoord0.zw;
#	endif  // SPARKLE

#	if defined(CPM_AVAILABLE)

#		if defined(LANDSCAPE)
	float2 terrainUVs[6];
	if (perPassParallax[0].EnableTerrainParallax && input.LandBlendWeights1.x > 0.0) {
		mipLevel[0] = GetMipLevel(uv, TexColorSampler);
		uv = GetParallaxCoords(viewPosition.z, uv, mipLevel[0], viewDirection, tbn, TexColorSampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights1.x);
		terrainUVs[0] = uv;
		if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
			sh0[0] = TexColorSampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
	}
#		endif  // LANDSCAPE

#		if defined(SPARKLE)
	diffuseUv = ProjectedUVParams2.yy * (input.TexCoord0.zw + (uv - uvOriginal));
#		else
	diffuseUv = uv;
#		endif  // SPARKLE
#	endif      // CPM_AVAILABLE

	float4 baseColor = 0;
	float4 normal = 0;
	float4 glossiness = 0;

#	if defined(LANDSCAPE)
	if (input.LandBlendWeights1.x > 0.0) {
#	endif  // LANDSCAPE

		float4 rawBaseColor = TexColorSampler.Sample(SampColorSampler, diffuseUv);

		baseColor = rawBaseColor;

		float landSnowMask1 = GetLandSnowMaskValue(baseColor.w);
		float4 normalColor = TexNormalSampler.Sample(SampNormalSampler, uv);

		normal = normalColor;
#	if defined(MODELSPACENORMALS)
#		if defined(LODLANDNOISE)
		normal.xyz = normal.xzy - 0.5.xxx;
		float lodLandNoiseParameter = GetLodLandBlendParameter(baseColor.xyz);
		float noise = TexLandLodNoiseSampler.Sample(SampLandLodNoiseSampler, uv * 3.0.xx).x;
		float lodLandNoiseMultiplier = GetLodLandBlendMultiplier(lodLandNoiseParameter, noise);
		baseColor.xyz *= lodLandNoiseMultiplier;
		normal.xyz *= 2;
		normal.w = 1;
		glossiness = 0;
#		elif defined(LODLANDSCAPE)
		normal.xyz = 2.0.xxx * (-0.5.xxx + normal.xzy);
		normal.w = 1;
		glossiness = 0;
#		else
		normal.xyz = normal.xzy * 2.0.xxx + -1.0.xxx;
		normal.w = 1;
		glossiness = TexSpecularSampler.Sample(SampSpecularSampler, uv).x;
#		endif  // LODLANDNOISE
#	elif (defined(SNOW) && defined(LANDSCAPE))
	normal.xyz = GetLandNormal(landSnowMask1, normal.xyz, uv, SampNormalSampler, TexNormalSampler);
	glossiness = normal.w;
#	else
	normal.xyz = TransformNormal(normal.xyz);
	glossiness = normal.w;
#	endif  // MODELSPACENORMALS

#	if defined(WORLD_MAP)
		normal.xyz = GetWorldMapNormal(input, normal.xyz, rawBaseColor.xyz);
#	endif  // WORLD_MAP

#	if defined(LANDSCAPE)
		baseColor *= input.LandBlendWeights1.x;
		normal *= input.LandBlendWeights1.x;
		glossiness *= input.LandBlendWeights1.x;
	}

#	endif  // LANDSCAPE

#	if defined(CPM_AVAILABLE) && defined(ENVMAP)
	float3 complexSpecular = 1.0f;

	if (complexMaterial) {
		complexMaterial = complexMaterialColor.y > (4.0f / 255.0f) && complexMaterialColor.y < (1 - (4.0f / 255.0f));

		if (complexMaterial) {
			shininess = shininess * complexMaterialColor.y;
			complexSpecular = lerp(1.0f, baseColor.xyz, complexMaterialColor.z);
		}
	}
#	endif  // defined (CPM_AVAILABLE) && defined(ENVMAP)

#	if defined(PBR)
	// float3 rmaoColor = TexRMAOSampler.Sample(SampRMAOSampler, uv).xyz;
	// float roughness = rmaoColor.x;
	// float metallic = rmaoColor.y;
	// float ao = rmaoColor.z;

	float roughness = 1 - glossiness;
	float metallic = 0;
#		if defined(ENVMAP)
	float envMaskColor = TexEnvMaskSampler.Sample(SampEnvMaskSampler, uv).x;
	metallic = envMaskColor * EnvmapData.x;
#		endif  // ENVMAP
	float ao = 1;

	float3 F0 = 0.04.xxx;
	F0 = lerp(F0, baseColor.xyz, metallic);

	float3 totalRadiance = 0.0.xxx;
#	endif  // PBR

#	if defined(FACEGEN)
	baseColor.xyz = GetFacegenBaseColor(baseColor.xyz, uv);
#	elif defined(FACEGEN_RGB_TINT)
	baseColor.xyz = GetFacegenRGBTintBaseColor(baseColor.xyz, uv);
#	endif  // FACEGEN

#	if defined(LANDSCAPE)

#		if defined(SNOW)
	float landSnowMask = LandscapeTexture1to4IsSnow.x * input.LandBlendWeights1.x;
#		endif  // SNOW

	if (input.LandBlendWeights1.y > 0.0) {
#		if defined(CPM_AVAILABLE)
		if (perPassParallax[0].EnableTerrainParallax) {
			mipLevel[1] = GetMipLevel(uvOriginal, TexLandColor2Sampler);
			uv = GetParallaxCoords(viewPosition.z, uvOriginal, mipLevel[1], viewDirection, tbn, TexLandColor2Sampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights1.y);
			terrainUVs[1] = uv;
			if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
				sh0[1] = TexLandColor2Sampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
		}
#		endif  // CPM_AVAILABLE
		float4 landColor2 = TexLandColor2Sampler.Sample(SampLandColor2Sampler, uv);
		float landSnowMask2 = GetLandSnowMaskValue(landColor2.w);
		baseColor += input.LandBlendWeights1.yyyy * landColor2;
		float4 landNormal2 = TexLandNormal2Sampler.Sample(SampLandNormal2Sampler, uv);
		landNormal2.xyz = GetLandNormal(landSnowMask2, landNormal2.xyz, uv, SampLandNormal2Sampler, TexLandNormal2Sampler);
		normal.xyz += input.LandBlendWeights1.yyy * landNormal2.xyz;
		glossiness += input.LandBlendWeights1.y * landNormal2.w;
#		if defined(SNOW)
		landSnowMask += LandscapeTexture1to4IsSnow.y * input.LandBlendWeights1.y * landSnowMask2;
#		endif  // SNOW
	}

	if (input.LandBlendWeights1.z > 0.0) {
#		if defined(CPM_AVAILABLE)
		if (perPassParallax[0].EnableTerrainParallax) {
			mipLevel[2] = GetMipLevel(uvOriginal, TexLandColor3Sampler);
			uv = GetParallaxCoords(viewPosition.z, uvOriginal, mipLevel[2], viewDirection, tbn, TexLandColor3Sampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights1.z);
			terrainUVs[2] = uv;
			if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
				sh0[2] = TexLandColor3Sampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
		}
#		endif  // CPM_AVAILABLE
		float4 landColor3 = TexLandColor3Sampler.Sample(SampLandColor3Sampler, uv);
		float landSnowMask3 = GetLandSnowMaskValue(landColor3.w);
		baseColor += input.LandBlendWeights1.zzzz * landColor3;
		float4 landNormal3 = TexLandNormal3Sampler.Sample(SampLandNormal3Sampler, uv);
		landNormal3.xyz = GetLandNormal(landSnowMask3, landNormal3.xyz, uv, SampLandNormal3Sampler, TexLandNormal3Sampler);
		normal.xyz += input.LandBlendWeights1.zzz * landNormal3.xyz;
		glossiness += input.LandBlendWeights1.z * landNormal3.w;
#		if defined(SNOW)
		landSnowMask += LandscapeTexture1to4IsSnow.z * input.LandBlendWeights1.z * landSnowMask3;
#		endif  // SNOW
	}

	if (input.LandBlendWeights1.w > 0.0) {
#		if defined(CPM_AVAILABLE)
		if (perPassParallax[0].EnableTerrainParallax) {
			mipLevel[3] = GetMipLevel(uvOriginal, TexLandColor4Sampler);
			uv = GetParallaxCoords(viewPosition.z, uvOriginal, mipLevel[3], viewDirection, tbn, TexLandColor4Sampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights1.w);
			terrainUVs[3] = uv;
			if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
				sh0[3] = TexLandColor4Sampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
		}
#		endif  // CPM_AVAILABLE
		float4 landColor4 = TexLandColor4Sampler.Sample(SampLandColor4Sampler, uv);
		float landSnowMask4 = GetLandSnowMaskValue(landColor4.w);
		baseColor += input.LandBlendWeights1.wwww * landColor4;
		float4 landNormal4 = TexLandNormal4Sampler.Sample(SampLandNormal4Sampler, uv);
		landNormal4.xyz = GetLandNormal(landSnowMask4, landNormal4.xyz, uv, SampLandNormal4Sampler, TexLandNormal4Sampler);
		normal.xyz += input.LandBlendWeights1.www * landNormal4.xyz;
		glossiness += input.LandBlendWeights1.w * landNormal4.w;
#		if defined(SNOW)
		landSnowMask += LandscapeTexture1to4IsSnow.w * input.LandBlendWeights1.w * landSnowMask4;
#		endif  // SNOW
	}

	if (input.LandBlendWeights2.x > 0.0) {
#		if defined(CPM_AVAILABLE)
		if (perPassParallax[0].EnableTerrainParallax) {
			mipLevel[4] = GetMipLevel(uvOriginal, TexLandColor5Sampler);
			uv = GetParallaxCoords(viewPosition.z, uvOriginal, mipLevel[4], viewDirection, tbn, TexLandColor5Sampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights2.x);
			terrainUVs[4] = uv;
			if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
				sh0[4] = TexLandColor5Sampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
		}
#		endif  // CPM_AVAILABLE
		float4 landColor5 = TexLandColor5Sampler.Sample(SampLandColor5Sampler, uv);
		float landSnowMask5 = GetLandSnowMaskValue(landColor5.w);
		baseColor += input.LandBlendWeights2.xxxx * landColor5;
		float4 landNormal5 = TexLandNormal5Sampler.Sample(SampLandNormal5Sampler, uv);
		landNormal5.xyz = GetLandNormal(landSnowMask5, landNormal5.xyz, uv, SampLandNormal5Sampler, TexLandNormal5Sampler);
		normal.xyz += input.LandBlendWeights2.xxx * landNormal5.xyz;
		glossiness += input.LandBlendWeights2.x * landNormal5.w;
#		if defined(SNOW)
		landSnowMask += LandscapeTexture5to6IsSnow.x * input.LandBlendWeights2.x * landSnowMask5;
#		endif  // SNOW
	}

	if (input.LandBlendWeights2.y > 0.0) {
#		if defined(CPM_AVAILABLE)
		if (perPassParallax[0].EnableTerrainParallax) {
			mipLevel[5] = GetMipLevel(uvOriginal, TexLandColor6Sampler);
			uv = GetParallaxCoords(viewPosition.z, uvOriginal, mipLevel[5], viewDirection, tbn, TexLandColor6Sampler, SampTerrainParallaxSampler, 3, input.LandBlendWeights2.y);
			terrainUVs[5] = uv;
			if (perPassParallax[0].EnableShadows && parallaxShadowQuality > 0.0f)
				sh0[5] = TexLandColor6Sampler.SampleLevel(SampTerrainParallaxSampler, uv, 0).w;
		}
#		endif  // CPM_AVAILABLE
		float4 landColor6 = TexLandColor6Sampler.Sample(SampLandColor6Sampler, uv);
		float landSnowMask6 = GetLandSnowMaskValue(landColor6.w);
		baseColor += input.LandBlendWeights2.yyyy * landColor6;
		float4 landNormal6 = TexLandNormal6Sampler.Sample(SampLandNormal6Sampler, uv);
		landNormal6.xyz = GetLandNormal(landSnowMask6, landNormal6.xyz, uv, SampLandNormal6Sampler, TexLandNormal6Sampler);
		normal.xyz += input.LandBlendWeights2.yyy * landNormal6.xyz;
		glossiness += input.LandBlendWeights2.y * landNormal6.w;
#		if defined(SNOW)
		landSnowMask += LandscapeTexture5to6IsSnow.y * input.LandBlendWeights2.y * landSnowMask6;
#		endif  // SNOW
	}

#		if defined(LOD_LAND_BLEND)
	float4 lodBlendColor = TexLandLodBlend1Sampler.Sample(SampLandLodBlend1Sampler, input.TexCoord0.zw);
	float lodBlendTmp = GetLodLandBlendParameter(lodBlendColor.xyz);
	float lodBlendMask = TexLandLodBlend2Sampler.Sample(SampLandLodBlend2Sampler, 3.0.xx * input.TexCoord0.zw).x;
	float lodBlendMul1 = GetLodLandBlendMultiplier(lodBlendTmp, lodBlendMask);
	float lodBlendMul2 = LODTexParams.z * input.LandBlendWeights2.w;
	baseColor.w = 0;
	baseColor = lodBlendMul2.xxxx * (lodBlendColor * lodBlendMul1.xxxx - baseColor) + baseColor;
	normal.xyz = lodBlendMul2.xxx * (float3(0, 0, 1) - normal.xyz) + normal.xyz;
	glossiness += lodBlendMul2 * -glossiness;
#		endif  // LOD_LAND_BLEND

#		if defined(SNOW)
	useSnowSpecular = landSnowMask > 0;
#		endif  // SNOW
#	endif      // LANDSCAPE

#	if defined(BACK_LIGHTING)
	float4 backLightColor = TexBackLightSampler.Sample(SampBackLightSampler, uv);
#	endif  // BACK_LIGHTING
#	if defined(SOFT_LIGHTING) || defined(RIM_LIGHTING)
	float4 rimSoftLightColor = TexRimSoftLightSampler.Sample(SampRimSoftLightSampler, uv);
#	endif  // defined (SOFT_LIGHTING) || defined(RIM_LIGHTING)

	float numLights = min(7, NumLightNumShadowLight.x);
#	if defined(DEFSHADOW)
	float numShadowLights = min(4, NumLightNumShadowLight.y);
#	endif  // DEFSHADOW

#	if defined(MODELSPACENORMALS) && !defined(SKINNED)
	float4 modelNormal = normal;
#	else
	float4 modelNormal = float4(normalize(mul(tbn, normal.xyz)), 1);

#		if defined(SPARKLE)
	float3 projectedNormal = normalize(mul(tbn, float3(ProjectedUVParams2.xx * normal.xy, normal.z)));
#		endif  // SPARKLE
#	endif      // defined (MODELSPACENORMALS) && !defined (SKINNED)

	float2 baseShadowUV = 1.0.xx;
#	if defined(DEFSHADOW)
	float4 shadowColor;
#		if !defined(SHADOW_DIR)
	if (numShadowLights > 0)
#		endif  // !defined (SHADOW_DIR)
	{
		baseShadowUV = input.Position.xy * DynamicResolutionParams2.xy;
		float2 shadowUV = min(float2(DynamicResolutionParams2.z, DynamicResolutionParams1.y), max(0.0.xx, DynamicResolutionParams1.xy * (baseShadowUV * VPOSOffset.xy + VPOSOffset.zw)));
		shadowColor = TexShadowMaskSampler.Sample(SampShadowMaskSampler, shadowUV);
	}
#		if !defined(SHADOW_DIR)
	else {
		shadowColor = 1.0.xxxx;
	}
#		endif  // SHADOW_DIR
#	endif      // DEFSHADOW

	float texProjTmp = 0;

#	if defined(PROJECTED_UV)
	float2 projNoiseUv = ProjectedUVParams.zz * input.TexCoord0.zw;
	float projNoise = TexProjNoiseSampler.Sample(SampProjNoiseSampler, projNoiseUv).x;
	float3 texProj = normalize(input.TexProj);
#		if defined(TREE_ANIM) || defined(LODOBJECTSHD)
	float vertexAlpha = 1;
#		else
	float vertexAlpha = input.Color.w;
#		endif  // defined (TREE_ANIM) || defined (LODOBJECTSHD)
	texProjTmp = -ProjectedUVParams.x * projNoise + (dot(modelNormal.xyz, texProj) * vertexAlpha - ProjectedUVParams.w);
#		if defined(LODOBJECTSHD)
	texProjTmp += (-0.5 + input.Color.w) * 2.5;
#		endif  // LODOBJECTSHD
#		if defined(SPARKLE)
	if (texProjTmp < 0) {
		discard;
	}

	modelNormal.xyz = projectedNormal;
#			if defined(SNOW)
	psout.SnowParameters.y = 1;
#			endif  // SNOW
#		else
	if (ProjectedUVParams3.w > 0.5) {
		float2 projNormalUv = ProjectedUVParams3.x * projNoiseUv;
		float3 projNormal = TransformNormal(TexProjNormalSampler.Sample(SampProjNormalSampler, projNormalUv).xyz);
		float2 projDetailUv = ProjectedUVParams3.y * projNoiseUv;
		float3 projDetail = TexProjDetail.Sample(SampProjDetailSampler, projDetailUv).xyz;
		float3 texProjTmp3 = (projDetail * 2.0.xxx + float3(projNormal.xy, -1));
		texProjTmp3.xy += -1.0.xx;
		texProjTmp3.z = projNormal.z * texProjTmp3.z;
		float3 finalProjNormal = normalize(texProjTmp3);
		float3 projDiffuse = TexProjDiffuseSampler.Sample(SampProjDiffuseSampler, projNormalUv).xyz;
		float texProjTmp1 = saturate(5 * (0.1 + texProjTmp));
		float texProjTmp2 = (texProjTmp1 * -2 + 3) * (texProjTmp1 * texProjTmp1);
		normal.xyz = texProjTmp2.xxx * (finalProjNormal - normal.xyz) + normal.xyz;
		baseColor.xyz = texProjTmp2.xxx * (projDiffuse * ProjectedUVParams2.xyz - baseColor.xyz) + baseColor.xyz;

		useSnowDecalSpecular = true;
#			if defined(SNOW)
		psout.SnowParameters.y = GetSnowParameterY(texProjTmp2, baseColor.w);
#			endif  // SNOW
	} else {
		if (texProjTmp > 0) {
			baseColor.xyz = ProjectedUVParams2.xyz;
			useSnowDecalSpecular = true;
#			if defined(SNOW)
			psout.SnowParameters.y = GetSnowParameterY(texProjTmp, baseColor.w);
#			endif  // SNOW
		} else {
#			if defined(SNOW)
			psout.SnowParameters.y = 0;
#			endif  // SNOW
		}
	}

#			if defined(SPECULAR)
	useSnowSpecular = useSnowDecalSpecular;
#			endif  // SPECULAR
#		endif      // SPARKLE

#	elif defined(SNOW)
#		if defined(LANDSCAPE)
	psout.SnowParameters.y = landSnowMask;
#		else
	psout.SnowParameters.y = baseColor.w;
#		endif  // LANDSCAPE
#	endif

#	if defined(WORLD_MAP)
	baseColor.xyz = GetWorldMapBaseColor(rawBaseColor.xyz, baseColor.xyz, texProjTmp);
#	endif  // WORLD_MAP

	float3 dirLightColor = DirLightColor.xyz;
	float selfShadowFactor = 1.0f;

	float3 nsDirLightColor = dirLightColor;

#	if defined(DEFSHADOW) && defined(SHADOW_DIR)
	dirLightColor *= shadowColor.xxx;
#	endif  // defined (DEFSHADOW) && defined (SHADOW_DIR)

#	if defined(SCREEN_SPACE_SHADOWS)
	float dirLightSShadow = PrepassScreenSpaceShadows(input.WorldPosition.xyz);
	dirLightColor *= dirLightSShadow;
#	endif  // SCREEN_SPACE_SHADOWS

#	if defined(CPM_AVAILABLE) && (defined(SKINNED) || !defined(MODELSPACENORMALS))
	float3 dirLightDirectionTS = mul(DirLightDirection, tbn).xyz;
	bool dirLightIsLit = true;

#		if defined(DEFSHADOW) && defined(SHADOW_DIR)
	if (shadowColor.x == 0)
		dirLightIsLit = false;
#		endif  // defined (DEFSHADOW) && defined (SHADOW_DIR)

#		if defined(SCREEN_SPACE_SHADOWS)
	if (dirLightSShadow == 0)
		dirLightIsLit = false;
#		endif  // SCREEN_SPACE_SHADOWS

#		if defined(LANDSCAPE)
	if (perPassParallax[0].EnableTerrainParallax && perPassParallax[0].EnableShadows)
		dirLightColor *= GetParallaxSoftShadowMultiplierTerrain(input, terrainUVs, mipLevel, dirLightDirectionTS, sh0, dirLightIsLit * parallaxShadowQuality);
#		elif defined(PARALLAX)
	if (perPassParallax[0].EnableParallax && perPassParallax[0].EnableShadows)
		dirLightColor *= GetParallaxSoftShadowMultiplier(uv, mipLevel, dirLightDirectionTS, sh0, TexParallaxSampler, SampParallaxSampler, 0, dirLightIsLit * parallaxShadowQuality);
#		elif defined(ENVMAP)
	if (complexMaterialParallax && perPassParallax[0].EnableShadows)
		dirLightColor *= GetParallaxSoftShadowMultiplier(uv, mipLevel, dirLightDirectionTS, sh0, TexEnvMaskSampler, SampEnvMaskSampler, 3, dirLightIsLit * parallaxShadowQuality);
#		endif  // LANDSCAPE
#	endif      // defined(CPM_AVAILABLE) && (defined (SKINNED) || !defined \
				// (MODELSPACENORMALS))

#	if defined(PBR)
	totalRadiance += GetLightRadiance(modelNormal.xyz, DirLightDirection.xyz, viewDirection, F0, dirLightColor.xyz, baseColor.xyz, roughness, metallic);
#	else
	float3 diffuseColor = 0.0.xxx;
	float3 specularColor = 0.0.xxx;

	float3 lightsDiffuseColor = 0.0.xxx;
	float3 lightsSpecularColor = 0.0.xxx;

	float dirLightAngle = dot(modelNormal.xyz, DirLightDirection.xyz);
	float3 dirDiffuseColor = dirLightColor * saturate(dirLightAngle.xxx);

#		if defined(SOFT_LIGHTING)
	lightsDiffuseColor += nsDirLightColor.xyz * GetSoftLightMultiplier(dirLightAngle) * rimSoftLightColor.xyz;
#		endif

#		if defined(RIM_LIGHTING)
	lightsDiffuseColor += nsDirLightColor.xyz * GetRimLightMultiplier(DirLightDirection, viewDirection, modelNormal.xyz) * rimSoftLightColor.xyz;
#		endif

#		if defined(BACK_LIGHTING)
	lightsDiffuseColor += nsDirLightColor.xyz * (saturate(-dirLightAngle) * backLightColor.xyz);
#		endif

	if (useSnowSpecular && useSnowDecalSpecular) {
#		if defined(SNOW)
		lightsSpecularColor = GetSnowSpecularColor(input, modelNormal.xyz, viewDirection);
#		endif
	} else {
#		if defined(SPECULAR) || defined(SPARKLE)
		lightsSpecularColor = GetLightSpecularInput(input, DirLightDirection, viewDirection, modelNormal.xyz, dirLightColor.xyz, shininess, uv);
#		endif
	}

	lightsDiffuseColor += dirDiffuseColor;
#	endif

	if (numLights > 0) {
		for (float lightIndex = 0; lightIndex < numLights; ++lightIndex) {
#	if defined(DEFSHADOW)
			float shadowComponent;
			if (lightIndex < numShadowLights) {
				shadowComponent = shadowColor[ShadowLightMaskSelect[lightIndex]];
			} else {
				shadowComponent = 1;
			}
#	endif
			int intLightIndex = lightIndex;
			float3 lightDirection = PointLightPosition[eyeIndex * numLights + intLightIndex].xyz - input.InputPosition.xyz;
			float lightDist = length(lightDirection);
			float intensityFactor = saturate(lightDist / PointLightPosition[intLightIndex].w);
			float intensityMultiplier = 1 - intensityFactor * intensityFactor;

			if (!(intensityMultiplier > 0.0))
				continue;

			float3 lightColor = PointLightColor[intLightIndex].xyz;
			float3 nsLightColor = lightColor;
#	if defined(DEFSHADOW)
			lightColor *= shadowComponent.xxx;
#	endif
			float3 normalizedLightDirection = normalize(lightDirection);

#	if defined(CPM_AVAILABLE)
			if (perPassParallax[0].EnableShadows) {
				float3 lightDirectionTS = mul(normalizedLightDirection, tbn).xyz;

				bool lightIsLit = true;

#		if defined(DEFSHADOW) && defined(SHADOW_DIR)
				if (shadowComponent.x == 0)
					lightIsLit = false;
#		endif

#		if defined(PARALLAX)
				if (perPassParallax[0].EnableParallax)
					lightColor *= GetParallaxSoftShadowMultiplier(uv, mipLevel, lightDirectionTS, sh0, TexParallaxSampler, SampParallaxSampler, 0, lightIsLit * parallaxShadowQuality);
#		elif defined(LANDSCAPE)
				if (perPassParallax[0].EnableTerrainParallax)
					lightColor *= GetParallaxSoftShadowMultiplierTerrain(input, terrainUVs, mipLevel, lightDirectionTS, sh0, lightIsLit * parallaxShadowQuality);
#		elif defined(ENVMAP)
				if (complexMaterialParallax)
					lightColor *= GetParallaxSoftShadowMultiplier(uv, mipLevel, lightDirectionTS, sh0, TexEnvMaskSampler, SampEnvMaskSampler, 3, lightIsLit * parallaxShadowQuality);
#		endif
			}
#	endif

#	if defined(PBR)
			totalRadiance += GetLightRadiance(modelNormal.xyz, normalizedLightDirection, viewDirection, F0, lightColor * intensityMultiplier.xxx, baseColor.xyz, roughness, metallic);
#	else
			float lightAngle = dot(modelNormal.xyz, normalizedLightDirection.xyz);
			float3 lightDiffuseColor = lightColor * saturate(lightAngle.xxx);

#		if defined(SOFT_LIGHTING)
			lightDiffuseColor += nsLightColor * GetSoftLightMultiplier(dot(modelNormal.xyz, lightDirection.xyz)) * rimSoftLightColor.xyz;
#		endif  // SOFT_LIGHTING

#		if defined(RIM_LIGHTING)
			lightDiffuseColor += nsLightColor * GetRimLightMultiplier(normalizedLightDirection, viewDirection, modelNormal.xyz) * rimSoftLightColor.xyz;
#		endif  // RIM_LIGHTING

#		if defined(BACK_LIGHTING)
			lightDiffuseColor += (saturate(-lightAngle) * backLightColor.xyz) * nsLightColor;
#		endif  // BACK_LIGHTING

#		if defined(SPECULAR) || (defined(SPARKLE) && !defined(SNOW))
			lightsSpecularColor += GetLightSpecularInput(input, normalizedLightDirection, viewDirection, modelNormal.xyz, lightColor, shininess, uv) * intensityMultiplier.xxx;
#		endif  // defined (SPECULAR) || (defined (SPARKLE) && !defined(SNOW))

			lightsDiffuseColor += lightDiffuseColor * intensityMultiplier.xxx;
#	endif      // PBR
		}
	}

#	if defined(PBR)
	// float3 ambientColor = 0.03 * baseColor.xyz * ao;
	float3 ambientColor = (mul(DirectionalAmbient, modelNormal) + IBLParams.yzw * IBLParams.xxx) * baseColor.xyz * ao;
	float3 color = ambientColor + totalRadiance;

	// color = color / (color + 1.0);
	// color = pow(color, 1.0 / 2.2);
#	else

	diffuseColor += lightsDiffuseColor;
	specularColor += lightsSpecularColor;

#		if defined(CHARACTER_LIGHT)
	float charLightMul =
		saturate(dot(viewDirection, modelNormal.xyz)) * CharacterLightParams.x +
		CharacterLightParams.y * saturate(dot(float2(0.164398998, -0.986393988), modelNormal.yz));
	float charLightColor = min(CharacterLightParams.w, max(0, CharacterLightParams.z * TexCharacterLightSampler.Sample(SampCharacterLightSampler, baseShadowUV).x));
	diffuseColor += (charLightMul * charLightColor).xxx;
#		endif  // CHARACTER_LIGHT

#		if defined(EYE)
	modelNormal.xyz = input.EyeNormal;
#		endif  // EYE

#		if defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(EYE)
	float envMaskColor = TexEnvMaskSampler.Sample(SampEnvMaskSampler, uv).x;
	float envMask = (EnvmapData.y * (envMaskColor - glossiness) + glossiness) * (EnvmapData.x * MaterialData.x);
	float viewNormalAngle = dot(modelNormal.xyz, viewDirection);
	float3 envSamplingPoint = (viewNormalAngle * 2) * modelNormal.xyz - viewDirection;
	float3 envColor = TexEnvSampler.Sample(SampEnvSampler, envSamplingPoint).xyz * envMask.xxx;
#		endif  // defined (ENVMAP) || defined (MULTI_LAYER_PARALLAX) || defined(EYE)

	float3 emitColor = EmitColor;
#		if defined(GLOWMAP)
	float3 glowColor = TexGlowSampler.Sample(SampGlowSampler, uv).xyz;
	emitColor *= glowColor;
#		endif  // GLOWMAP

	float3 directionalAmbientColor = mul(DirectionalAmbient, modelNormal);
	diffuseColor = directionalAmbientColor + emitColor.xyz + diffuseColor;
	diffuseColor += IBLParams.yzw * IBLParams.xxx;

	float4 color;
	color.xyz = diffuseColor * baseColor.xyz;

#	endif  // PBR

#	if defined(HAIR)
	float3 vertexColor = (input.Color.yyy * (TintColor.xyz - 1.0.xxx) + 1.0.xxx) * color.xyz;
#	else
	float3 vertexColor = input.Color.xyz * color.xyz;
#	endif  // HAIR

#	if defined(MULTI_LAYER_PARALLAX)
	float layerValue = MultiLayerParallaxData.x * TexLayerSampler.Sample(SampLayerSampler, uv).w;
	float3 tangentViewDirection = mul(viewDirection, tbn);
	float3 layerNormal = MultiLayerParallaxData.yyy * (normalColor.xyz * 2.0.xxx + float3(-1, -1, -2)) + float3(0, 0, 1);
	float layerViewAngle = dot(-tangentViewDirection.xyz, layerNormal.xyz) * 2;
	float3 layerViewProjection = -layerNormal.xyz * layerViewAngle.xxx - tangentViewDirection.xyz;
	float2 layerUv = uv * MultiLayerParallaxData.zw + (0.0009765625 * (layerValue / abs(layerViewProjection.z))).xx * layerViewProjection.xy;

	float3 layerColor = TexLayerSampler.Sample(SampLayerSampler, layerUv).xyz;

	vertexColor = (saturate(viewNormalAngle) * (1 - baseColor.w)).xxx * ((directionalAmbientColor + lightsDiffuseColor) * (input.Color.xyz * layerColor) - vertexColor) + vertexColor;

#	endif  // MULTI_LAYER_PARALLAX

	//#	if !defined(VR)
	float2 screenMotionVector = GetSSMotionVector(input.WorldPosition, input.PreviousWorldPosition, eyeIndex);
	// #	else   // VR
	// float2 screenMotionVector = GetSSMotionVector(worldPositionVR, PreviousWorldPositionVR, eyeIndex);
// #	endif  // !VR
#	if !defined(PBR)
#		if defined(SPECULAR)
	specularColor = (specularColor * glossiness * MaterialData.yyy) * SpecularColor.xyz;
#		elif defined(SPARKLE)
	specularColor *= glossiness;
#		endif  // SPECULAR
#	endif      // !defined(PBR)
	if (useSnowSpecular) {
		specularColor = 0;
	}

#	if defined(AMBIENT_SPECULAR)
	float viewAngle = saturate(dot(modelNormal.xyz, viewDirection));
	float ambientSpecularColorMultiplier = exp2(AmbientSpecularTintAndFresnelPower.w * log2(1 - viewAngle));
	float3 ambientSpecularColor = AmbientSpecularTintAndFresnelPower.xyz * saturate(mul(DirectionalAmbient, float4(modelNormal.xyz, 0.15)));
	specularColor += ambientSpecularColor * ambientSpecularColorMultiplier.xxx;
#	endif  // AMBIENT_SPECULAR

#	if !defined(PBR) && (defined(ENVMAP) || defined(MULTI_LAYER_PARALLAX) || defined(EYE))
#		if defined(CPM_AVAILABLE) && defined(ENVMAP)
	vertexColor += diffuseColor * envColor * complexSpecular;
#		else
	vertexColor += diffuseColor * envColor;

#		endif  // defined (CPM_AVAILABLE) && defined(ENVMAP)
#	endif      // !defined(PBR) && (defined (ENVMAP) || defined (MULTI_LAYER_PARALLAX) \
				// || defined(EYE))

	color.xyz = lerp(vertexColor.xyz, input.FogParam.xyz, input.FogParam.w);
	color.xyz = vertexColor.xyz - color.xyz * FogColor.w;

	float3 tmpColor = color.xyz * FrameParams.yyy;
	color.xyz = tmpColor.xyz + ColourOutputClamp.xxx;
	color.xyz = min(vertexColor.xyz, color.xyz);

#	if !defined(PBR)
#		if defined(CPM_AVAILABLE) && defined(ENVMAP)
	color.xyz += specularColor * complexSpecular;
#		else
	color.xyz += specularColor;
#		endif  // defined (CPM_AVAILABLE) && defined(ENVMAP)

#		if defined(SPECULAR) || defined(AMBIENT_SPECULAR) || defined(SPARKLE)
	float3 specularTmp = lerp(color.xyz, input.FogParam.xyz, input.FogParam.w);
	specularTmp = color.xyz - specularTmp.xyz * FogColor.w;

	tmpColor = specularTmp.xyz * FrameParams.yyy;
	specularTmp.xyz = tmpColor.xyz + ColourOutputClamp.zzz;
	color.xyz = min(specularTmp.xyz, color.xyz);
#		endif  // defined (SPECULAR) || defined(AMBIENT_SPECULAR) || defined(SPARKLE)

#	endif  // !defined(PBR)

#	if defined(LANDSCAPE) && !defined(LOD_LAND_BLEND)
	psout.Albedo.w = 0;
#	else
	float alpha = baseColor.w;
#		if !defined(ADDITIONAL_ALPHA_MASK)
	alpha *= MaterialData.z;
#		else
	uint2 alphaMask = input.Position.xy;
	alphaMask.x = ((alphaMask.x << 2) & 12);
	alphaMask.x = (alphaMask.y & 3) | (alphaMask.x & ~3);
	const float maskValues[16] = {
		0.003922,
		0.533333,
		0.133333,
		0.666667,
		0.800000,
		0.266667,
		0.933333,
		0.400000,
		0.200000,
		0.733333,
		0.066667,
		0.600000,
		0.996078,
		0.466667,
		0.866667,
		0.333333,
	};

	float testTmp = 0;
	if (MaterialData.z - maskValues[alphaMask.x] < 0) {
		discard;
	}
#		endif  // !defined(ADDITIONAL_ALPHA_MASK)
#		if !(defined(TREE_ANIM) || defined(LODOBJECTSHD) || defined(LODOBJECTS))
	alpha *= input.Color.w;
#		endif  // !(defined(TREE_ANIM) || defined(LODOBJECTSHD) || defined(LODOBJECTS))
#		if defined(DO_ALPHA_TEST)
#			if defined(DEPTH_WRITE_DECALS)
	if (alpha - 0.0156862754 < 0) {
		discard;
	}
	alpha = saturate(1.05 * alpha);
#			endif  // DEPTH_WRITE_DECALS
	if (alpha - AlphaThreshold < 0) {
		discard;
	}
#		endif      // DO_ALPHA_TEST
	psout.Albedo.w = alpha;

#	endif

	psout.Albedo.xyz = color.xyz - tmpColor.xyz * FrameParams.zzz;

#	if defined(SNOW)
	psout.SnowParameters.x = dot(lightsSpecularColor, float3(0.3, 0.59, 0.11));
#	endif  // SNOW

	psout.MotionVectors.xy = SSRParams.z > 1e-5 ? float2(1, 0) : screenMotionVector.xy;
	psout.MotionVectors.zw = float2(0, 1);

	float tmp = -1e-5 + SSRParams.x;
	float tmp3 = (SSRParams.y - tmp);
	float tmp2 = (glossiness - tmp);
	float tmp1 = 1 / tmp3;
	tmp = saturate(tmp1 * tmp2);
	tmp *= tmp * (3 + -2 * tmp);
	psout.ScreenSpaceNormals.w = tmp * SSRParams.w;

#	if defined(WATER_BLENDING)
	if (perPassWaterBlending[0].EnableWaterBlendingSSR) {
		// Compute distance to water surface
		float distToWater = max(0, input.WorldPosition.z - perPassWaterBlending[0].WaterHeight);
		float blendFactor = smoothstep(viewPosition.z * 0.001 * 4, viewPosition.z * 0.001 * 16 * perPassWaterBlending[0].SSRBlendRange, distToWater);

		// Reduce SSR amount
		normal.z *= blendFactor;
		normal.xyz = normalize(normal.xyz);
		psout.ScreenSpaceNormals.w *= blendFactor;
	}
#	endif  // WATER_BLENDING

#	if !defined(LANDSCAPE) && defined(SPECULAR)
	// Green reflections fix
	psout.ScreenSpaceNormals.w = psout.ScreenSpaceNormals.w * pow(psout.Albedo.w, 10);
#	endif  //  !defined(LANDSCAPE) && defined(SPECULAR)

	float3 screenSpaceNormal;
	screenSpaceNormal.x = dot(input.ScreenNormalTransform0.xyz, normal.xyz);
	screenSpaceNormal.y = dot(input.ScreenNormalTransform1.xyz, normal.xyz);
	screenSpaceNormal.z = dot(input.ScreenNormalTransform2.xyz, normal.xyz);
	screenSpaceNormal = normalize(screenSpaceNormal);

	screenSpaceNormal.z = max(0.001, sqrt(8 + -8 * screenSpaceNormal.z));
	screenSpaceNormal.xy /= screenSpaceNormal.zz;
	psout.ScreenSpaceNormals.xy = screenSpaceNormal.xy + 0.5.xx;
	psout.ScreenSpaceNormals.z = 0;

#	if defined(OUTLINE)
	psout.Albedo = float4(1, 0, 0, 1);
#	endif  // OUTLINE

	return psout;
}
#endif  // PSHADER
