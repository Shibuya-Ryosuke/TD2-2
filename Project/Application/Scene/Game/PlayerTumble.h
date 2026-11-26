#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;
	class Floor;
	class GameDirection;

	//足場がなくなったプレイヤーが足をばたばたさせながら下の層へ落ちて、どすんと尻もちをつく
	class PlayerTumble
	{
	public:
		//今の位置から下の層のlandingへ落ち始める
		void Begin(const RyoEngine::Vector3& from, const RyoEngine::Vector3& landing, Player& player);
		//1フレーム進めて、尻もちから立ち上がったらtrue
		bool Update(float deltaTime, Player& player, Floor& landingFloor, GameDirection& direction);

		bool IsActive() const { return stage_ != Stage::Idle; }
		//落ちる線を出す間か
		bool IsFalling() const { return stage_ == Stage::Fall; }

	private:
		//落ちる段階
		enum class Stage
		{
			Idle, //落ちていない
			Fall, //ばたばた落ちている
			Sit, //尻もちをついている
		};

		RyoEngine::Vector3 from_{};
		RyoEngine::Vector3 landing_{};
		float timer_ = 0.0f; //今の段階に入ってからの秒数
		Stage stage_ = Stage::Idle;
	};
}
