#pragma once
#include <RyoEngine.h>
#include <cstdint>

//床の大きさと崩れ方と色
namespace App::FloorConfig
{
	//床と長さ
	inline constexpr float kWorldScale = 1.25f; //タイルを大きくするために床とボスの届く距離などの長さにかける倍率
	inline constexpr float kFloorSize = 20.0f * kWorldScale; //床全体の1辺の長さ
	inline constexpr float kTileThickness = 0.45f; //タイルの厚さ
	inline constexpr float kTileGap = 0.055f; //1マスに対するタイルのすき間の割合

	//床を崩す
	inline constexpr float kCollapseDelay = 1.5f; //崩してから穴になるまでの秒数
	inline constexpr float kShapePreviewBlend = 0.6f; //床に出す正面1マスの予告の色の濃さ
	inline constexpr RyoEngine::Vector4 kBreakPreviewColor = { 0.2f, 0.9f, 0.95f, 1.0f }; //正面1マスの予告の水色
	inline constexpr float kPreviewGlowMin = 0.8f; //予告のマスの光のいちばん弱いとき
	inline constexpr float kPreviewGlowMax = 1.8f; //予告のマスの光のいちばん強いとき
	inline constexpr float kPreviewGlowSpeed = 1.5f; //予告のマスが1秒に明滅する回数

	//切り離し
	inline constexpr float kEscapeDelay = 2.0f; //切り離されてから落ちるまでの秒数
	inline constexpr float kShakeDistance = 0.12f; //切り離された床の揺れ幅
	inline constexpr float kShakeFrequency = 13.0f; //切り離された床が1秒に揺れる回数

	//仮のモデルにかける色
	inline constexpr RyoEngine::Vector4 kFloorColor = { 0.329f, 0.435f, 0.557f, 1.0f }; //ふつうの床
	inline constexpr RyoEngine::Vector4 kIsolatedColor = { 0.925f, 0.349f, 0.459f, 1.0f }; //切り離された床
	inline constexpr RyoEngine::Vector4 kBonusColor = { 0.05f, 0.74f, 0.91f, 1.0f }; //ヒビが入って飛ばすと1ダメージの穴のとなりの床
}
