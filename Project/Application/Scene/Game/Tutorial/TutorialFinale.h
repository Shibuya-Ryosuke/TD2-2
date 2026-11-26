#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "../TileBarrage.h"

namespace App
{
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;

	//チュートリアルの床から下の層へ落ちて尻もちをつき、残った足場が寝ているボスへ飛んでボスが起きるまで
	class TutorialFinale
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //足場が届いたボスへのダメージ
			bool isFinished = false; //ボスが起きてボス戦を始められる
		};

		//ボスの反対側の床へ落ち始める
		void Begin(LayerFloors& floors, Player& player, const Boss& boss);
		//1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);

		//落ちる線を出す間か
		bool IsFalling() const { return stage_ == Stage::Fall; }
		//ボスの上に出す文字で、なければnullptr
		const char* GetBossText() const;

	private:
		//流れの段階
		enum class Stage
		{
			Fall, //じたばた落ちる
			Sit, //尻もちをついている
			Barrage, //残った足場が寝ているボスへ飛ぶ
			Wake, //ボスが起きる
			Done, //終わった
		};

		Stage stage_ = Stage::Fall;
		float timer_ = 0.0f; //今の段階に入ってからの秒数
		RyoEngine::Vector3 fallStart_{}; //落ち始めた位置
		RyoEngine::Vector3 landing_{}; //尻もちをつく位置
		float startFloorHeight_ = 0.0f;
		float landingFloorHeight_ = 0.0f;
		RyoEngine::Vector3 bossRest_{}; //寝ているボスの足元
		TileBarrage barrage_; //残った足場をボスへ飛ばす
	};
}
