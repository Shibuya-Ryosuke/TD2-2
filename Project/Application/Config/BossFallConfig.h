#pragma once
#include <RyoEngine.h>
#include <cstdint>

//ボスが落ちて叩きつけられ、残った足場がためてプレイヤーを落とし、ハンマーの合図で足場がボスへ降って起き上がるまでの数値
namespace App::BossFallConfig
{
	//流れの秒数で、ボスが落ちてから起き上がるまで全部で約5.9秒
	inline constexpr float kZoomInTime = 0.4f; //カメラがボスへ寄って、今の層のタイルが透けきるまでの秒数
	inline constexpr float kDropTime = 1.0f; //ボスがじたばたしながら下の層まで落ちる秒数
	inline constexpr float kSlamHoldTime = 0.5f; //ドスンのあと、寄ったまま座りこむのを見せる秒数
	inline constexpr float kWideReturnTime = 0.4f; //カメラが両方の層を映す所まで引いて、タイルが元に戻る秒数
	inline constexpr float kSettleTime = 0.2f; //跳んでいる途中だったプレイヤーが近くの足場へ着地しなおす秒数
	inline constexpr float kSettleHop = 0.3f; //着地しなおすときの山の高さ
	inline constexpr float kHopTotalTime = 0.5f; //穴のふちまでぴょんぴょん跳ぶ秒数で、遠くても近くても同じ
	inline constexpr float kHopHeight = 0.9f; //ぴょんの山の高さ
	inline constexpr int32_t kHopMaxCount = 6; //穴のふちまでのいちばん多い跳ぶ回数
	inline constexpr float kCrouchTime = 0.15f; //穴のふちでぐっとためる秒数
	inline constexpr float kCrouchSink = 0.15f; //ためるときに沈む深さ
	inline constexpr float kDiveHopTime = 0.2f; //えいっと穴の上まで飛ぶ秒数で、ここでカメラがプレイヤーへ寄る
	inline constexpr float kDiveHopHeight = 1.2f; //えいっと飛ぶ山の高さ
	inline constexpr float kDiveFallTime = 0.7f; //穴の上から下の層までじたばた落ちる秒数
	inline constexpr float kHammerTimeout = 1.0f; //ハンマーが当たった合図が来なくても先へ進む秒数
	inline constexpr float kLiftTime = 0.3f; //ドンのあとカメラが引いて、上の足場がふわっと浮く秒数
	inline constexpr float kBarrageWindow = 0.15f; //穴のとなりの足場とふつうの足場が、それぞれ全部飛び立ち終わるまでの秒数
	inline constexpr float kBarrageFlight = 0.35f; //足場が1枚ボスまで飛ぶ秒数
	inline constexpr float kBarrageGroupPause = 0.0f; //穴のとなりの足場が飛び立ち終わってからふつうの足場が飛び立つまでの秒数
	inline constexpr float kGetUpTime = 0.5f; //ボスが座りこみから立ち上がる秒数
	inline constexpr float kReturnTime = 0.5f; //カメラが下の層のいつもの映し方へ戻る秒数で、起き上がりと同時に始める

	//カメラ
	inline constexpr float kBossZoomOrthoSize = 6.5f; //ボスへ寄ったときに画面の縦の半分に映す長さ
	inline constexpr float kBossFocusHeight = 3.0f; //ボスへ寄るときに画面のまん中に来る足元からの高さ
	inline constexpr float kWideHeight = 0.5f; //上の層と下の層の両方を映すときに、上の層から下の層までのどこを映すかで0.5はまん中
	inline constexpr float kWideZoom = 1.15f; //両方の層を映すときにいつもより引く倍率
	inline constexpr float kDiveZoomOrthoSize = 4.5f; //落ちるプレイヤーについていくときに画面の縦の半分に映す長さ
	inline constexpr float kDiveFocusHeight = 1.2f; //プレイヤーへ寄るときに画面のまん中に来る足元からの高さ
	inline constexpr float kTileFadeAlpha = 0.0f; //落ちるのを見せる間の今の層のタイルの濃さで、0で消える

	//ボスのパーツ
	inline constexpr float kRigScale = 1.5f; //高さ4のパーツを高さ6のボスに合わせる倍率
	inline constexpr float kFaceCameraOffsetDeg = 25.0f; //座りこむときにカメラの方から少しななめを向ける度数
	inline constexpr float kSitSink = 0.85f; //座りこむときに体を下げる高さで、倍率をかける前の長さ
	inline constexpr float kFlailSpeed = 20.0f; //落ちながら手足をじたばたさせる速さ
	inline constexpr float kHurtSwaySpeed = 3.0f; //痛そうに頭をゆらす速さ
	inline constexpr float kPoseBlendTime = 0.12f; //じたばたし始めるときと、ドスンで手足が投げ出されるまでの秒数
	inline constexpr float kHurtSettleTime = 0.24f; //手足が投げ出されてから頭を押さえて座りこむまでの秒数
	//関節の位置で、読みこむときにXが反転したあとのパーツのモデルの中の位置
	inline constexpr RyoEngine::Vector3 kPelvisPivot = { 0.0f, 1.14f, 0.0f }; //胴体の下で、上半身はここを中心に傾く
	inline constexpr RyoEngine::Vector3 kNeckPivot = { 0.0f, 2.46f, 0.0f }; //首
	inline constexpr RyoEngine::Vector3 kLeftShoulderPivot = { -0.72f, 2.25f, 0.0f }; //左の肩
	inline constexpr RyoEngine::Vector3 kRightShoulderPivot = { 0.72f, 2.25f, 0.0f }; //右の肩
	inline constexpr RyoEngine::Vector3 kLeftHipPivot = { -0.39f, 1.15f, 0.05f }; //左の足のつけ根
	inline constexpr RyoEngine::Vector3 kRightHipPivot = { 0.39f, 1.15f, 0.05f }; //右の足のつけ根

	//ドスン
	inline constexpr float kSlamSquash = 0.35f; //叩きつけられて縦につぶれる量
	inline constexpr float kSlamSpringSpeed = 26.0f; //つぶれたのがぽよんと戻る速さ
	inline constexpr float kSlamSpringDamping = 7.0f; //ぽよんが収まる速さ
	inline constexpr float kSlamShake = 0.8f; //画面が揺れる強さ
	inline constexpr float kSlamJiggle = 2.0f; //下の層の床がぷるるんと揺れる強さ

	//足場が当たるたびのむぎゅ
	inline constexpr float kHitSquash = 0.18f; //足場が当たって縦につぶれる量
	inline constexpr float kLastHitSquash = 0.4f; //最後の1枚が当たったときに大きくむぎゅーっとつぶれる量
	inline constexpr float kHitFlash = 1.2f; //当たった瞬間に白く光る強さ
	inline constexpr float kHitFlashDecay = 10.0f; //白い光が消える速さ
	inline constexpr RyoEngine::Vector3 kHitFlashColor = { 1.0f, 0.92f, 0.96f }; //当たったときに光る色
	inline constexpr float kHitShake = 0.12f; //足場が1枚当たるごとに画面が揺れる強さ
	inline constexpr float kBarrageTargetHeight = 2.2f; //足場が飛んでいく先のボスの足元からの高さ

	//ドンのあと上の足場がふわっと浮いて光る
	inline constexpr float kLiftHeight = 0.4f; //浮く高さ
	inline constexpr float kLiftGlow = 0.8f; //光の強さで、まぶしくしない
	inline constexpr RyoEngine::Vector3 kLiftColor = { 1.0f, 0.4f, 0.72f }; //光のピンク
	inline constexpr float kLiftHoverHeight = 0.05f; //浮いている間にふわふわ上下する幅
	inline constexpr float kLiftHoverSpeed = 8.0f; //ふわふわの速さ
	inline constexpr float kLiftHoverPhaseStep = 1.7f; //となりの足場とふわふわをずらすラジアン

	//プレイヤーが着地する
	inline constexpr float kLandJiggle = 1.2f; //着地で下の層の床が揺れる強さ
	inline constexpr float kLandShake = 0.2f; //着地で画面が揺れる強さ
	inline constexpr float kDiveSpinSpeed = 720.0f; //落ちながらくるくる回る1秒の度数

	//ハンマーで地面をドン
	inline constexpr float kHammerJiggle = 2.4f; //ドンで床がぷるるんとなる強さ
	inline constexpr float kHammerShake = 0.55f; //ドンで画面が揺れる強さ
	inline constexpr float kHammerHearts = 1.6f; //ドンで出るハートの強さ
	inline constexpr float kHammerHeartSize = 1.4f; //ドンで出るハートの大きさの倍率

	//エンジンのパーティクルで、エディターに保存した値があるとそちらが優先
	inline constexpr const char* kSlamPuffName = "BossSlamPuff"; //ボスが叩きつけられたときに足元に広がるピンクの煙
	inline constexpr const char* kHammerRingName = "HammerSlamRing"; //ハンマーのドンで地面に広がる光の輪
	inline constexpr uint32_t kSlamPuffMaxParticles = 128; //煙の最大の数
	inline constexpr uint32_t kSlamPuffCount = 28; //1回に出す煙の数
	inline constexpr float kSlamPuffRadius = 1.6f; //煙が出る輪の半径
	inline constexpr float kSlamPuffInnerRatio = 0.6f; //輪の内側の空ける割合
	inline constexpr float kSlamPuffStartScale = 0.6f; //出たときの大きさの割合
	inline constexpr float kSlamPuffEndScale = 1.4f; //消えるときの大きさの割合で、ふわっと広がる
	inline constexpr float kSlamPuffSpeedMin = 2.0f; //外へ広がる速さの最小
	inline constexpr float kSlamPuffSpeedMax = 4.5f; //外へ広がる速さの最大
	inline constexpr float kSlamPuffDrag = 3.0f; //煙が遅くなる強さ
	inline constexpr float kSlamPuffLifeMin = 0.35f; //煙が消えるまでの秒数の最小
	inline constexpr float kSlamPuffLifeMax = 0.6f; //煙が消えるまでの秒数の最大
	inline constexpr float kSlamPuffScaleMin = 0.35f; //煙の大きさの最小
	inline constexpr float kSlamPuffScaleMax = 0.7f; //煙の大きさの最大
	inline constexpr RyoEngine::Vector4 kSlamPuffColorA = { 1.0f, 0.78f, 0.88f, 0.55f }; //淡いピンクの煙
	inline constexpr RyoEngine::Vector4 kSlamPuffColorB = { 1.0f, 0.95f, 0.98f, 0.45f }; //白っぽい煙
	inline constexpr float kSlamPuffIntensity = 1.0f; //煙の明るさで、まぶしくしない
	inline constexpr uint32_t kHammerRingMaxParticles = 128; //光の輪の最大の数
	inline constexpr uint32_t kHammerRingCount = 36; //1回に出す光の数
	inline constexpr float kHammerRingRadius = 0.4f; //光が出る輪の半径
	inline constexpr float kHammerRingSpeedMin = 5.0f; //外へ広がる速さの最小
	inline constexpr float kHammerRingSpeedMax = 6.0f; //外へ広がる速さの最大
	inline constexpr float kHammerRingDrag = 4.0f; //光が遅くなる強さ
	inline constexpr float kHammerRingLifeMin = 0.3f; //光が消えるまでの秒数の最小
	inline constexpr float kHammerRingLifeMax = 0.4f; //光が消えるまでの秒数の最大
	inline constexpr float kHammerRingScaleMin = 0.18f; //光の大きさの最小
	inline constexpr float kHammerRingScaleMax = 0.28f; //光の大きさの最大
	inline constexpr float kHammerRingEndScale = 0.4f; //消えるときの大きさの割合
	inline constexpr RyoEngine::Vector4 kHammerRingColorA = { 1.0f, 0.35f, 0.68f, 0.8f }; //濃いピンク
	inline constexpr RyoEngine::Vector4 kHammerRingColorB = { 1.0f, 0.8f, 0.92f, 0.9f }; //淡いピンク
	inline constexpr float kHammerRingIntensity = 1.6f; //光の明るさ
	inline constexpr float kParticleLift = 0.15f; //床に埋まらないように上げる高さ
}
