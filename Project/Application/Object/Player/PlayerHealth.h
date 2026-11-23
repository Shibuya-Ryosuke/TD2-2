#pragma once
#include <cstdint>

namespace App
{
	//ダメージで少しの間無敵になって点滅するプレイヤーのハート
	class PlayerHealth
	{
	public:
		//HPを最大にして無敵を消す
		void Reset();
		//無敵の時間を進める
		void Update(float deltaTime);

		//ダメージを受けたらtrueを返す
		bool Damage(int32_t amount);
		//最大まで回復する
		void Heal(int32_t amount);

		void StartInvincible(); //無敵にする
		void ClearInvincible(); //無敵を消す

		int32_t GetHearts() const { return hearts_; }
		bool IsDead() const { return hearts_ <= 0; }
		bool IsInvincible() const { return invincible_ > 0.0f; }
		//点滅中に見えているか
		bool IsVisible() const;

	private:
		int32_t hearts_ = 0;
		float invincible_ = 0.0f; //無敵の残り時間
	};
}
