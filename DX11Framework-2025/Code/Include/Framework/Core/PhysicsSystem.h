/** @file   PhysicsSystem.h
 *  @brief  JoltPhysics を管理するシステム
 *  @date   2025/11/17
 */
#pragma once

#include <memory>

#include "Include/Framework/Physics/PhysicsLayers.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>

// 前方宣言（ヘッダ依存を減らす）
namespace JPH { class NarrowPhaseQuery; }

namespace Framework::Physics
{
	/** @class  PhysicsSystem
	 *  @brief  JoltPhysics の初期化・更新・破棄を行うシステム
	 */
	class PhysicsSystem
	{
	public:
		/// @brief コンストラクタ
		PhysicsSystem();

		/// @brief デストラクタ
		~PhysicsSystem();

		/** @brief 初期化処理
		 *  @return 初期化に成功したら true
		 */
		bool Initialize();

		/** @brief 物理シミュレーションを進める
		 *  @param _deltaTime 経過時間
		 */
		void Step(float _deltaTime);

		/// @brief 内部リソースの解放処理
		void Dispose();

		/// @brief Jolt の型登録解除（アプリ終了時に1回だけ呼ぶ）
		static void ShutdownJolt();

		/** @brief 剛体操作用のインターフェースを取得
		 *  @return 剛体インターフェースの参照
		 */
		[[nodiscard]] JPH::BodyInterface& GetBodyInterface();

		/** @brief 剛体ロック用のインターフェースを取得
		 *  @return 剛体ロックインターフェースの参照
		 */
		[[nodiscard]] JPH::BodyLockInterface& GetBodyLockInterface();

		/** @brief 詳細な衝突クエリ用インターフェースを取得
		 *  @return NarrowPhaseQuery の参照
		 */
		[[nodiscard]] const JPH::NarrowPhaseQuery& GetNarrowPhaseQuery() const;

	private:
		/// @brief ログ出力（Jolt から呼ばれる）
		static void TraceImpl(const char* _fmt, ...);

		/// @brief アサート失敗時の処理（Jolt から呼ばれる）
		static bool AssertImpl(const char* _expr, const char* _msg, const char* _file, JPH::uint _line);

	private:
		std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;	///< 一時領域管理
		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;	///< ジョブシステム
		std::unique_ptr<JPH::PhysicsSystem> physics;			///< 物理システム

		Framework::Physics::BPLayerInterfaceImpl broadphase;			///< レイヤー管理
		Framework::Physics::ObjectVsBroadPhaseLayerFilterImpl bpFilter;	///< レイヤー間フィルタ
		Framework::Physics::ObjectLayerPairFilterImpl layerFilter;		///< レイヤーペアフィルタ
	};
}