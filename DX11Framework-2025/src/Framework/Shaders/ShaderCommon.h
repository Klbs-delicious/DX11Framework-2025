/**@file   ShaderCommon.h
 * @brief  シェーダーの共通定義
 * @date   2025/10/09
 */
#pragma once
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

	/**	@struct	ShaderInfo
	 *	@brief シェーダー情報
	 *	@details シェーダーの種類やパスなどを管理する
	 */
	struct ShaderInfo
	{
		ShaderType shaderType;		///< シェーダーの種類
		std::wstring filePath;		///< シェーダーファイルのパス
		///< シェーダーに対応した入力レイアウトタイプ？

		ShaderInfo(ShaderType _shaderType, std::wstring _filePath)
		{
			this->shaderType = _shaderType;
			this->filePath = _filePath;
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
}
