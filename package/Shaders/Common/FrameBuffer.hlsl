cbuffer PerFrame : register(b12)
{
#if !defined(VR)
	row_major float4x4 CameraView[1] : packoffset(c0);
	row_major float4x4 CameraProj[1] : packoffset(c4);
	row_major float4x4 CameraViewProj[1] : packoffset(c8);
	row_major float4x4 CameraViewProjUnjittered[1] : packoffset(c12);
	row_major float4x4 CameraPreviousViewProjUnjittered[1] : packoffset(c16);
	row_major float4x4 CameraProjUnjittered[1] : packoffset(c20);
	row_major float4x4 CameraProjUnjitteredInverse[1] : packoffset(c24);
	row_major float4x4 CameraViewInverse[1] : packoffset(c28);
	row_major float4x4 CameraViewProjInverse[1] : packoffset(c32);
	row_major float4x4 CameraProjInverse[1] : packoffset(c36);
	float4 CameraPosAdjust[1] : packoffset(c40);
	float4 CameraPreviousPosAdjust[1] : packoffset(c41);  // fDRClampOffset in w
	float4 FrameParams : packoffset(c42);                 // inverse fGamma in x, some flags in yzw
	float4 DynamicResolutionParams1 : packoffset(c43);    // fDynamicResolutionWidthRatio in x,
														  // fDynamicResolutionHeightRatio in y,
														  // fDynamicResolutionPreviousWidthRatio in z,
														  // fDynamicResolutionPreviousHeightRatio in w
	float4 DynamicResolutionParams2 : packoffset(c44);    // inverse fDynamicResolutionWidthRatio in x, inverse
														  // fDynamicResolutionHeightRatio in y,
														  // fDynamicResolutionWidthRatio - fDRClampOffset in z,
														  // fDynamicResolutionPreviousWidthRatio - fDRClampOffset in w
#else
	row_major float4x4 CameraView[2] : packoffset(c0);
	row_major float4x4 CameraProj[2] : packoffset(c8);
	row_major float4x4 CameraViewProj[2] : packoffset(c16);
	row_major float4x4 CameraViewProjUnjittered[2] : packoffset(c24);
	row_major float4x4 CameraPreviousViewProjUnjittered[2] : packoffset(c32);
	row_major float4x4 CameraProjUnjittered[2] : packoffset(c40);
	row_major float4x4 CameraProjUnjitteredInverse[2] : packoffset(c48);
	row_major float4x4 CameraViewInverse[2] : packoffset(c56);
	row_major float4x4 CameraViewProjInverse[2] : packoffset(c64);
	row_major float4x4 CameraProjInverse[2] : packoffset(c72);
	float4 CameraPosAdjust[2] : packoffset(c80);
	float4 CameraPreviousPosAdjust[2] : packoffset(c82);  // fDRClampOffset in w
	float4 FrameParams : packoffset(c84);                 // inverse fGamma in x, some flags in yzw
	float4 DynamicResolutionParams1 : packoffset(c85);    // fDynamicResolutionWidthRatio in x,
														  // fDynamicResolutionHeightRatio in y,
														  // fDynamicResolutionPreviousWidthRatio in z,
														  // fDynamicResolutionPreviousHeightRatio in w
	float4 DynamicResolutionParams2 : packoffset(c86);    // inverse fDynamicResolutionWidthRatio in x, inverse
														  // fDynamicResolutionHeightRatio in y,
														  // fDynamicResolutionWidthRatio - fDRClampOffset in z,
														  // fDynamicResolutionPreviousWidthRatio - fDRClampOffset in w
#endif  // !VR
}

float2 GetDynamicResolutionAdjustedScreenPosition(float2 screenPosition)
{
	float2 adjustedScreenPosition =
		max(0.0.xx, DynamicResolutionParams1.xy * screenPosition);
	return min(float2(DynamicResolutionParams2.z, DynamicResolutionParams1.y),
		adjustedScreenPosition);
}

float2
	GetPreviousDynamicResolutionAdjustedScreenPosition(float2 screenPosition)
{
	float2 adjustedScreenPosition =
		max(0.0.xx, DynamicResolutionParams1.zw * screenPosition);
	return min(float2(DynamicResolutionParams2.w, DynamicResolutionParams1.w),
		adjustedScreenPosition);
}

float3 ToSRGBColor(float3 linearColor)
{
	return pow(linearColor, FrameParams.x);
}

float3 WorldToView(float3 x, bool is_position = true, uint a_eyeIndex = 0)
{
	float4 newPosition = float4(x, (float)is_position);
	return mul(CameraView[a_eyeIndex], newPosition).xyz;
}

float2 ViewToUV(float3 x, bool is_position = true, uint a_eyeIndex = 0)
{
	float4 newPosition = float4(x, (float)is_position);
	float4 uv = mul(CameraProj[a_eyeIndex], newPosition);
	return (uv.xy / uv.w) * float2(0.5f, -0.5f) + 0.5f;
}
