#include "PlayerHealth.h"
#include "../../Config/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace App
{
	// HP を最大にして、無敵を消す
	void PlayerHealth::Reset()
	{
		hearts_ = GameConfig::kMaxHearts;
		invincible_ = 0.0f;
	}

	// 無敵の時間を進める
	void PlayerHealth::Update(float deltaTime)
	{
		invincible_ = std::max(0.0f, invincible_ - deltaTime);
	}

	// ダメージ
	bool PlayerHealth::Damage(int32_t amount)
	{
		if (amount <= 0 || IsInvincible())
		{
			return false;
		}
		// ハートを減らして、少しの間無敵 (0 になったら無敵にしない)
		hearts_ = std::max(0, hearts_ - amount);
		if (!IsDead())
		{
			StartInvincible();
		}
		return true;
	}

	// 回復 (最大まで)
	void PlayerHealth::Heal(int32_t amount)
	{
		hearts_ = std::min(GameConfig::kMaxHearts, hearts_ + amount);
	}

	void PlayerHealth::StartInvincible()
	{
		invincible_ = GameConfig::kInvincibilityTime;
	}

	void PlayerHealth::ClearInvincible()
	{
		invincible_ = 0.0f;
	}

	// 点滅中に見えているか
	bool PlayerHealth::IsVisible() const
	{
		if (!IsInvincible())
		{
			return true;
		}
		// sin の + と - で 見える ⇔ 見えない を切り替える
		const float wave = std::sin(invincible_ * GameConfig::kHitBlinkFrequency * std::numbers::pi_v<float> * 2.0f);
		return wave >= 0.0f;
	}
}
