/** @file   PhysicsSystem.cpp
 *  @brief  PhysicsSystem の実装
 *  @date   2025/11/17
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <cstdarg>
#include <cstdio>

#include "Include/Framework/Core/PhysicsSystem.h"
#include "Include/Framework/Physics/PhysicsLayers.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/TempAllocator.h>

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
	{
	}

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
		if (this->physics){ return true; }

		// メモリ管理
		JPH::RegisterDefaultAllocator();

		// ログ
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertImpl;)

		// ★ Factory の作成
		JPH::Factory::sInstance = new JPH::Factory();

		// 型登録
		JPH::RegisterTypes();

		// 一時アロケータ
		this->tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

		//-------------
		// スレッド数を決定
		//-------------
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

		//-------------
		// ジョブシステム初期化
		//-------------
		const JPH::uint maxJobs = JPH::cMaxPhysicsJobs;
		const JPH::uint maxBarriers = JPH::cMaxPhysicsBarriers;

		this->jobSystem = std::make_unique<JPH::JobSystemThreadPool>();
		this->jobSystem->Init(maxJobs, maxBarriers, numThreads);

		// PhysicsSystem
		this->physics = std::make_unique<JPH::PhysicsSystem>();

		const JPH::uint maxBodies = 1024;
		const JPH::uint numBodyMutexes = 0;
		const JPH::uint maxBodyPairs = 1024;
		const JPH::uint maxContactConstraints = 1024;

		this->physics->Init(
			maxBodies,
			numBodyMutexes,
			maxBodyPairs,
			maxContactConstraints,
			this->broadphase,
			this->bpFilter,
			this->layerFilter
		);

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
		if (!this->physics && !this->jobSystem && !this->tempAllocator)
		{
			return; // まだ初期化されていない or すでに破棄済
		}

		// PhysicsSystem の解放（内部で Body や Shape が破棄される）
		this->physics.reset();

		// ジョブシステム / 一時アロケータを解放
		this->jobSystem.reset();
		this->tempAllocator.reset();

		// Jolt のグローバル終了処理
		this->ShutdownJolt();
	}

	/// @brief Jolt の型登録解除（アプリ終了時に1回だけ呼ぶ）
	void PhysicsSystem::ShutdownJolt()
	{
		// Jolt 型登録を解除
		JPH::UnregisterTypes();

		// Factory を解放
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
		// Jolt 側は const BodyLockInterfaceLocking& を返すので、
		// 基底クラス BodyLockInterface にアップキャストしてから const_cast する
		const JPH::BodyLockInterface& iface = this->physics->GetBodyLockInterface();
		return const_cast<JPH::BodyLockInterface&>(iface);
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
}