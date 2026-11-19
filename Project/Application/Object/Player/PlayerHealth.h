#pragma once
#include <cstdint>

namespace App
{
	// プレイヤーの HP (ハート) と無敵時間 (Game から分けたもの)
	// ダメージを受けると少しの間無敵になって、その間は点滅する
	class PlayerHealth
	{
	public:
		// HP を最大にして、無敵を消す
		void Reset();
		// 無敵の時間を進める (毎フレーム)
		void Update(float deltaTime);

		// ダメージ (無敵中・0 以下は受けない)。受けたら true
		bool Damage(int32_t amount);
		// 回復 (最大まで)
		void Heal(int32_t amount);

		void StartInvincible();  // 無敵にする (kInvincibilityTime 秒)
		void ClearInvincible();  // 無敵を消す

		int32_t GetHearts() const { return hearts_; }
		bool IsDead() const { return hearts_ <= 0; }
		bool IsInvincible() const { return invincible_ > 0.0f; }
		// 点滅中に見えているか (無敵でなければいつも true)
		bool IsVisible() const;

	private:
		int32_t hearts_ = 0;
		float invincible_ = 0.0f; // 無敵の残り時間
	};
}
