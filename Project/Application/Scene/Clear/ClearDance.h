#pragma once
#include <RyoEngine.h>
#include "../../Object/Player/Rig/PlayerRig.h"
#include "../../Direction/Player/HeartBurst.h"

namespace App
{
	//クリアでプレイヤーが万歳で跳ねて喜び、右回りに1回転してからギャルっぽいポーズでリズムに乗る
	class ClearDance
	{
	public:
		ClearDance() = default;
		~ClearDance() = default;

		//モデルを持つのでコピーを禁止する
		ClearDance(const ClearDance&) = delete;
		ClearDance& operator=(const ClearDance&) = delete;

		void Initialize(); //プレイヤーとハートを作る
		void Update(float deltaTime); //喜ぶ、回る、ポーズでリズムを1フレーム進める
		void Draw();

		//ポーズを決めてリズムに乗っているか
		bool IsPosing() const;
		//プレイヤーの頭の上のワールドの位置
		RyoEngine::Vector3 GetHeadPosition() const { return rig_.GetHeadPosition(); }

	private:
		PlayerRig rig_; //踊るプレイヤー
		HeartBurst hearts_; //跳ねたときと拍ごとのハート
		float timer_ = 0.0f; //始まってからの秒数
		int32_t lastBeat_ = -1; //ハートを出した最後の拍
		int32_t lastHop_ = 0; //着地した万歳の回数
	};
}
