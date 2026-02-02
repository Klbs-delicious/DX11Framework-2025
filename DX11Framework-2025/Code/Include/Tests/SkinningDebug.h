/** @file   SkinningDebug.h
 *  @brief  SkeletonCache のスキニング基準情報ダンプ
 *  @date   2026/02/01
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <string>
#include "Include/Framework/Utils/CommonTypes.h"

namespace Graphics::Import
{
	struct SkeletonCache;
	struct Pose;
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Debug::Output
//-----------------------------------------------------------------------------
namespace Graphics::Debug::Output
{
	/** @brief SkeletonCache のスキニング基準情報をテキストへ出力する（ロード直後に1回だけ呼ぶ想定）
	 *  @param _filePath 出力ファイルパス
	 *  @param _skeletonCache SkeletonCache
	 */
	void DumpSkinningBasisToText(
		const std::string& _filePath,
		const Graphics::Import::SkeletonCache& _skeletonCache);

	/// @brief 行列が単位行列からどれだけズレているかを返す
	float MaxAbsDiffIdentity(const DX::Matrix4x4& _m);

	/** @brief Reset 直後の pose を bind pose として検証する
	 *  @param _skeletonCache SkeletonCache
	 *  @param _pose Pose
	 */
	void DumpBindPoseGlobalCheckOnce(
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const Graphics::Import::Pose& _pose);

	/** @brief 各 bone の bind skin 行列が単位になっているか確認する
	 *  @param _skeletonCache SkeletonCache
	 *  @param _pose Pose
	 */
	void DumpBindPoseSkinCheckOnce(
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const Graphics::Import::Pose& _pose);

	// 見出しを出す
	void LogHeader(const char* _title);

	// 行列を行単位で出す
	void PrintMatrix4x4(const char* _label, const DX::Matrix4x4& _m);

	// 単位行列との差の最大絶対値を返す
	float MaxAbsDiffIdentity(const DX::Matrix4x4& _m);

	// SkeletonCache の基準情報をテキストへ出す
	void DumpSkeletonBasisToText(const char* _filename, const Graphics::Import::SkeletonCache& _cache);
}

