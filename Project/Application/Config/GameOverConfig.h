#pragma once
#include <RyoEngine.h>
#include <cstdint>

//ゲームオーバーでプレイヤーがきょろきょろして積み木のシャワーに埋もれる演出の数値
namespace App::GameOverConfig
{
	inline constexpr RyoEngine::Vector3 kOverEye = { 0.0f, 4.2f, -11.0f }; //カメラの位置
	inline constexpr RyoEngine::Vector3 kOverTarget = { 0.0f, 1.6f, 0.0f }; //カメラが見る所
	inline constexpr RyoEngine::Vector3 kOverPlayer = { 0.0f, 0.0f, 0.0f }; //プレイヤーの足元
	inline constexpr float kLookTime = 2.4f; //きょろきょろする秒数
	inline constexpr float kLookSpeed = 4.0f; //きょろきょろの速さ
	inline constexpr int32_t kShowerCount = 360; //降ってくる積み木の数
	inline constexpr float kShowerTime = 2.4f; //全部降らせ終わるまでの秒数
	inline constexpr float kShowerSpread = 1.0f; //降ってくる積み木の広がりで、正規分布の幅
	inline constexpr float kShowerHeight = 14.0f; //降らせ始める高さ
	inline constexpr float kShowerSpeed = 16.0f; //降ってくる最初の速さ
	inline constexpr float kShowerGravity = 30.0f; //降ってくる積み木の重力
	inline constexpr float kShowerMinSize = 0.42f; //降ってくる積み木のいちばん小さい1辺
	inline constexpr float kShowerMaxSize = 0.7f; //降ってくる積み木のいちばん大きい1辺
	inline constexpr float kPileCell = 0.4f; //積もる高さを覚える升目の1辺
	inline constexpr int32_t kPileCells = 15; //積もる升目の1辺の数
	inline constexpr float kPileStack = 0.6f; //1個積もるとその升目が高くなる積み木の1辺に対する割合
	inline constexpr float kPileSpill = 0.15f; //1個積もるととなりの升目が高くなる積み木の1辺に対する割合
	inline constexpr int32_t kPileRollLimit = 4; //高すぎるときに転がすいちばん多い升目の数
	inline constexpr float kMenuDelay = 0.6f; //埋もれきってからメニューを出すまでの秒数
}
