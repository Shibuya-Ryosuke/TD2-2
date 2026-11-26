#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;
	class Floor;
	class GameDirection;

	//足場がばねみたいにぷるんとなるか、穴の下までしっかり落ちてから、じたばたくるくる吹っ飛ばされて決めた所へ尻もちで戻る
	class PlayerLaunch
	{
	public:
		//今の位置からtargetへ飛ばす準備を始める
		void Begin(const RyoEngine::Vector3& from, const RyoEngine::Vector3& target);
		//穴の下までじたばた落ちてから、下からtargetへ吹っ飛ばす
		void BeginFromHole(const RyoEngine::Vector3& from, const RyoEngine::Vector3& target);
		//1フレーム進めて、尻もちから立ち上がったらtrue
		bool Update(float deltaTime, Player& player, Floor& floor, GameDirection& direction);
		bool IsActive() const { return stage_ != Stage::Idle; }

	private:
		//飛ばされる段階
		enum class Stage
		{
			Idle, //飛ばしていない
			Drop, //穴の下までじたばた落ちていく
			Windup, //足場がぐっと沈んでためる
			Fly, //じたばたくるくる飛んでいく
			Land, //尻もちをついている
		};

		RyoEngine::Vector3 from_{};
		RyoEngine::Vector3 target_{};
		float timer_ = 0.0f; //今の段階に入ってからの秒数
		Stage stage_ = Stage::Idle;
	};
}
