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
	struct AnimationClip;
}

namespace Graphics::Debug::Config
{
#if defined(_DEBUG)
	inline constexpr bool BuildAllowDebugOutput = false;
	//inline constexpr bool BuildAllowDebugOutput = true;
#else
	inline constexpr bool BuildAllowDebugOutput = false;
#endif

	inline bool EnableImportDumps = true;

	inline constexpr const char* PoseDumpFilePath = "PoseDump_AnimationComponent.txt";
	inline constexpr const char* SkeletonImportDumpPath = "Import_Skeleton_Dump.txt";
	inline constexpr const char* SkeletonOrderDumpPath = "SkeletonOrderCheck.txt";
	inline constexpr const char* AnimDetailDumpPath = "Anim_Import_Detail_Check.txt";

	inline bool IsImportDumpEnabled()
	{
		return BuildAllowDebugOutput && EnableImportDumps;
	}
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

	/// @brief SkeletonCache の概要を出力する
	void DumpSkeletonImportCheck(
		const char* _filePath,
		const Graphics::Import::SkeletonCache& _cache,
		size_t _maxNodeDumpCount);

	/// @brief order が親子順になっているかを出力する
	void DumpSkeletonOrderCheck(
		const char* _filePath,
		const Graphics::Import::SkeletonCache& _cache);

	/// @brief トラックの焼き込み状態を出力する
	void DumpTrackBakeStatus(
		const char* _filePath,
		const Graphics::Import::AnimationClip& _clip,
		const Graphics::Import::SkeletonCache* _skeletonCache,
		const char* _tag);

	/// @brief BakeNodeIndices 後の妥当性を一度だけ出力する
	void DumpBakeValidationOnce(
		const char* _filePath,
		const Graphics::Import::AnimationClip& _clip,
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const char* _tag);

	/// @brief CPU行列とアップロード行列のペアを一度だけ出力する
	void DumpBoneMatrixCpuGpuPairOnce(
		const char* _filePath,
		int _boneIndex,
		const DX::Matrix4x4& _skinCpu,
		const DX::Matrix4x4& _skinUploaded);
}


