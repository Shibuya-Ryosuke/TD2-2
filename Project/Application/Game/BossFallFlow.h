#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "DescentSequence.h"
#include "GamePhase.h"
#include "TileBarrage.h"

namespace App
{
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;

	// ボスを下の層へ落とす流れ (Game から分けたもの)
	// Escape (切り離された床が揺れる) → Descent (ボスが落ちる・プレイヤーも降りる)
	// → LandingImpact (少し待つ) → TileBarrage (上の層の足場をボスへ飛ばす)
	// Game は Update の結果を見て、ダメージ・やり直し・層を進めたあとの準備・終わり をする
	class BossFallFlow
	{
	public:
		// このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0;      // ボスへのダメージ (落下・足場)
			bool isPlayerCaught = false; // 揺れ終わったとき、プレイヤーが切り離された床・穴の上にいた (落ちる)
			bool isDropStarted = false;  // ボスが落ち始めた
			bool isArrived = false;      // 下の層に着いた (層はもう進めてある)
			bool isFinished = false;     // 足場を飛ばし終わった
		};

		// ボスの島が切り離された: 揺れ始める
		void BeginEscape();
		// ボスが下の層へ落ち始める (下の層がなければ作る)
		void BeginDrop(LayerFloors& floors, const Player& player, const Boss& boss);

		// 1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera);

		// プレイヤーを動かせない場面か (落下・着地の待ち・足場を飛ばす間。揺れている間は動ける)
		static bool IsLockedPhase(GamePhase phase);

		// 今どの場面か (Escape / Descent / LandingImpact / TileBarrage)
		GamePhase GetPhase() const { return phase_; }
		float GetEscapeTimer() const { return timer_; }
		const TileBarrage& GetBarrage() const { return barrage_; }

	private:
		void UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, Result& result); // 揺れる → ボスごと落とす
		void UpdateDescent(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, Result& result); // 下の層へ
		void UpdateBarrage(float deltaTime, LayerFloors& floors, const Boss& boss, Result& result); // 足場を飛ばす
		void BeginBarrage(LayerFloors& floors, Result& result); // 足場を飛ばし始める

		GamePhase phase_ = GamePhase::Escape;
		float timer_ = 0.0f;        // 揺れの残り時間・着地してからの時間
		DescentSequence descent_;   // 下の層へ降りる動き
		TileBarrage barrage_;       // 足場を飛ばす動き
	};
}
