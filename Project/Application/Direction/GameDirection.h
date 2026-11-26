#pragma once
#include <cstdint>
#include "Screen/DamageFlash.h"
#include "Screen/FallLines.h"
#include "Screen/ScreenShake.h"

namespace App
{
	class PlayerHealth;
	class Player;
	class Boss;
	class Floor;

	//ゲームシーンの画面の演出と揺れと着地のぷるるんをまとめる
	class GameDirection
	{
	public:
		GameDirection() = default;
		~GameDirection() = default;

		GameDirection(const GameDirection&) = delete;
		GameDirection& operator=(const GameDirection&) = delete;

		void Initialize(); //ポストエフェクトを用意する
		//HPと落ちているかと着地を見て演出を進める
		void Update(const PlayerHealth& health, bool isFalling, const Player& player, const Boss& boss, Floor& floor);

		//画面を揺らす
		void Shake(float power) { shake_.Play(power); }
		//このフレームにカメラをずらす分
		const RyoEngine::Vector3& GetShakeOffset() const { return shakeOffset_; }

	private:
		DamageFlash damage_;
		FallLines fallLines_;
		ScreenShake shake_;
		RyoEngine::Vector3 shakeOffset_{}; //このフレームの揺れ
		int32_t lastHearts_ = -1; //前のフレームのHP
		bool wasBossJumping_ = false; //前のフレームでボスが跳んでいたか
	};
}
