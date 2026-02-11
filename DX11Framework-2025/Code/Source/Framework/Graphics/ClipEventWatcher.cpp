#include "Include/Framework/Graphics/ClipEventWatcher.h"

namespace Graphics::Import
{
	void ClipEventWatcher::Reset(float _normalizedTime)
	{
		// 初期状態として現在の正規化時間を記録する
		this->prevNormalizedTime = _normalizedTime;
		this->hasPrev = true;
	}

	void ClipEventWatcher::Update(
		const Graphics::Import::ClipEventTable* _table,
		float _nowNormalizedTime,
		std::vector<Graphics::Import::ClipEventId>& _outPassed)
	{
		_outPassed.clear();

		if (!_table)
		{
			// イベントテーブルが無効なら何もしない
			this->prevNormalizedTime = _nowNormalizedTime;
			this->hasPrev = true;
			return;
		}

		if (!this->hasPrev)
		{
			// 初回呼び出し時は何もせず現在時刻を記録するだけにする
			this->prevNormalizedTime = _nowNormalizedTime;
			this->hasPrev = true;
			return;
		}

		const float prev = this->prevNormalizedTime;
		const float now = _nowNormalizedTime;

		if (now < prev)
		{
			// 巻き戻りは無視する
			this->prevNormalizedTime = now;
			return;
		}

		// イベントテーブルを走査して、通過したイベントを列挙する
		const auto& events = _table->GetEvents();
		for (const auto& def : events)
		{
			if (prev < def.normalizedTime && def.normalizedTime <= now)
			{
				_outPassed.push_back(def.eventId);
			}
		}

		this->prevNormalizedTime = now;
	}
} // namespace Graphics::Import