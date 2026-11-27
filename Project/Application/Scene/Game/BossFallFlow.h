#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "BossFallCamera.h"
#include "DescentSequence.h"
#include "GamePhase.h"
#include "PlayerDive.h"
#include "PlayerEdgeHop.h"
#include "TileBarrage.h"
#include "../../Direction/BossFall/BossFallShow.h"
#include "../../Direction/BossFall/TileLift.h"
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;

	//ボスが落ちて叩きつけられ、プレイヤーが穴へ飛びこんでハンマーでドン、上の足場がボスへ降って起き上がるまでの流れ
	class BossFallFlow
	{
	public:
		BossFallFlow() = default;
		~BossFallFlow() = default;

		//モデルを持つパーツのボスを持つのでコピーとムーブを禁止する
		BossFallFlow(const BossFallFlow&) = delete;
		BossFallFlow& operator=(const BossFallFlow&) = delete;
		BossFallFlow(BossFallFlow&&) = delete;
		BossFallFlow& operator=(BossFallFlow&&) = delete;

		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //落下と足場のボスへのダメージ
			bool isPlayerCaught = false; //揺れ終わったときプレイヤーが落ちる所にいた
			bool isDropStarted = false; //ボスが落ち始めた
			bool isArrived = false; //ボスが下の層に着いて層を進めた
			bool isFinished = false; //ボスが起き上がってカメラが戻った
		};

		//パーツのボスを用意する
		void Initialize() { show_.Initialize(); }
		//ボスの島が切り離されたら揺れ始める
		void BeginEscape();
		//ボスが下の層へ落ち始める
		void BeginDrop(LayerFloors& floors, Player& player, Boss& boss, const StageCamera& camera);
		//下の層でプレイヤーが着地する位置を決める
		void SetPlayerLanding(const RyoEngine::Vector3& landing) { playerLanding_ = landing; }

		//1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);
		//プレイヤーが穴へ飛びこんで落ちている間か
		bool IsPlayerFalling() const { return dive_.IsActive(); }
		//パーツのボスを描く
		void Draw() { show_.Draw(); }

		//落下中と着地の待ちと足場を飛ばす間はプレイヤーを動かせない
		static bool IsLockedPhase(GamePhase phase);

		//今の場面
		GamePhase GetPhase() const { return phase_; }
		float GetEscapeTimer() const { return timer_; }
		const TileBarrage& GetBarrage() const { return barrage_; }

	private:
		//ボスが落ちてからの細かい段階
		enum class Stage
		{
			ZoomIn, //カメラがボスへ寄って今の層のタイルが透ける
			Drop, //ボスがじたばた落ちる
			Slam, //ドスンと座りこむのを寄ったまま見せる
			Widen, //両方の層を映す所まで引いてタイルが戻る
			Hop, //プレイヤーが穴のふちまでぴょんぴょん跳ぶ
			Dive, //えいっと飛びこんで、寄ったカメラのまま下の層へ落ちる
			Hammer, //着地して地面をハンマーでドン
			Lift, //カメラが引いて上の足場がふわっと浮く
			Barrage, //足場がいっせいにボスへ降る
			GetUp, //ボスが起き上がってカメラが戻る
		};

		static float ToYawDeg(const RyoEngine::Vector3& direction); //水平な向きをパーツのボスの向きの度数にする
		static bool FindDiveEdge(const Floor& floor, const RyoEngine::Vector3& from, RyoEngine::Vector3& edge, RyoEngine::Vector3& hole); //いちばん近い穴のふちのマスと、その穴のマスのまん中
		Floor* GetTopFloor(LayerFloors& floors) const; //ボスが落ちる前に立っていた層
		void SetTopFade(LayerFloors& floors, float alpha) const; //ボスが落ちる前に立っていた層のタイルを透かす
		void SetStage(Stage stage); //段階を変えて、その段階の場面にする
		void UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, const StageCamera& camera, Result& result); //揺れてボスごと落とす
		void UpdateZoomIn(LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera); //ボスへ寄って足元が消える
		void UpdateDrop(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, GameDirection& direction, Result& result); //じたばた落ちてドスン
		void UpdateSlam(StageCamera& camera); //座りこむのを見せる
		void UpdateWiden(LayerFloors& floors, Player& player, StageCamera& camera); //引いてタイルを戻す
		void UpdateHop(float deltaTime, Player& player, StageCamera& camera); //穴のふちまで跳ぶ
		void UpdateDive(float deltaTime, LayerFloors& floors, Player& player, const Boss& boss, StageCamera& camera, GameDirection& direction); //飛びこんで落ちる
		void UpdateHammer(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction); //ハンマーでドン
		void UpdateLift(float deltaTime, LayerFloors& floors, StageCamera& camera); //引いて上の足場を浮かせる
		void UpdateBarrage(float deltaTime, LayerFloors& floors, StageCamera& camera, GameDirection& direction, Result& result); //足場が当たるたびにむぎゅ
		void UpdateGetUp(LayerFloors& floors, Boss& boss, StageCamera& camera, Result& result); //起き上がってカメラを戻す

		GamePhase phase_ = GamePhase::Escape;
		Stage stage_ = Stage::ZoomIn;
		float timer_ = 0.0f; //揺れの残り時間
		float stageTimer_ = 0.0f; //今の段階に入ってからの秒数
		DescentSequence descent_; //ボスが下の層へ落ちる動き
		TileBarrage barrage_; //足場を飛ばす動き
		TileLift lift_; //ドンのあと上の足場が浮く見た目
		PlayerEdgeHop hop_; //プレイヤーが穴のふちまで跳ぶ動き
		PlayerDive dive_; //プレイヤーが穴へ飛びこんで落ちる動き
		BossFallShow show_; //パーツに分けたボスの動き
		BossFallCamera camera_; //映し方のつなぎ
		RyoEngine::Vector3 bossLanding_{}; //ボスが着く下の層の位置
		RyoEngine::Vector3 playerLanding_{}; //下の層でプレイヤーが着地する位置
		RyoEngine::Vector3 settleFrom_{}; //ボスが落ち始めたときのプレイヤーの位置
		RyoEngine::Vector3 settleTo_{}; //プレイヤーが立って待つ上の層の足場の位置
		RyoEngine::Vector3 diveHole_{}; //プレイヤーが飛びこむ穴の上
		float upperHeight_ = 0.0f; //ボスが落ちる前の層の高さ
		float lowerHeight_ = 0.0f; //ボスが落ちた先の層の高さ
		bool isAdvanced_ = false; //層を1つ進めたか
		bool isSettled_ = false; //プレイヤーが足場に立ちなおしたか
		bool isGetUpDecided_ = false; //起き上がるか倒れたままか決めたか
		bool isBossStaysDown_ = false; //倒しきったので起き上がらない
		bool isBossRevealed_ = false; //いつものボスに戻したか
	};
}
