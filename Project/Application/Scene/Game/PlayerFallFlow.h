#pragma once
#include <RyoEngine.h>

namespace App
{
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;

	//ボス戦で穴に落ちたらプレイヤーに寄って、じたばた落ちて尻もちをつき、ボスも降りてきて下の層で再開する流れ
	class PlayerFallFlow
	{
	public:
		//落ちた所の真下の床と、ボスが降りる反対側の床を決めて始める
		void Begin(LayerFloors& floors, Player& player, Boss& boss);

		//1フレーム進めて、再開できるようになったらtrue
		bool Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);

		//落ちる線を出す間か
		bool IsFalling() const { return stage_ == Stage::Fall; }

	private:
		//流れの段階
		enum class Stage
		{
			Idle, //落ちていない
			Fall, //寄りながらじたばた落ちる
			Sit, //尻もちをついている
			BossDrop, //ボスが上から降りてきて、カメラが戻る
		};

		void UpdateFall(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction);
		void UpdateBossDrop(LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);

		Stage stage_ = Stage::Idle;
		float timer_ = 0.0f; //今の段階に入ってからの秒数
		RyoEngine::Vector3 fallStart_{}; //落ち始めた位置
		RyoEngine::Vector3 landing_{}; //下の層で尻もちをつく位置
		RyoEngine::Vector3 bossLanding_{}; //ボスが降りてくる位置
		float startFloorHeight_ = 0.0f; //落ちる前の床の高さ
		float landingFloorHeight_ = 0.0f; //下の層の床の高さ
		bool isBossLanded_ = false;
	};
}
