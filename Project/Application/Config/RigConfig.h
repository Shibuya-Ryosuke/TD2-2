#pragma once
#include <RyoEngine.h>

//頭と体と両腕と両足に分けたプレイヤーのモデルをコードで動かすときの数値
namespace App::RigConfig
{
	//分けたパーツのモデル
	inline constexpr const char* kHeadModel = "Model/player/Head/playerHead.obj";
	inline constexpr const char* kBodyModel = "Model/player/Body/player.Body.obj";
	inline constexpr const char* kLeftArmModel = "Model/player/LeftArm/playerLeftArm.obj";
	inline constexpr const char* kRightArmModel = "Model/player/RightArm/playerRightArm.obj";
	inline constexpr const char* kLeftLegModel = "Model/player/LeftFeet/playerLeftFeet.obj";
	inline constexpr const char* kRightLegModel = "Model/player/RightFeet/playerRightFeet.obj";

	//関節の位置で、読みこむときにXが反転したあとのモデルの中の位置
	inline constexpr RyoEngine::Vector3 kWaistPivot = { -0.09f, 1.20f, -0.05f }; //腰で、上半身はここを中心に傾く
	inline constexpr RyoEngine::Vector3 kNeckPivot = { -0.10f, 2.40f, -0.03f }; //首
	inline constexpr RyoEngine::Vector3 kLeftShoulderPivot = { -0.08f, 2.22f, -0.43f }; //左の肩
	inline constexpr RyoEngine::Vector3 kRightShoulderPivot = { -0.05f, 2.22f, 0.40f }; //右の肩
	inline constexpr RyoEngine::Vector3 kLeftHipPivot = { -0.02f, 1.22f, -0.34f }; //左の足のつけ根
	inline constexpr RyoEngine::Vector3 kRightHipPivot = { -0.02f, 1.22f, 0.24f }; //右の足のつけ根
	inline constexpr RyoEngine::Vector3 kLeftHand = { -0.12f, 1.37f, -1.19f }; //左手の先
	inline constexpr RyoEngine::Vector3 kRightHand = { 0.03f, 1.26f, 1.11f }; //右手の先
}
