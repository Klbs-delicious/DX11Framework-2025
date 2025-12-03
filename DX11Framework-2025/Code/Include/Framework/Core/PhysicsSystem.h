/** @file   PhysicsSystem.h
 *  @brief  JoltPhysics を管理するシステム
 *  @date   2025/11/17
 */
#pragma once

#include <memory>	
#include <array> 

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

		/** @brief ShapeCast 用 BroadPhaseLayerFilter を取得
		 *  @param _layer 自身の ObjectLayer
		 *  @return BroadPhaseLayerFilter への参照
		 */
		[[nodiscard]] const JPH::BroadPhaseLayerFilter& GetBroadPhaseLayerFilter(JPH::ObjectLayer _layer) const;

		/** @brief ShapeCast 用 ObjectLayerFilter を取得
		 *  @param _layer 自身の ObjectLayer
		 *  @return ObjectLayerFilter への参照
		 */
		[[nodiscard]] const JPH::ObjectLayerFilter& GetObjectLayerFilter(JPH::ObjectLayer _layer) const;

	private:
		/// @brief ログ出力（Jolt から呼ばれる）
		static void TraceImpl(const char* _fmt, ...);

		/// @brief アサート失敗時の処理（Jolt から呼ばれる）
		static bool AssertImpl(const char* _expr, const char* _msg, const char* _file, JPH::uint _line);

		/// @brief ShapeCast 用のフィルタ群を初期化する
		void InitializeShapeCastFilters();

	private:
		// 基本リソース
		std::unique_ptr<JPH::TempAllocatorImpl>   tempAllocator;
		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
		std::unique_ptr<JPH::PhysicsSystem>       physics;

		// レイヤー / 衝突フィルタ共通
		BPLayerInterfaceImpl              bpLayerInterface;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseFilter;
		ObjectLayerPairFilterImpl         objectPairFilter;

		// ShapeCast 用プリキャッシュフィルタ
		std::array<std::unique_ptr<JPH::BroadPhaseLayerFilter>, PhysicsLayer::NUM_LAYERS> shapeCastBroadFilters;
		std::array<std::unique_ptr<JPH::ObjectLayerFilter>, PhysicsLayer::NUM_LAYERS> shapeCastObjectFilters;
	};
}