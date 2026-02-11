//-----------------------------------------------------------------------------
// AnimationData.h
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Utils/CommonTypes.h"

#include <string>
#include <vector>
#include <memory>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	struct SkeletonCache;

	/** @struct AnimKeyVec3
	 *  @brief 3Dベクトルアニメーションキー
	 */
	struct AnimKeyVec3
	{
		double ticksTime = 0.0; // ticks
		DX::Vector3 value{};
		AnimKeyVec3() = default;
		AnimKeyVec3(double _time, const DX::Vector3& _v) : ticksTime(_time), value(_v) {}
	};

	/** @struct AnimKeyQuat
	 *  @brief クォータニオンアニメーションキー
	 */
	struct AnimKeyQuat
	{
		double ticksTime = 0.0; // ticks
		DX::Quaternion value{};
		AnimKeyQuat() = default;
		AnimKeyQuat(double _time, const DX::Quaternion& _q) : ticksTime(_time), value(_q) {}
	};

	/** @struct NodeTrack
	 *  @brief ノードごとのアニメーショントラックデータ
	 */
	struct NodeTrack
	{
		int nodeIndex = -1;          ///< 対応ノード index（焼き込み後に確定）
		std::string nodeName = "";   ///< デバッグ用に残す（不要ならBake後に捨てても良い）

		std::vector<AnimKeyVec3> positionKeys{};
		std::vector<AnimKeyQuat> rotationKeys{};
		std::vector<AnimKeyVec3> scaleKeys{};

		bool hasPosition = false;    ///< Bake時に確定
		bool hasRotation = false;    ///< Bake時に確定
		bool hasScale = false;       ///< Bake時に確定
	};

	/** @enum ClipEventId
	 *  @brief アニメーションクリップのイベントID
	 */
	enum class ClipEventId
	{
		// 基本イベント
		Start,
		End,

		// ヒットイベント
		HitOn,
		HitOff,
	};

	///** @struct ClipEvent
	// *  @brief アニメーションクリップのイベント情報
	// */
	//struct ClipEventDef
	//{
	//	float normalizedTime = 0.0f;	///< イベント発生位置（0.0〜1.0）
	//	ClipEventId eventId{};			///< イベントID
	//};

	/** @struct ClipEvent
	 *  @brief アニメーションクリップのイベント情報
	 */
	struct ClipEvent
	{
		float normalizedTime = 0.0f;	///< イベント発生位置（0.0〜1.0）
		ClipEventId eventId{};			///< イベントID
	};

	/** @class ClipEventTable
	 *  @brief アニメーションクリップのイベントテーブル
	 */
	class ClipEventTable
	{
	public:
		/** @brief イベントを登録する
		 *  @param _time イベント発生位置（0.0〜1.0）
		 *  @param _id 登録するイベントID
		 */
		void AddEvent(float _time, ClipEventId _id)
		{
			this->events.push_back({ _time, _id });
		}

		/** @brief イベントを登録する
		 *  @param _event 登録するイベント
		 */
		void AddEvent(const ClipEvent& _event)
		{
			this->events.push_back(_event);
		}

		/** @brief 登録されているイベント配列を取得する
		 *  @return イベント配列への参照
		 */
		const std::vector<ClipEvent>& GetEvents() const
		{
			return this->events;
		}

	private:
		std::vector<ClipEvent> events{};	///< イベント配列
	};

	/** @struct AnimationClip
	 *  @brief アニメーションクリップデータ
	 */
	struct AnimationClip
	{
		std::string name{};							///< クリップ名（デバッグ用）
		std::string keyName{};						///< クリップキー名

		double durationTicks = 0.0;					///< Bake時に「全キー最大時刻」で確定
		double ticksPerSecond = 0.0;				///< 0にならないようインポータで補正
		std::vector<NodeTrack> tracks{};			///< ノードトラック配列

		/** @brief ノード index を焼き込む
		 *  @param _skeletonCache
		 */
		void BakeNodeIndices(const SkeletonCache& _skeletonCache);

		/** @brief 焼き込み時の SkeletonCache ID を取得する
		 *  @return SkeletonCache ID
		 */
		uint64_t GetBakedSkeletonID() const { return this->bakesSkeletonID; }

		/** @brief クリップのイベントテーブルを取得する
		 *  @return クリップのイベントテーブル（無ければ nullptr）
		 */
		const ClipEventTable* GetEventTable() const { return this->eventTable.get(); }

		/** @brief クリップのイベントテーブルを設定する
		 *  @param _table クリップのイベントテーブル
		 */
		void SetEventTable(std::unique_ptr<ClipEventTable> _table) { this->eventTable = std::move(_table); }

	private:
		uint64_t bakesSkeletonID = 0;				///< 焼き込み時の SkeletonCache ID
		std::unique_ptr<ClipEventTable> eventTable;	///< クリップのイベントテーブル
	};

} // namespace Graphics::Import

namespace Graphics::Animation
{
	struct Graphics::Import::SkeletonCache;
	using AnimationClip = Graphics::Import::AnimationClip;

	/** @struct LocalPose
	 *  @brief ローカルポーズ情報（バインド姿勢）
	 */
	struct LocalPose
	{
		std::vector<DX::Matrix4x4> localMatrices{}; ///< ローカル行列（ノード数分）

		/** @brief バインドローカルからリセットする
		 *  @param _skeletonCache スケルトンキャッシュ
		 */
		void ResetFromBindLocal(const Graphics::Import::SkeletonCache& _skeletonCache);
	};

	template<typename StateId>
	/** @struct CrossFadeData
	 *  @brief クロスフェード中の状態情報
	 */
	struct CrossFadeData
	{
		bool isActive = false;		///< クロスフェード中か

		float elapsed = 0.0f;		///< 経過時間（秒）
		float duration = 0.0f;		///< 遷移時間（秒）

		// 遷移元／先の状態
		StateId fromState{};
		StateId toState{};

		// 各クリップの再生時間
		float fromTime = 0.0f;		///< 遷移元の再生時間
		float toTime = 0.0f;		///< 遷移先の再生時間（必ず0開始）
	};

	/** @struct StateDef
	 *  @brief 状態に紐づく設定
	 */
	struct StateDef
	{
		AnimationClip* clip = nullptr;			///< 使用するアニメーションクリップ
		float playbackSpeed = 1.0f;				///< 再生速度
		bool isLoop = true;						///< ループするか
		float recommendedCrossFadeSec = 0.1f;	///< 推奨クロスフェード秒
	};

	//-----------------------------------------------------------------------------
	// StateTable
	//-----------------------------------------------------------------------------
	namespace Detail
	{
		/** @struct EnumHash
		 *  @brief enum class を unordered_map で扱うためのハッシュ
		 */
		template<typename T>
		struct EnumHash
		{
			size_t operator()(T _value) const noexcept
			{
				return static_cast<size_t>(_value);
			}
		};
	}

	/** @class StateTable
	 *  @brief 状態IDから状態定義を引くためのテーブル
	 */
	template<typename StateId>
	class StateTable
	{
	public:
		using Table_t = std::unordered_map<StateId, StateDef, Detail::EnumHash<StateId>>;	///< 状態定義テーブル

	public:
		StateTable() = default;
		~StateTable() = default;

		/** @brief 状態定義を登録する（上書き）
		 *  @param _id 状態ID
		 *  @param _def 状態定義
		 */
		void Set(StateId _id, const StateDef& _def)
		{
			this->table[_id] = _def;
		}

		/** @brief 状態定義を検索する
		 *  @param _id 状態ID
		 *  @return 見つかった場合は状態定義へのポインタ。見つからなければ nullptr
		 */
		const StateDef* Find(StateId _id) const
		{
			const auto it = this->table.find(_id);
			if (it == this->table.end())
			{
				return nullptr;
			}

			return &it->second;
		}

	private:
		Table_t table{};	///< 状態定義テーブル
	};
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Debug::Output
//-----------------------------------------------------------------------------
namespace Graphics::Debug::Output
{
	/** @brief AnimationClip をテキストにダンプする
	 *  @param _filePath 出力先パス
	 *  @param _clip ダンプ対象
	 */
	void DumpToText(const std::string& _filePath, const Graphics::Import::AnimationClip& _clip);
}