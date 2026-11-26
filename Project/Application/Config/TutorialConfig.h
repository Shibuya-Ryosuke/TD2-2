#pragma once
#include <RyoEngine.h>
#include <cstdint>

//チュートリアルの数値と、csvの数字の意味
namespace App::TutorialConfig
{
	//チュートリアルの床のcsvで、1以上は全部床
	inline constexpr const char* kCsvPath = "Resources/ApplicationResources/Stage/tutorial.csv";
	inline constexpr int32_t kCodeStart = 2; //プレイヤーの最初の位置
	inline constexpr int32_t kCodeFirstBlock = 3; //1つ目の積み木が落ちてくるマス
	inline constexpr int32_t kCodeWall = 4; //切り離しの前に勝手に崩れるマス
	inline constexpr int32_t kCodeSecondBlock = 5; //2つ目の積み木が落ちてくる島のマス
	inline constexpr int32_t kCodeBridge = 6; //壊すと島が切り離されるマス
	inline constexpr int32_t kCodeIsland = 7; //2つ目の積み木が乗る島のマス

	//やること
	inline constexpr int32_t kMoveCount = 5; //動くマスの数
	inline constexpr int32_t kBreakCount = 3; //ハンマーで壊す数
	inline constexpr float kClearShowTime = 1.2f; //CLEAR!を出す秒数
	inline constexpr float kNextStepDelay = 0.8f; //CLEAR!のあと次のやることを出すまでの秒数
	inline constexpr float kRestoreJiggle = 1.2f; //練習で壊したマスが戻るときに揺れる強さ
	inline constexpr float kRepairAt = 0.6f; //やることを終えて飛ばされ始めてから、飛んでいる間に床を直すまでの秒数

	//壊すマスを照らすスポットライト
	inline constexpr const char* kSpotShader = "TutorialSpot";
	inline constexpr float kSpotHeight = 14.0f; //光の柱の高さ
	inline constexpr float kSpotWidthRate = 0.95f; //1マスに対する光の柱の太さで積み木より少し太い
	inline constexpr float kSpotFrameThickness = 0.06f; //マスの上に置く枠の厚み
	inline constexpr float kSpotFadeTime = 0.35f; //光がふわっと出たり消えたりする秒数
	inline constexpr float kRejectTextTime = 1.0f; //壊せないマスを叩いたときの文字を出す秒数

	//積み木
	inline constexpr float kBlockSizeRate = 0.8f; //1マスに対する積み木の大きさ
	inline constexpr RyoEngine::Vector4 kBlockColor = { 1.0f, 0.62f, 0.82f, 1.0f }; //パステルピンクの積み木
	inline constexpr float kBlockDropHeight = 26.0f; //積み木が落ち始める高さ
	inline constexpr float kBlockDropTime = 1.6f; //積み木が床に着くまでの秒数
	inline constexpr float kBlockLookUpPitch = -0.85f; //落ち始めにカメラを見上げさせるラジアン
	inline constexpr float kBlockZoomOrthoSize = 9.0f; //積み木を追うときに画面の縦の半分に映す長さ
	inline constexpr float kBlockCameraBackTime = 0.7f; //ドン！のあといつもの映し方へ戻る秒数
	inline constexpr float kBlockLandShake = 0.7f; //ドン！で画面が揺れる強さ
	inline constexpr float kBlockLandJiggle = 2.0f; //ドン！で足場が揺れる強さ
	inline constexpr float kBlockSinkTime = 1.0f; //床と一緒に落ちて消えるまでの秒数
	inline constexpr float kBlockSinkSpeed = 14.0f; //床がなくなってから落ちる速さ

	//足場がばねみたいにはねて最初の位置へ吹っ飛ばす
	inline constexpr float kLaunchWindupTime = 0.3f; //足場がぐっと沈んでためる秒数
	inline constexpr float kLaunchWindupSink = 0.5f; //ためるときに沈む深さ
	inline constexpr float kLaunchTime = 1.1f; //吹っ飛んでいる秒数
	inline constexpr float kLaunchHeight = 8.0f; //吹っ飛ぶ高さ
	inline constexpr float kLaunchSpinSpeed = 14.0f; //吹っ飛びながら回る1秒のラジアン
	inline constexpr float kLaunchJiggle = 3.0f; //ボヨン！で足場がはねる強さ
	inline constexpr float kLaunchLandJiggle = 1.6f; //尻もちで足場が揺れる強さ
	inline constexpr float kLaunchHearts = 1.2f; //尻もちで着地したときに出るハートの強さ
	inline constexpr float kLaunchShake = 0.3f; //ボヨン！と尻もちで画面が揺れる強さ
	inline constexpr float kLaunchSitTime = 0.45f; //戻ってから尻もちをついている秒数
	inline constexpr float kRescueDropTime = 0.7f; //穴に落ちてから下ではね返されるまでの秒数
	inline constexpr float kRescueDropDepth = 9.0f; //穴に落ちて下ではね返されるまでに落ちる深さ
	inline constexpr float kReshapeDelay = 0.6f; //戻ってから床が崩れ始めるまでの秒数
	inline constexpr float kWallCollapseDelay = 1.0f; //切り離しの前に崩れるマスが穴になるまでの秒数

	//下の層へ落ちて残った足場が寝ているボスへ当たるまで
	inline constexpr float kDescentTime = 1.4f; //下の層へ落ちる秒数
	inline constexpr float kLandSitTime = 0.9f; //尻もちのあと足場が飛び始めるまでの秒数
	inline constexpr float kBarrageLookTime = 0.6f; //カメラが寝ているボスを向くまでの秒数
	inline constexpr float kBarrageOrthoSize = 9.0f; //足場が当たるのを見せるときに画面の縦の半分に映す長さ
	inline constexpr float kBarrageFocusHeight = 1.0f; //ボスの足元から映すまん中までの高さ
	inline constexpr float kBarrageTargetHeight = 1.0f; //足場が飛んでいくボスの足元からの高さ
	inline constexpr float kBarrageMinTime = 0.5f; //最後の足場が当たってから戻るまでの余韻の秒数
	inline constexpr float kBarrageHitShake = 0.18f; //1枚当たるごとに画面が揺れる強さ
	inline constexpr float kBarrageHitJiggle = 0.8f; //1枚当たるごとにボスの下の足場が揺れる強さ
	inline constexpr float kFinaleZoomBackTime = 0.7f; //いつもの映し方へ戻る秒数

	//ゲームシーンでボスが起きる
	inline constexpr float kWakeDozeTime = 0.5f; //ゲームシーンに入ってからまだ寝ている秒数
	inline constexpr float kWakeTime = 1.0f; //飛び起きて着地するまでの秒数
	inline constexpr float kWakeHopHeight = 2.0f; //飛び起きるときに跳ねる高さ
	inline constexpr float kWakeShake = 0.6f; //起きて着地したときに画面が揺れる強さ
}
