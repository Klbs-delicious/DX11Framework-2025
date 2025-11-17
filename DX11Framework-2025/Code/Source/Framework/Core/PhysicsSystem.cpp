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

#include <Jolt/Physics/PhysicsSystem.h>

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
	// Jolt のメモリ管理を使用するように登録する
	JPH::RegisterDefaultAllocator();

	// ログ出力とアサートを設定する
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertImpl; )

		// 型登録
		JPH::RegisterTypes();

	// 一時領域（10 MB）
	this->tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

	// ジョブシステム（引数なし：スレッド数を自動設定する）
	this->jobSystem = std::make_unique<JPH::JobSystemThreadPool>();

	// 物理システム本体
	this->physics = std::make_unique<JPH::PhysicsSystem>();

	// 最低限のパラメータで初期化
	const JPH::uint maxBodies = 1024;
	const JPH::uint numBodyMutexes = 0;
	const JPH::uint maxBodyPairs = 1024;
	const JPH::uint maxContactConstraints = 1024;

	this->physics->Init(
		maxBodies,
		numBodyMutexes,
		maxBodyPairs,
		maxContactConstraints,
		broadphase,
		bpFilter,
		layerFilter
	);

	return true;
}

/** @brief 物理シミュレーション更新
 *  @param _deltaTime 経過時間
 */
void PhysicsSystem::Step(float _deltaTime)
{
	if (!this->physics) { return; }

	this->physics->Update(
		_deltaTime,
		1,
		this->tempAllocator.get(),
		this->jobSystem.get()
	);
}

/// @brief リソース破棄処理
void PhysicsSystem::Dispose()
{
	// 終了処理
	this->ShutdownJolt();

	// 内部リソースの解放
	this->physics.reset();
	this->jobSystem.reset();
	this->tempAllocator.reset();
}

/// @brief アプリ終了時の Jolt 終了処理
void PhysicsSystem::ShutdownJolt()
{
	JPH::UnregisterTypes();
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
	std::printf("Jolt Assert: %s %s (%s:%u)\n",
		_expr,
		_msg ? _msg : "",
		_file,
		_line);

	// true を返すと __debugbreak() を実行する挙動となる
	return true;
}