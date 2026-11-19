#include "BreakShapeQueue.h"
#include <algorithm>

namespace App 
{
	//乱数を毎回ちがう値で始める
	BreakShapeQueue::BreakShapeQueue() : random_(std::random_device{}())
	{
	}

	//順番を空にする (次に Peek したときに袋から引き直す)
	void BreakShapeQueue::Reset(bool shuffle)
	{
		shuffle_ = shuffle;
		upcoming_.clear();
		bag_.clear();
	}

	//index番目の形を見る (足りなければ袋から足す)
	BreakShapeType BreakShapeQueue::Peek(int32_t index)
	{
		if (index < 0) {
			index = 0;
		}
		while (static_cast<int32_t>(upcoming_.size()) <= index)
		{
			upcoming_.push_back(Draw());
		}
		return upcoming_[static_cast<size_t>(index)];
	}

	//今の形を捨てて、次の形へ
	void BreakShapeQueue::Advance() 
	{
		Peek(0);
		upcoming_.erase(upcoming_.begin());
	}

	//袋から1つ取り出す (空なら全部入れて混ぜる)
	BreakShapeType BreakShapeQueue::Draw() 
	{
		// 袋が空なら、全部の形を入れて混ぜる
		if (bag_.empty()) 
		{
			for (int32_t i = 0; i < kBreakShapeTypeCount; ++i) 
			{
				bag_.push_back(static_cast<BreakShapeType>(i));
			}
			if (shuffle_) 
			{
				std::shuffle(bag_.begin(), bag_.end(), random_);
			}
		}
		const BreakShapeType shape = bag_.front();
		bag_.erase(bag_.begin());
		return shape;
	}
}
