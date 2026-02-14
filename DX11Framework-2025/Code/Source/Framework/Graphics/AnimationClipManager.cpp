/** @file   AnimationClipManager.cpp
 *  @brief  アニメーションクリップ管理
 *  @date   2026/01/13
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationClipManager.h"

#include <iostream>
#include <algorithm>

//-----------------------------------------------------------------------------
// AnimationClipManager class
//-----------------------------------------------------------------------------

AnimationClipManager::AnimationClipManager()
	: defaultClip(nullptr)
{
	//---------------------------------------------------------
	// クリップ情報の事前登録
	//---------------------------------------------------------
	this->AddClipInfo("Walk", "Assets/Animations/Wheelbarrow Walk Turn.fbx");
	this->AddClipInfo("Punch", "Assets/Animations/Zombie Punching.fbx");
	this->AddClipInfo("Run", "Assets/Animations/Fast Run.fbx");
	this->AddClipInfo("Jump", "Assets/Animations/Jumping.fbx");
	this->AddClipInfo("Fight", "Assets/Animations/Fighting.fbx");
	this->AddClipInfo("Dance", "Assets/Animations/dance.fbx");
	this->AddClipInfo("Idle", "Assets/Animations/Idle.fbx");
	this->AddClipInfo("HeadHit", "Assets/Animations/Head Hit.fbx");
	this->AddClipInfo("Dodge", "Assets/Animations/Dodging Right.fbx");

	//---------------------------------------------------------
	// イベントテーブル構築
	//---------------------------------------------------------
	this->eventDefMap.emplace(
		"Punch",
		std::vector<Graphics::Import::ClipEvent>
	{
		{ 0.25f, Graphics::Import::ClipEventId::HitOn },
		{ 0.30f, Graphics::Import::ClipEventId::HitOff }
	});
}

AnimationClipManager::~AnimationClipManager()
{
	this->Clear();
}

/** @brief リソースを登録する
 *  @param _key リソースのキー
 *  @return 登録済みならそれ、失敗なら nullptr
 */
Graphics::Import::AnimationClip* AnimationClipManager::Register(const std::string& _key)
{
	// 既に登録済みならそれを返す
	{
		auto it = this->clipMap.find(_key);
		if (it != this->clipMap.end())
		{
			return it->second.get();
		}
	}

	// ファイルパス情報が無いなら登録できない
	auto infoIt = this->clipInfoMap.find(_key);
	if (infoIt == this->clipInfoMap.end())
	{
		std::cerr << "[Error] AnimationClipManager::Register: ClipInfo not found: " << _key << std::endl;
		return nullptr;
	}

	const std::string& filename = infoIt->second;

	// 読み込み
	auto clip = std::make_unique<Graphics::Import::AnimationClip>();

	// メンバ importer を使う（ローカル生成はしない）
	if (!this->importer.LoadSingleClip(filename, *clip))
	{
		std::cerr << "[Error] AnimationClipManager::Register: Import failed: " << _key
			<< " (" << filename << ")" << std::endl;
		return nullptr;
	}
	clip->keyName = _key;

	// ロード直後にイベントテーブルを適用（定義があれば）
	{
		auto defIt = this->eventDefMap.find(_key);
		if (defIt != this->eventDefMap.end())
		{
			this->BuildEventTable(*clip, defIt->second);
		}
	}

	Graphics::Import::AnimationClip* clipRaw = clip.get();
	this->clipMap.emplace(_key, std::move(clip));

	if (this->defaultClip == nullptr)
	{
		// デフォルトが無ければ最初に登録できたものをデフォルトにする
		this->defaultClip = clipRaw;
	}

	return clipRaw;
}

/** @brief リソースの登録を解除する
 *  @param _key リソースのキー
 */
void AnimationClipManager::Unregister(const std::string& _key)
{
	auto it = this->clipMap.find(_key);
	if (it == this->clipMap.end())
	{
		return;
	}

	// デフォルトが消える場合は nullptr に戻す（必要なら別キーに差し替えても良い）
	if (this->defaultClip == it->second.get())
	{
		this->defaultClip = nullptr;
	}

	this->clipMap.erase(it);
}

/** @brief キーに対応するリソースを取得する
 *  @param _key リソースのキー
 *  @return 見つからなかった場合は nullptr
 */
Graphics::Import::AnimationClip* AnimationClipManager::Get(const std::string& _key)
{
	auto it = this->clipMap.find(_key);
	if (it == this->clipMap.end())
	{
		return nullptr;
	}

	return it->second.get();
}

Graphics::Import::AnimationClip* AnimationClipManager::Default() const
{
	if(!this->defaultClip){
		std::cerr << "[Warning] AnimationClipManager::Default: Default clip is not set." << std::endl;
		return nullptr;
	}
	return this->defaultClip;
}

void AnimationClipManager::AddClipInfo(const std::string& _key, const std::string& _filename)
{
	this->clipInfoMap.emplace(_key, _filename);
}

void AnimationClipManager::Clear()
{
	this->clipMap.clear();
	this->defaultClip = nullptr;
}

//-----------------------------------------------------------------------------
// AnimationClipManager : BuildEventTable
//-----------------------------------------------------------------------------

void AnimationClipManager::BuildEventTable(
	Graphics::Import::AnimationClip& _clip,
	const std::vector<Graphics::Import::ClipEvent>& _clipEvents)
{
	auto table = std::make_unique<Graphics::Import::ClipEventTable>();

	// クリップの長さを秒に変換する
	const double tps = (_clip.ticksPerSecond > 0.0) ? _clip.ticksPerSecond : 1.0;
	const float durationSec = static_cast<float>(_clip.durationTicks / tps);

	for (const auto& event : _clipEvents)
	{
		table->AddEvent(event.normalizedTime, event.eventId);
	}

	_clip.SetEventTable(std::move(table));
}