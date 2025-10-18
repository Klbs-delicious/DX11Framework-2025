﻿/**@file   ShaderCommon.h
 * @brief  シェーダーの共通定義
 * @date   2025/10/09
 */
#pragma once
#include<d3d11.h>

#include<string>
#include<array>

 /**@namespace	ShaderCommon
  * @brief		シェーダー情報の共通定義
  */
namespace ShaderCommon
{
	/**	@enum	ShaderType
	 *	@brief シェーダーの種類
	 */
	enum class ShaderType
	{
		VertexShader,		///< 頂点シェーダー
		PixelShader,		///< ピクセルシェーダー
		GeometryShader,		///< ジオメトリシェーダー

		ComputeShader,		///< コンピュートシェーダー
		TessellationShader,	///< テッセレーションシェーダー

		MaxShaderType,
	};

	/**	@struct	LayoutType
	 *	@brief	入力レイアウトの種類
	 */
	enum class LayoutType
	{
		Basic,
		//PosOnly,
		//PosColor,
		//Skinned,
		Max,
	};

	/**	@struct	ShaderInfo
	 *	@brief シェーダー情報
	 *	@details シェーダーの種類やパスなどを管理する
	 */
	struct ShaderInfo
	{
		ShaderType shaderType;		///< シェーダーの種類
		std::wstring filePath;		///< シェーダーファイルのパス
		LayoutType layoutType;		///< シェーダーに対応した入力レイアウトタイプ？

		ShaderInfo(ShaderType _shaderType, std::wstring _filePath, LayoutType _layoutType = LayoutType::Basic)
		{
			this->shaderType = _shaderType;
			this->filePath = _filePath;
			this->layoutType = _layoutType;
		}
	};

	/// @brief シェーダーのエントリーポイント名
	inline static const std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> EntryPointName = {
		//"VS_Main",
		//"PS_Main",
		//"GS_Main",
		//"CS_Main",
		//"HS_Main"
		//"HS_Main"
		////"DS_Main"
		"main",
		"main",
		"main",
		"main",
		"main",
	};

	/// @brief シェーダーのターゲット名
	inline static const std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> TargetName = {
		"vs_5_0",
		"ps_5_0",
		"gs_5_0",
		"cs_5_0",
		"hs_5_0"
		//"ds_5_0"
	};

	/// @brief LayoutType ごとのレイアウト定義
	inline static const std::array<std::vector<D3D11_INPUT_ELEMENT_DESC>, static_cast<size_t>(LayoutType::Max)> LayoutDescs = {
		// LayoutType::Basic
		std::vector<D3D11_INPUT_ELEMENT_DESC>{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(float) * 7,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
		},
		//// LayoutType::PosOnly
		//std::vector<D3D11_INPUT_ELEMENT_DESC>{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//},
		//// LayoutType::PosColor
		//std::vector<D3D11_INPUT_ELEMENT_DESC>{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//},
		//// LayoutType::Skinned
		//std::vector<D3D11_INPUT_ELEMENT_DESC>{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "INDICES",  0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//}
	};
}
