/** @file   PhysicsSystem.cpp
 *  @brief  PhysicsSystem の実装
 *  @date   2025/11/17
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "Include/Framework/Core/PhysicsSystem.h"
#include "Include/Framework/Physics/PhysicsLayers.h"
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include <Jolt/RegisterTypes.h>

namespace Framework::Physics
{
	//-----------------------------------------------------------------------------
	// PhysicsSystem Class
	//-----------------------------------------------------------------------------

	/// @brief コンストラクタ
	PhysicsSystem::PhysicsSystem()
		: tempAllocator(nullptr)
		, jobSystem(nullptr)
		, physics(nullptr)
		, bpLayerInterface()
		, objectVsBroadPhaseFilter()
		, objectPairFilter()
		, shapeCastBroadFilters()
		, shapeCastObjectFilters()
		, currContact()
		, prevContact()
		, contactListener(*this)
		, bodyMap()
	{}

	/// @brief デストラクタ
	PhysicsSystem::~PhysicsSystem()
	{
		this->Dispose();
	}

	/** @brief 初期化処理
	 *  @return 成功したら true
	 */
	bool PhysicsSystem::Initialize()
	{
		// すでに初期化されていたら何もしない
		if (this->physics)
		{
			return true;
		}

		// メモリ管理
		JPH::RegisterDefaultAllocator();

		// ログ
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertImpl;)

		// Factory の作成と型登録
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		// 一時アロケータ
		this->tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

		// スレッド数を決定（スレッド数は CPU コア数 − 1）
		unsigned int hwThreads = std::thread::hardware_concurrency();
		if (hwThreads == 0)
		{
			hwThreads = 1;
		}

		int numThreads = static_cast<int>(hwThreads) - 1;
		if (numThreads < 0)
		{
			numThreads = 0;
		}

		const JPH::uint maxJobs = JPH::cMaxPhysicsJobs;
		const JPH::uint maxBarriers = JPH::cMaxPhysicsBarriers;

		this->jobSystem = std::make_unique<JPH::JobSystemThreadPool>();
		this->jobSystem->Init(maxJobs, maxBarriers, numThreads);

		// PhysicsSystem の生成
		this->physics = std::make_unique<JPH::PhysicsSystem>();

		const JPH::uint maxBodies = 2048;
		const JPH::uint numBodyMutexes = 0;
		const JPH::uint maxBodyPairs = 1024;
		const JPH::uint maxContactConstraints = 1024;

		// BroadPhaseLayerInterface / 衝突フィルタの登録
		this->physics->Init(
			maxBodies,
			numBodyMutexes,
			maxBodyPairs,
			maxContactConstraints,
			this->bpLayerInterface,
			this->objectVsBroadPhaseFilter,
			this->objectPairFilter
		);

		// ShapeCast 用フィルタ群の初期化
		this->InitializeShapeCastFilters();

		// コンタクトリスナーの登録
		this->physics->SetContactListener(&this->contactListener);

		return true;
	}

	/** @brief 物理シミュレーション更新
	 *  @param _deltaTime 経過時間
	 */
	void PhysicsSystem::Step(float _deltaTime)
	{
		if (!this->physics)
		{
			return;
		}

		this->physics->Update(
			_deltaTime,
			1,
			this->tempAllocator.get(),
			this->jobSystem.get()
		);
	}

	/// @brief 内部リソースの解放処理
	void PhysicsSystem::Dispose()
	{
		// まだ初期化されていない場合
		if (!this->physics && !this->jobSystem && !this->tempAllocator)
		{
			return;
		}

		// PhysicsSystem の解放（内部で Body や Shape が破棄される）
		this->physics.reset();

		// ジョブシステム / 一時アロケータを解放
		this->jobSystem.reset();
		this->tempAllocator.reset();

		// ShapeCast フィルタも明示的に破棄
		for (auto& f : this->shapeCastBroadFilters)
		{
			f.reset();
		}
		for (auto& f : this->shapeCastObjectFilters)
		{
			f.reset();
		}

		// Jolt のグローバル終了処理
		this->ShutdownJolt();
	}

	/// @brief Jolt の型登録解除（アプリ終了時に1回だけ呼ぶ）
	void PhysicsSystem::ShutdownJolt()
	{
		JPH::UnregisterTypes();

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	/** @brief 剛体操作用のインターフェースを取得
	 *  @return 剛体インターフェースの参照
	 */
	JPH::BodyInterface& PhysicsSystem::GetBodyInterface()
	{
		return this->physics->GetBodyInterface();
	}

	/** @brief 剛体ロック用のインターフェースを取得
	 *  @return 剛体ロックインターフェースの参照
	 */
	JPH::BodyLockInterface& PhysicsSystem::GetBodyLockInterface()
	{
		const JPH::BodyLockInterface& iface = this->physics->GetBodyLockInterface();
		return const_cast<JPH::BodyLockInterface&>(iface);
	}

	/** @brief NarrowPhaseQuery を取得する
	 *  @return NarrowPhaseQuery への参照
	 */
	const JPH::NarrowPhaseQuery& PhysicsSystem::GetNarrowPhaseQuery() const
	{
		if (!this->physics)
		{
			static JPH::NarrowPhaseQuery dummy;
			return dummy;
		}
		return this->physics->GetNarrowPhaseQuery();
	}

	/** @brief 接触した剛体ペアを追加する
	 *  @param _bodyA       ぶつかった剛体1
	 *  @param _bodyB       ぶつかった剛体2
	 *	@detail 
	 *		- BodyIDを昇順に揃える
	 *		- Jolt側で当たった時、当たっている時に呼び出される
	 */
	void PhysicsSystem::AddContactPair(JPH::BodyID _bodyA, JPH::BodyID _bodyB)
	{
		if (_bodyA > _bodyB)
		{
			// BodyIDを昇順に揃える
			std::swap(_bodyA, _bodyB);
		}
		this->currContact[_bodyA].insert(_bodyB);
	}

	void PhysicsSystem::ProcessContactEvents()
	{
		auto& bodyInterface = this->physics->GetBodyInterface();

		//=======================================
		// 無効になったBodyを履歴から取り除く
		//=======================================
		auto cleanContactTable = [&](auto& table)
		{
			for (auto itA = table.begin(); itA != table.end(); )
			{
				if (!IsBodyValid(itA->first))
				{
					itA = table.erase(itA);
					continue;
				}

				for (auto itB = itA->second.begin(); itB != itA->second.end(); )
				{
					if (!IsBodyValid(*itB))
						itB = itA->second.erase(itB);
					else
						++itB;
				}
				++itA;
			}
		};

		cleanContactTable(this->currContact);
		cleanContactTable(this->prevContact);

		//=======================================
		// Enter & Stay 判定
		//=======================================
		for (auto& [bodyA, currentSet] : this->currContact)
		{
			auto& prevSet = this->prevContact[bodyA];

			for (const auto& bodyB : currentSet)
			{
				bool isPrev = (prevSet.count(bodyB) > 0);

				if (!isPrev)
				{
					//std::cout << "Contact Enter: " << bodyA.GetIndex() << " - " << bodyB.GetIndex() << "\n";
					this->HandleContact(ContactType::Coll_Entered, bodyA, bodyB);
				}
				else
				{
					//std::cout << "Contact Stay : " << bodyA.GetIndex() << " - " << bodyB.GetIndex() << "\n";
					this->HandleContact(ContactType::Coll_Stayed, bodyA, bodyB);
				}
			}
		}

		//===========================
		// Exit（安定化処理）
		//===========================
		for (auto& [bodyA, prevSet] : this->prevContact)
		{
			auto itCurr = this->currContact.find(bodyA);
			const auto* currSet = (itCurr != this->currContact.end()) ? &itCurr->second : nullptr;

			for (auto& bodyB : prevSet)
			{
				bool stillContact = currSet && currSet->count(bodyB) > 0;

				if (!stillContact)
				{
					//std::cout << "Contact Exit  : " << bodyA.GetIndex() << " - " << bodyB.GetIndex() << "\n";
					this->HandleContact(ContactType::Coll_Exited, bodyA, bodyB);
				}
			}
		}

		//=======================================
		// 履歴の更新
		//=======================================
		this->prevContact = this->currContact;	// ここで前回の状態になる
		this->currContact.clear();				// 現在フレームの情報をリセット
	}

	/** @brief BodyID が有効かどうか調べる
	 *  @param _body 調べる BodyID
	 *  @return 有効なら true
	 */
	void PhysicsSystem::HandleContact(ContactType _type, JPH::BodyID _bodyA, JPH::BodyID _bodyB)
	{
		{
			// Rigidbody3D を取得する
			auto rbA = GetRigidbody3D(_bodyA);
			auto rbB = GetRigidbody3D(_bodyB);
			if (!rbA || !rbB) return;

			// センサーボディかどうか調べる
			bool aIsSensor = IsSensorBody(_bodyA);
			bool bIsSensor = IsSensorBody(_bodyB);

			ContactType typeA = _type;
			ContactType typeB = _type;

			// センサーならトリガーイベントに変換する
			if (aIsSensor) ConvertToTrigger(typeA);
			if (bIsSensor) ConvertToTrigger(typeB);

			// イベント発行
			rbA->DispatchContactEvent(typeA, GetCollider3D(_bodyA), GetCollider3D(_bodyB));
			rbB->DispatchContactEvent(typeB, GetCollider3D(_bodyB), GetCollider3D(_bodyA));
		}
	}

	/** @brief BodyID が有効かどうか調べる
	 *  @param _body 調べる BodyID
	 *  @return 有効なら true
	 */
	bool PhysicsSystem::IsSensorBody(JPH::BodyID _id)
	{
		JPH::BodyLockRead lock(this->physics->GetBodyLockInterface(), _id);
		if (!lock.Succeeded()) { return false; }
		return lock.GetBody().IsSensor();
	}

	void PhysicsSystem::ConvertToTrigger(ContactType& _type)
	{
		if (_type == ContactType::Coll_Entered) _type = ContactType::Trigger_Entered;
		else if (_type == ContactType::Coll_Stayed) _type = ContactType::Trigger_Stayed;
		else if (_type == ContactType::Coll_Exited) _type = ContactType::Trigger_Exited;
	}

	/** @brief BodyID と Rigidbody3D の関連付けを登録する
	 *  @param _bodyID  登録する BodyID
	 *  @param _rigidbody 関連付ける Rigidbody3D
	 */
	void PhysicsSystem::RegisterRigidbody3D(JPH::BodyID _bodyID, Rigidbody3D* _rigidbody)
	{
		// 登録
		this->bodyMap[_bodyID] =/* { _rigidbody, _bodyID.GetSequenceNumber() };*/_rigidbody;
	}

	/** @brief BodyID と Rigidbody3D の関連付けを解除する
	 *  @param _bodyID 解除する BodyID
	 */
	void PhysicsSystem::UnregisterRigidbody3D(JPH::BodyID _bodyID)
	{
		this->bodyMap.erase(_bodyID);
	}

	/** @brief BodyID から Rigidbody3D を取得する
	 *  @param _bodyID 取得する BodyID
	 *  @return 対応する Rigidbody3D（存在しない場合は nullptr）
	 */
	Rigidbody3D* PhysicsSystem::GetRigidbody3D(JPH::BodyID _bodyID)
	{
		auto it = this->bodyMap.find(_bodyID);
		if (it != this->bodyMap.end())
		{
			return it->second;
		}

		// 存在しない
		return nullptr;
	}

	/** @brief BodyID と Collider3DComponent の関連付けを登録する
	 *  @param _bodyID  登録する BodyID
	 *  @param _collider 関連付ける Collider3DComponent
	 */
	void PhysicsSystem::RegisterCollider3D(JPH::BodyID _bodyID, Collider3DComponent* _collider)
	{
		// 登録
		this->colliderMap[_bodyID] = _collider;
	}

	/** @brief BodyID と Collider3DComponent の関連付けを解除する
	 *  @param _bodyID 解除する BodyID
	 */
	void PhysicsSystem::UnregisterCollider3D(JPH::BodyID _bodyID)
	{
		this->colliderMap.erase(_bodyID);
	}

	/** @brief BodyID から Collider3DComponent を取得する
	 *  @param _bodyID 取得する BodyID
	 *  @return 対応する Collider3DComponent（存在しない場合は nullptr）
	 */
	Collider3DComponent* PhysicsSystem::GetCollider3D(JPH::BodyID _bodyID)
	{
		auto it = this->colliderMap.find(_bodyID);
		if (it != this->colliderMap.end())
		{
			return it->second;
		}
		// 存在しない
		return nullptr;
	}

	/// @brief Jolt ログ出力
	void PhysicsSystem::TraceImpl(const char* _fmt, ...)
	{
		va_list args;
		va_start(args, _fmt);
		std::vprintf(_fmt, args);
		std::printf("\n");
		va_end(args);
	}

	/// @brief Jolt アサート失敗時の処理
	bool PhysicsSystem::AssertImpl(const char* _expr, const char* _msg, const char* _file, JPH::uint _line)
	{
		std::printf(
			"Jolt Assert: %s %s (%s:%u)\n",
			_expr,
			_msg ? _msg : "",
			_file,
			_line
		);

		// true を返すと __debugbreak() を実行する挙動となる
		return true;
	}

	/// @brief ShapeCast 用のフィルタ群を初期化する
	void PhysicsSystem::InitializeShapeCastFilters()
	{
		for (JPH::ObjectLayer layer = 0; layer < PhysicsLayer::NUM_LAYERS; ++layer)
		{
			this->shapeCastBroadFilters[layer] =
				std::make_unique<ShapeCastBroadPhaseLayerFilter>(
					&this->bpLayerInterface,
					&this->objectVsBroadPhaseFilter,
					layer
				);

			this->shapeCastObjectFilters[layer] =
				std::make_unique<ShapeCastObjectLayerFilter>(
					&this->objectPairFilter,
					layer
				);
		}
	}

	/** @brief ShapeCast 用 BroadPhaseLayerFilter を取得
	 *  @param _layer 自身の ObjectLayer
	 */
	const JPH::BroadPhaseLayerFilter& PhysicsSystem::GetBroadPhaseLayerFilter(JPH::ObjectLayer _layer) const
	{
		return *(this->shapeCastBroadFilters[_layer]);
	}

	/** @brief ShapeCast 用 ObjectLayerFilter を取得
	 *  @param _layer 自身の ObjectLayer
	 */
	const JPH::ObjectLayerFilter& PhysicsSystem::GetObjectLayerFilter(JPH::ObjectLayer _layer) const
	{
		return *(this->shapeCastObjectFilters[_layer]);
	}

	/** @brief BodyID が有効かどうか調べる
	 *  @param _body 調べる BodyID
	 *  @return 有効なら true
	 */
	bool PhysicsSystem::IsBodyValid(JPH::BodyID _body)
	{
		auto& iface = this->physics->GetBodyInterface();

		// 破棄されていない限り有効扱いする
		return iface.IsAdded(_body);
	}
} // namespace Framework::Physics