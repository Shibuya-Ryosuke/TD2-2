#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>

//タイトルとクリアとゲームオーバーで共通の、カメラと床とプレイヤーと積み木の数値
namespace App::ShowcaseConfig
{
	//3つの画面で共通
	inline constexpr float kCameraFovY = 0.6f; //遠近法のカメラの縦の画角のラジアン
	inline constexpr float kPlayerScale = 0.75f; //プレイヤーの大きさの倍率
	inline constexpr float kTileSize = 2.5f; //床のタイルの1辺
	inline constexpr int32_t kFloorTilesX = 5; //床のタイルの横の数
	inline constexpr int32_t kFloorTilesZ = 4; //床のタイルの奥の数
	inline constexpr RyoEngine::Vector4 kFloorColor = { 1.0f, 0.86f, 0.92f, 1.0f }; //床のうすいピンク
	inline constexpr std::array<RyoEngine::Vector4, 5> kBlockColors = { { //積み木のパステル
		{ 1.0f, 0.75f, 0.85f, 1.0f },
		{ 0.75f, 0.9f, 1.0f, 1.0f },
		{ 1.0f, 0.95f, 0.7f, 1.0f },
		{ 0.8f, 1.0f, 0.85f, 1.0f },
		{ 0.88f, 0.8f, 1.0f, 1.0f } } };

	//床が落ちる前にがたがた揺れる
	inline constexpr float kFloorRumbleRamp = 0.8f; //揺れがいちばん強くなるまでの秒数
	inline constexpr float kFloorRumbleHeight = 0.06f; //いちばん近い床が上下に揺れる高さ
	inline constexpr float kFloorRumbleTilt = 2.5f; //いちばん近い床が傾く度
	inline constexpr float kFloorRumbleSpeed = 38.0f; //がたがたの1秒のラジアン

	//床が消えて落ちる
	inline constexpr float kFloorFallStagger = 0.06f; //1マス離れるごとに床が落ち始めるのが遅れる秒数
	inline constexpr float kFloorFallPop = 0.12f; //落ちる前にぴょこっと浮く秒数
	inline constexpr float kFloorFallGravity = 22.0f; //床が落ちる重力
}
