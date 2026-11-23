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

	//揺れてボスが落ち、足場を飛ばすまでのボスを下の層へ落とす流れ
	class BossFallFlow
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //落下と足場のボスへのダメージ
			bool isPlayerCaught = false; //揺れ終わったときプレイヤーが落ちる所にいた
			bool isDropStarted = false; //ボスが落ち始めた
			bool isArrived = false; //下の層に着いた
			bool isFinished = false; //足場を飛ばし終わった
		};

		//ボスの島が切り離されたら揺れ始める
		void BeginEscape();
		//ボスが下の層へ落ち始める
		void BeginDrop(LayerFloors& floors, const Player& player, const Boss& boss);

		//1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera);

		//落下中と着地の待ちと足場を飛ばす間はプレイヤーを動かせない
		static bool IsLockedPhase(GamePhase phase);

		//今の場面
		GamePhase GetPhase() const { return phase_; }
		float GetEscapeTimer() const { return timer_; }
		const TileBarrage& GetBarrage() const { return barrage_; }

	private:
		void UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, Result& result); //揺れてボスごと落とす
		void UpdateDescent(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, Result& result); //下の層へ
		void UpdateBarrage(float deltaTime, LayerFloors& floors, const Boss& boss, Result& result); //足場を飛ばす
		void BeginBarrage(LayerFloors& floors, Result& result); //足場を飛ばし始める

		GamePhase phase_ = GamePhase::Escape;
		float timer_ = 0.0f; //揺れの残り時間か着地してからの時間
		DescentSequence descent_; //下の層へ降りる動き
		TileBarrage barrage_; //足場を飛ばす動き
	};
}
