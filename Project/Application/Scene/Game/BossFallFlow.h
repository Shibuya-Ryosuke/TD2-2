#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "DescentSequence.h"
#include "GamePhase.h"
#include "PlayerTumble.h"
#include "TileBarrage.h"

namespace App
{
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;

	//揺れてボスが落ち、上の層の足場がボスへ飛んで、足場がなくなったプレイヤーも落ちて尻もちをつくまでの流れ
	class BossFallFlow
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //落下と足場のボスへのダメージ
			bool isPlayerCaught = false; //揺れ終わったときプレイヤーが落ちる所にいた
			bool isDropStarted = false; //ボスが落ち始めた
			bool isArrived = false; //ボスが下の層に着いて層を進めた
			bool isFinished = false; //足場を飛ばし終わり、プレイヤーも下の層で立ち上がってカメラが戻った
		};

		//ボスの島が切り離されたら揺れ始める
		void BeginEscape();
		//ボスが下の層へ落ち始める
		void BeginDrop(LayerFloors& floors, Player& player, const Boss& boss);
		//下の層でプレイヤーが尻もちをつく位置を決める
		void SetPlayerLanding(const RyoEngine::Vector3& landing) { playerLanding_ = landing; }

		//1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);
		//プレイヤーがばたばた落ちている間か
		bool IsPlayerFalling() const { return tumble_.IsFalling(); }

		//落下中と着地の待ちと足場を飛ばす間はプレイヤーを動かせない
		static bool IsLockedPhase(GamePhase phase);

		//今の場面
		GamePhase GetPhase() const { return phase_; }
		float GetEscapeTimer() const { return timer_; }
		const TileBarrage& GetBarrage() const { return barrage_; }

	private:
		void UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, Result& result); //揺れてボスごと落とす
		void UpdateDescent(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, Result& result); //ボスを下の層へ
		void UpdateBarrage(float deltaTime, LayerFloors& floors, Player& player, const Boss& boss, GameDirection& direction, Result& result); //足場を飛ばしてプレイヤーも落とす
		void UpdateCameraReturn(float deltaTime, LayerFloors& floors, StageCamera& camera, Result& result); //カメラを下の層のいつもの映し方へ戻す
		void BeginBarrage(LayerFloors& floors); //足場を飛ばし始める
		bool IsFootingGone(const Floor& upper, const Player& player) const; //プレイヤーの立っている足場が飛び立ったか

		GamePhase phase_ = GamePhase::Escape;
		float timer_ = 0.0f; //揺れの残り時間か着地してからの時間
		DescentSequence descent_; //ボスが下の層へ落ちる動き
		TileBarrage barrage_; //足場を飛ばす動き
		PlayerTumble tumble_; //足場がなくなったプレイヤーが落ちて尻もちをつく
		RyoEngine::Vector3 playerLanding_{}; //下の層でプレイヤーが尻もちをつく位置
		bool isPlayerDropped_ = false; //プレイヤーが落ち始めたか
		bool isUpperReleased_ = false; //上の層を消したか
		bool isCameraReturning_ = false; //カメラを戻しているか
		float returnTimer_ = 0.0f; //カメラを戻し始めてからの秒数
		float returnFromHeight_ = 0.0f; //戻し始めたときのカメラの床の高さ
		float returnFromSize_ = 0.0f; //戻し始めたときの映す広さ
	};
}
