/** @file   PhysicsSystem.h
 *  @brief  JoltPhysics を管理するシステム
 *  @date   2025/11/17
 */
#pragma once

#include <memory>	
#include <array>
#include <unordered_set>
#include <unordered_map>

#include "Include/Framework/Physics/PhysicsLayers.h"
#include "Include/Framework/Physics/PhysicsContactListener.h"

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

		/** @brief 接触した剛体ペアを追加
		 *  @param _bodyA       ぶつかった剛体A
		 *  @param _bodyB       ぶつかった剛体B
		 */
		void AddContactPair(JPH::BodyID _bodyA, JPH::BodyID _bodyB);

		/// @brief 接触イベントを処理する
		void ProcessContactEvents();

		/** @brief BodyID が有効かどうか調べる
		 *  @param _body 調べる BodyID
		 *  @return 有効なら true
		 */
		[[nodiscard]] bool IsBodyValid(JPH::BodyID _body);

	private:
		/// @brief ログ出力（Jolt から呼ばれる）
		static void TraceImpl(const char* _fmt, ...);

		/// @brief アサート失敗時の処理（Jolt から呼ばれる）
		static bool AssertImpl(const char* _expr, const char* _msg, const char* _file, JPH::uint _line);

		/// @brief ShapeCast 用のフィルタ群を初期化する
		void InitializeShapeCastFilters();

	private:
		// 基本リソース
		std::unique_ptr<JPH::TempAllocatorImpl>		tempAllocator;	///< 一時アロケータ
		std::unique_ptr<JPH::JobSystemThreadPool>	jobSystem;		///< ジョブシステム
		std::unique_ptr<JPH::PhysicsSystem>			physics;		///< 物理システム

		// レイヤー / 衝突フィルタ共通
		BPLayerInterfaceImpl					bpLayerInterface;			///< BroadPhaseLayer インターフェース
		ObjectVsBroadPhaseLayerFilterImpl		objectVsBroadPhaseFilter;	///< Object vs BroadPhase レイヤーフィルタ
		ObjectLayerPairFilterImpl				objectPairFilter;			///< ObjectLayer ペアフィルタ

		// ShapeCast 用プリキャッシュフィルタ
		std::array<std::unique_ptr<JPH::BroadPhaseLayerFilter>, PhysicsLayer::NUM_LAYERS>	shapeCastBroadFilters;	///< ShapeCast 用 BroadPhaseLayerFilter
		std::array<std::unique_ptr<JPH::ObjectLayerFilter>, PhysicsLayer::NUM_LAYERS>		shapeCastObjectFilters;	///< ShapeCast 用 ObjectLayerFilter

		// 衝突差分
		std::unordered_map<JPH::BodyID, std::unordered_set<JPH::BodyID>>	currContact;	///< 今フレームの接触情報
		std::unordered_map<JPH::BodyID, std::unordered_set<JPH::BodyID>>	prevContact;	///< 前フレームの接触情報

		// コンタクトリスナー
		PhysicsContactListener	contactListener;	///< コンタクトリスナー
	};
}