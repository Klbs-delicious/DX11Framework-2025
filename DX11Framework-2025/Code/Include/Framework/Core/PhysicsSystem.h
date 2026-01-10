/** @file   PhysicsSystem.h
 *  @brief  JoltPhysics を管理するシステム
 *  @date   2025/11/17
 */
#pragma once

#include <memory>	
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

#include "Include/Framework/Physics/PhysicsLayers.h"
#include "Include/Framework/Physics/PhysicsContactListener.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>

 // 前方宣言
namespace JPH { class NarrowPhaseQuery; }

namespace Framework::Physics
{
	class Rigidbody3D;
	class Collider3DComponent;

	/// @brief 接触タイプ
	enum class ContactType
	{
		Trigger_Entered,
		Trigger_Stayed,
		Trigger_Exited,

		Coll_Entered,
		Coll_Stayed,
		Coll_Exited,
		Max
	};

	/// @brief コライダー識別キー
	struct ColliderKey
	{
		JPH::BodyID bodyID;
		int colliderID;

		bool operator==(const ColliderKey& other) const noexcept
		{
			return bodyID == other.bodyID && colliderID == other.colliderID;
		}
	};

	/// @brief コライダー識別キーのハッシュ関数
	struct ColliderKeyHash
	{
		std::size_t operator()(const ColliderKey& _key) const noexcept
		{
			// BodyID と SubShapeID の値を混ぜる
			// BodyID は GetIndex と GetSequenceNumber の組み合わせで安定させる
			std::size_t h1 = std::hash<JPH::uint32>()(_key.bodyID.GetIndex()) ^ (std::hash<JPH::uint32>()(_key.bodyID.GetSequenceNumber()) << 1);
			std::size_t h2 = std::hash<int>()(_key.colliderID);

			return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
		}
	};

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
		void AddContactPair(ColliderKey _bodyA, ColliderKey _bodyB);

		/// @brief 接触イベントを処理する
		void ProcessContactEvents();

		/** @brief 接触イベントハンドル
		 *  @param _type   接触タイプ
		 *  @param _bodyA  剛体Aの BodyID
		 *  @param _bodyB  剛体Bの BodyID
		 */
		void HandleContact(ContactType _type, ColliderKey _bodyA, ColliderKey _bodyB);

		/** @brief BodyID がセンサーボディかどうか調べる
		 *  @param _id 調べる BodyID
		 *  @return センサーボディなら true
		 */
		[[nodiscard]] bool IsSensorBody(JPH::BodyID _id);

		/** @brief 接触タイプをトリガー用に変換する
		 *  @param _type 変換する接触タイプ
		 */
		void ConvertToTrigger(ContactType& _type);

		/** @brief BodyID が有効かどうか調べる
		 *  @param _body 調べる BodyID
		 *  @return 有効なら true
		 */
		[[nodiscard]] bool IsBodyValid(JPH::BodyID _body);

		/** @brief BodyID と Rigidbody3D の関連付けを登録する
		 *  @param _bodyID  登録する BodyID
		 *  @param _rigidbody 関連付ける Rigidbody3D
		 */
		void RegisterRigidbody3D(JPH::BodyID _bodyID, Rigidbody3D* _rigidbody);

		/** @brief BodyID と Rigidbody3D の関連付けを解除する
		 *  @param _bodyID 解除する BodyID
		 */
		void UnregisterRigidbody3D(JPH::BodyID _bodyID);

		/** @brief BodyID から Rigidbody3D を取得する
		 *  @param _bodyID 取得する BodyID
		 *  @return 対応する Rigidbody3D（存在しない場合は nullptr）
		 */
		Rigidbody3D* GetRigidbody3D(JPH::BodyID _bodyID);

		/** @brief Collider3DComponent にIDを割り当てる
		 *  @param _collider 対象コライダー
		 *  @return 割り当てたID
		 */
		int AssignColliderID(Collider3DComponent* _collider);

		/** @brief BodyID と Collider3DComponent の関連付けを登録する
		 *  @param _bodyID  登録する BodyID
		 *  @param _collider 関連付けるコライダー
		 */
		void RegisterCollider3D(JPH::BodyID _bodyID, Collider3DComponent* _collider);

		/** @brief BodyID と Collider3DComponent の関連付けを解除する
		 *  @param _bodyID 解除する BodyID
		 */
		void UnregisterCollider3D(JPH::BodyID _bodyID);

		/** @brief ColliderID から Collider3DComponent を取得する
		 *  @param _colliderID 取得する ColliderID
		 *  @return 対応する Collider3DComponent（存在しない場合は nullptr）
		 */
		Collider3DComponent* GetCollider3D(int _colliderID);

		/** @brief BodyID から Collider3DComponent を取得する
		 *  @param _bodyID 取得する BodyID
		 *  @return 対応する Collider3DComponent（存在しない場合は nullptr）
		 */
		Collider3DComponent* GetCollider3D(JPH::BodyID _bodyID);

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
		std::array<std::unique_ptr<JPH::BroadPhaseLayerFilter>, Framework::Physics::PhysicsLayer::NUM_LAYERS>	shapeCastBroadFilters;	///< ShapeCast 用 BroadPhaseLayerFilter
		std::array<std::unique_ptr<JPH::ObjectLayerFilter>, Framework::Physics::PhysicsLayer::NUM_LAYERS>		shapeCastObjectFilters;	///< ShapeCast 用 ObjectLayerFilter

		// 衝突検知
		std::unordered_map<ColliderKey, std::unordered_set<ColliderKey, ColliderKeyHash>, ColliderKeyHash>	currContact;		///< 今フレームの接触情報
		std::unordered_map<ColliderKey, std::unordered_set<ColliderKey, ColliderKeyHash>, ColliderKeyHash>	prevContact;		///< 前フレームの接触情報
		PhysicsContactListener												contactListener;	///< コンタクトリスナー

		// 追加: 衝突テーブル用ミューテックス
		std::mutex contactMutex;

		std::unordered_map < JPH::BodyID, Framework::Physics::Rigidbody3D* > bodyMap;				///< BodyIDに対するRigidbody3Dマップ
		std::unordered_map < int, Framework::Physics::Collider3DComponent* > colliderIDMap;		///< ColliderIDに対するCollider3DComponentマップ
		std::unordered_map < JPH::BodyID, Framework::Physics::Collider3DComponent* > bodyColliderMap;	///< BodyIDに対するCollider3DComponentマップ
		int nextColliderID = 1;																		///< 次に割り当てるColliderID
	};
} // namespace Framework::Physics
