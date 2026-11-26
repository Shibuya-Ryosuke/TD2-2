#pragma once
#include <RyoEngine.h>
#include <cstdint>

//ゲームシーンなどの背景の天球
namespace App::SkyConfig
{
	//天球
	inline constexpr const char* kSkyDomeModel = "Model/SkyDome/SkyDome.obj"; //内側から見る天球のモデル
	inline constexpr float kSkyDomeSpan = 1.45f; //画面の何倍の大きさの楕円にするかで、小さいほど縦にピンクから水色まで広く映る
	inline constexpr float kSkyDomeDepth = 60.0f; //カメラから天球の奥までの長さでカメラの遠い面より手前
	inline constexpr float kSkyScrollSpeed = 0.004f; //1秒に雲が横に流れる量で1で1周
	inline constexpr float kSkyTextureFill = 0.97f; //画像の上から下までのうち画面の上から下に入れる割合で1より大きいとはしが反対側に回り込む
}
