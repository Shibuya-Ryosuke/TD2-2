#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace App::DirectionConfig
{
	//演出の時間と色と強さの数値

	//場面切り替えの積み木
	inline constexpr const char* kTransitionBlockShader = "TransitionBlock"; //積み木と紙吹雪のシェーダー
	inline constexpr const char* kGustLinesShader = "GustLines"; //吹き飛ぶときの横風のポストエフェクト
	inline constexpr float kTransitionDepth = 20.0f; //遠近法のカメラから積み木の壁までの縮める前の距離
	inline constexpr float kTransitionNearScale = 0.05f; //遠近法のときに積み木をカメラの近くへ縮める倍率で、壁はカメラから1の所に来る
	inline constexpr float kTransitionOrthoDepth = 2.0f; //平行投影のカメラから積み木の壁までの距離で、床より手前
	inline constexpr float kTransitionNearHide = 0.5f; //これよりカメラに近づいたブロックは消す
	inline constexpr float kTransitionMaxDeltaTime = 1.0f / 30.0f; //読み込み直後に一気に進まないよう1フレームで進める秒数の上限
	//がれき
	inline constexpr int32_t kTransitionDebrisCount = 20; //がれきの数
	inline constexpr float kTransitionDebrisTime = 0.35f; //がれきが落ちきるまでの秒数
	inline constexpr float kTransitionDebrisSpread = 0.12f; //落ち始めのばらつきの秒数
	inline constexpr float kTransitionDebrisMinSize = 0.2f; //積み木の高さに対する大きさの最小
	inline constexpr float kTransitionDebrisSizeRange = 0.2f; //大きさのばらつき
	inline constexpr float kTransitionDebrisSway = 0.6f; //紙吹雪が左右にひらひらする幅
	inline constexpr float kTransitionDebrisSwaySpeed = 30.0f; //ひらひらの速さ
	//紙吹雪とドン！の縁の光に使うパステルの色
	inline constexpr std::array<RyoEngine::Vector4, 6> kTransitionPopColors = { {
		{ 1.0f, 0.55f, 0.75f, 1.0f }, //ピンク
		{ 1.0f, 0.85f, 0.35f, 1.0f }, //きいろ
		{ 0.5f, 0.95f, 0.7f, 1.0f }, //ミント
		{ 0.5f, 0.8f, 1.0f, 1.0f }, //みずいろ
		{ 0.78f, 0.6f, 1.0f, 1.0f }, //むらさき
		{ 1.0f, 0.68f, 0.45f, 1.0f }, //オレンジ
	} };
	//積み上がる
	inline constexpr int32_t kTransitionColumns = 10; //横に並べられるいちばん多い数で、画面の横幅に合わせて使う数を決める
	inline constexpr int32_t kTransitionRows = 5; //縦の数で、積み木の大きさは画面の高さをこれで割った正方形
	inline constexpr float kTransitionBlockGap = 0.015f; //マスに対する積み木どうしのすき間
	inline constexpr float kTransitionStackTime = 0.9f; //全部積み上がるまでの秒数
	inline constexpr float kTransitionBlockFallTime = 0.15f; //1個が落ちてくる秒数
	inline constexpr float kTransitionGlowTime = 0.2f; //ドン！の縁の光が消えるまでの秒数
	inline constexpr float kTransitionSquash = 0.3f; //ドン！で高さに対してつぶれる量
	inline constexpr float kTransitionSquashDamping = 8.0f; //ぽよよんが収まる速さ
	inline constexpr float kTransitionSquashSpeed = 36.0f; //ぽよよんの細かさ
	inline constexpr float kTransitionSquashWiden = 0.7f; //つぶれたとき横にふくらむ量
	inline constexpr float kTransitionFallStretch = 0.15f; //落ちている間に縦にのびる量
	inline constexpr float kTransitionHoldTime = 0.1f; //埋まってから次のシーンへ行くまでの秒数
	//次のシーンで吹き飛ぶ間は操作させない
	inline constexpr float kTransitionBlowTime = 0.9f; //全部吹き飛ぶまでの秒数
	inline constexpr float kTransitionBlowColumnDelay = 0.06f; //風は左からなので1列右ごとに飛び始めが遅れる秒数
	inline constexpr float kTransitionBlowRandomDelay = 0.08f; //飛び始めのばらつきの秒数
	inline constexpr float kTransitionBlowDistance = 1.2f; //画面の外へ出てからさらに飛ぶ長さ
	inline constexpr float kTransitionBlowLift = 0.35f; //ふわっと上下に流れる量
	inline constexpr float kGustLinesStrength = 1.0f; //横風の線の強さ

	//床が崩れる演出
	//ハートがポンっ、ガン！でヒビ、ガン！でぱっかーん、ガタガタ、ひびにそって砕ける
	inline constexpr const char* kFloorTileShader = "FloorTile";
	inline constexpr RyoEngine::Vector3 kHeartColor = { 1.0f, 0.3f, 0.62f }; //飛び散る欠片の光の色
	inline constexpr float kHeartPopTime = 0.3f; //ハートがポンっと出て落ち着くまでの秒数
	inline constexpr std::array<float, 2> kHammerHits = { 0.35f, 0.62f }; //崩れるまでの時間に対するガン！2回のタイミング
	inline constexpr float kHeartCrackTime = 0.15f; //1回目のガン！でヒビが走りきるまでの秒数
	inline constexpr float kHeartBreakTime = 0.3f; //2回目のガン！でハートが割れて消えるまでの秒数
	inline constexpr float kTileCrackTime = 0.12f; //2回目のガン！でひびがタイル全体へ走りきるまでの秒数
	inline constexpr float kHammerFlashDecay = 12.0f; //ガン！の光が消える速さ
	inline constexpr float kHammerJolt = 0.1f; //ガン！でタイルがビクッと沈む量
	inline constexpr float kCrumbleShakeTime = 0.45f; //穴になる何秒前からガタガタ揺れるか
	inline constexpr float kCrumbleShakeDistance = 0.08f; //ガタガタのいちばん大きい揺れ幅
	inline constexpr float kCrumbleShakeSpeed = 55.0f; //ガタガタの速さ
	inline constexpr float kCrumblePuff = 0.12f; //穴になる直前にぷくっとふくらむ量
	inline constexpr float kDropPop = 0.15f; //落ち始めにぽんっとふくらむ量
	inline constexpr float kWearGrowTime = 0.35f; //穴のとなりになった床に細いヒビが広がりきるまでの秒数
	inline constexpr float kDropPopTime = 0.25f; //落ちる時間に対するぽんっの長さ
	inline constexpr float kIsolatedDissolveStart = 0.3f; //切り離された床がどこまで落ちたら砕け始めるか
	//砕けたときに飛び散るピンクの欠片
	inline constexpr size_t kShardPoolSize = 96; //使い回す欠片の数
	inline constexpr int32_t kShardsPerTile = 6; //1マスで飛ぶ欠片の数
	inline constexpr float kShardSize = 0.2f; //タイルの幅に対する欠片の大きさ
	inline constexpr float kShardSpeed = 3.0f; //外へ飛ぶ速さ
	inline constexpr float kShardJump = 5.0f; //上へはじける速さ
	inline constexpr float kShardGravity = 20.0f; //落ちる強さ
	inline constexpr float kShardLife = 0.6f; //消えるまでの秒数
	inline constexpr float kShardGlow = 0.8f; //欠片の光の強さ
	inline constexpr RyoEngine::Vector4 kShardColor = { 1.0f, 0.62f, 0.8f, 1.0f }; //欠片の色

	//見た目だけのプレイヤーの着地のピカッとほこり
	inline constexpr const char* kPlayerPopShader = "PlayerPop"; //着地でふちがピカッと光るシェーダー
	inline constexpr float kPlayerLandSpeedRef = 10.0f; //この速さの着地をふつうの着地にする
	inline constexpr float kPlayerLandMinSpeed = 3.0f; //これより遅い着地はピカッとしない
	inline constexpr float kPlayerStopSpeed = 2.0f; //着地したとみなす止まった速さ
	inline constexpr float kPlayerTeleportDistance = 2.0f; //1フレームでこれより動いたら置き直しとみなす
	inline constexpr float kPlayerLandGlow = 1.0f; //着地のピカッの強さ
	inline constexpr float kPlayerLandGlowDecay = 7.0f; //ピカッが消える速さ
	//向きが変わったときにクルッと回る
	inline constexpr float kPlayerTurnSpeed = 1800.0f; //1秒に回る度数で、半回転が0.1秒

	//プレイヤーのアニメーションで、名前はglbの中のアニメーションの名前
	inline constexpr const char* kPlayerJumpFile = "animation/Player/playerJump.glb"; //移動のアニメーションのファイル
	inline constexpr const char* kPlayerSwingFile = "animation/Player/playerSwingDown.glb"; //足場を壊すアニメーションのファイル
	inline constexpr const char* kPlayerPoseFile = "animation/Player/playerPice.glb"; //クリアのポーズのファイル
	inline constexpr const char* kPlayerIdleClip = "Animation"; //モデルのplayerIdol.glbに入っている待機のループ
	inline constexpr const char* kPlayerJumpClip = "Jump"; //移動で1回跳ぶ
	inline constexpr const char* kPlayerSwingClip = "Swing"; //ハンマーを振り下ろして首をかしげる
	inline constexpr const char* kPlayerPoseClip = "Pose"; //クリアのポーズ
	inline constexpr float kPlayerAnimBlend = 0.12f; //アニメーションを切り替えるときに混ぜる秒数
	inline constexpr float kPlayerJumpLandTime = 0.69f; //ジャンプのアニメーションで足が床に着く秒
	inline constexpr float kPlayerSwingSpeed = 2.8f; //振り上げてから振り下ろすまでの再生の速さ
	inline constexpr float kPlayerSwingImpactTime = 1.39f; //振るアニメーションでハンマーが床に当たる秒
	inline constexpr float kPlayerSwingAfterSpeed = 1.2f; //当たってから首をかしげ終わるまでの再生の速さ
	inline constexpr float kSwingImpactDelay = kPlayerSwingImpactTime / kPlayerSwingSpeed; //ボタンを押してからハンマーが当たるまでの秒数
	inline constexpr float kBreakRejectJiggle = 1.3f; //チュートリアルで壊せないマスを叩いたときのぷるるんの強さ

	//足場を壊すときに右手で振るハンマー
	inline constexpr const char* kHammerModel = "Model/hanmer/hanmer.obj"; //柄の下が原点で柄が上へのびるハンマー
	inline constexpr const char* kHammerJoint = "Hand.R"; //ハンマーを持つ右手のボーン
	inline constexpr RyoEngine::Vector3 kHammerRotateDeg = { -38.414f, -2.325f, -55.391f }; //右手のボーンから見たハンマーの向き
	inline constexpr RyoEngine::Vector3 kHammerOffset = { -0.2307f, -0.0285f, 0.2173f }; //右手のボーンから見たハンマーの柄の下の位置
	inline constexpr float kHammerGrip = 0.35f; //手で持つ所の柄の下からの高さで、床にめり込むときはここを中心に起こす
	inline constexpr RyoEngine::Vector3 kHammerHeadMin = { -0.94f, 1.78f, -0.5f }; //ハンマーの頭の箱のいちばん小さい角
	inline constexpr RyoEngine::Vector3 kHammerHeadMax = { 0.94f, 3.14f, 0.88f }; //ハンマーの頭の箱のいちばん大きい角
	inline constexpr float kHammerPopTime = 0.1f; //振り始めにポンっと出てくる秒数
	inline constexpr float kHammerHideTime = 0.12f; //首をかしげ終わってからしゅっと消える秒数

	//クリアのときにプレイヤーをアップにしてポーズ
	inline constexpr float kClearZoomTime = 1.0f; //カメラがプレイヤーへ寄っていく秒数
	inline constexpr float kClearZoomOrthoSize = 4.5f; //寄ったときに画面の縦の半分に映す長さ
	inline constexpr float kClearFocusHeight = 1.2f; //画面の真ん中に来るプレイヤーの足元からの高さ
	inline constexpr float kClearHoldTime = 1.0f; //ポーズのあとクリア画面へ行くまで待つ秒数

	//じたばたと尻もちの姿勢
	inline constexpr float kPoseBlendSpeed = 10.0f; //姿勢が切りかわる速さ
	inline constexpr float kSitDropSpeed = 25.0f; //尻もちでどすんと座る速さ
	inline constexpr float kFlailSpeed = 22.0f; //じたばたで手足を振る速さ
	inline constexpr float kSitSink = 1.0f; //尻もちで沈む高さで、プレイヤーの倍率をかける前の長さ

	//ボス戦で穴に落ちたとき
	inline constexpr float kFallTime = 1.3f; //じたばたしながら下の層へ落ちる秒数
	inline constexpr float kFallZoomTime = 0.45f; //プレイヤーへ寄りきるまでの秒数
	inline constexpr float kFallZoomOrthoSize = 6.0f; //寄ったときに画面の縦の半分に映す長さ
	inline constexpr float kFallFloorShrinkTime = 0.5f; //落ちた層の床が小さくなって消えるまでの秒数
	inline constexpr float kFallLandJiggle = 1.6f; //尻もちで足場が揺れる強さ
	inline constexpr float kFallLandHearts = 1.6f; //尻もちで出るハートの強さ
	inline constexpr float kFallLandShake = 0.35f; //尻もちで画面が揺れる強さ
	inline constexpr float kFallSitTime = 0.9f; //尻もちをついている秒数
	inline constexpr float kFallBossDropTime = 0.6f; //ボスが上から降りてくる秒数
	inline constexpr float kFallBossDropHeight = 14.0f; //ボスが降り始める高さ
	inline constexpr float kFallBossLandShake = 0.6f; //ボスが着いたときに画面が揺れる強さ
	inline constexpr float kFallZoomOutTime = 0.5f; //ボスが着いてからカメラが戻りきるまでの秒数

	//ボスが落ちて足場が当たったあと、足場がなくなったプレイヤーも落ちる
	inline constexpr float kBossFallViewHeight = 0.5f; //上の層から下の層までのどこにカメラを置くかで0.5はまん中
	inline constexpr float kBossFallViewZoom = 1.15f; //上の層と下の層の両方が入るようにいつもより引く倍率
	inline constexpr float kBossFallViewReturnTime = 0.6f; //プレイヤーが立ち上がってからカメラが下の層へ戻る秒数
	inline constexpr float kTumbleTime = 0.9f; //足場がなくなってから下の層に尻もちをつくまでの秒数
	inline constexpr float kTumbleSitTime = 0.6f; //尻もちをついている秒数

	//跳んだときと着地で足場からはじけて舞い上がるハート
	inline constexpr const char* kHeartPopShader = "HeartPop";
	inline constexpr size_t kHeartPoolSize = 64; //使い回すハートの数
	inline constexpr float kHeartMaxPower = 1.4f; //強い着地でも数と高さがこれ以上ふえない強さ
	inline constexpr int32_t kHeartPerBurst = 5; //ふつうの着地で出る数で、控えめにする
	inline constexpr float kHeartSize = 0.5f; //大きさで、形はわかるけど主張しすぎない大きさ
	inline constexpr float kHeartRingRadius = 0.4f; //足元のどれくらい外から出るか
	inline constexpr float kHeartOutSpeed = 1.0f; //外へ広がる速さ
	inline constexpr float kHeartUpSpeed = 1.8f; //上へはじける速さで、足元の少し上までにする
	inline constexpr float kHeartBuoyancy = 0.3f; //ふわっと浮いていく強さ
	inline constexpr float kHeartDrag = 2.6f; //だんだん遅くなる強さ
	inline constexpr float kHeartSwaySpeed = 7.0f; //左右にゆれる速さ
	inline constexpr float kHeartSwayWidth = 0.1f; //左右にゆれる幅
	inline constexpr float kHeartBurstPopTime = 0.18f; //ぽんっと出てくる秒数
	inline constexpr float kHeartLife = 0.8f; //消えるまでの秒数
	inline constexpr float kHeartTilt = 0.35f; //ハートが左右に傾くいちばん大きいラジアン
	inline constexpr float kHeartTiltSpeed = 1.5f; //ハートが傾いていく速さのいちばん大きい値
	inline constexpr float kHeartGlow = 0.8f; //光の強さ
	inline constexpr std::array<RyoEngine::Vector4, 6> kHeartColors = { { //ホットピンク、桃、ミント、ラベンダー、水色、ピーチ
		{ 1.0f, 0.18f, 0.55f, 1.0f },
		{ 1.0f, 0.55f, 0.75f, 1.0f },
		{ 0.45f, 0.95f, 0.75f, 1.0f },
		{ 0.75f, 0.55f, 1.0f, 1.0f },
		{ 0.45f, 0.85f, 1.0f, 1.0f },
		{ 1.0f, 0.7f, 0.5f, 1.0f } } };

	//ゲームシーンの色の仕上げ
	inline constexpr bool kGameUsesAces = false; //trueで明るい所をなめらかに詰めるACESを使い、falseで画像の色のまま出す

	//ダメージを受けたときに赤くぎらっと光る画面のふち
	inline constexpr const char* kDamageVignetteShader = "DamageVignette";
	inline constexpr float kDamageFlashRise = 0.04f; //パッと光るまでの秒数
	inline constexpr float kDamageFlashTime = 0.75f; //光り始めてからすっと消えるまでの秒数
	inline constexpr float kDamageFlashStrength = 1.0f; //シェーダーの値に掛ける強さ
	inline constexpr float kDamageShakePower = 0.45f; //ダメージで画面が揺れる強さ

	//画面の揺れ
	inline constexpr float kShakeTime = 0.35f; //強さ1で揺れが収まるまでの秒数
	inline constexpr float kShakeSpeed = 55.0f; //揺れの速さ

	//層を降りるときに下から上へ流れる線
	inline constexpr const char* kFallLinesShader = "FallLines";
	inline constexpr float kFallLinesRise = 0.15f; //線が出きるまでの秒数
	inline constexpr float kFallLinesFade = 0.3f; //降り終わってから消えるまでの秒数
	inline constexpr float kFallLinesStrength = 1.0f; //線の強さ

	//着地したときの足場のぷるるん
	inline constexpr float kJiggleRadius = 1.5f; //揺れが広がるマスの数
	inline constexpr float kJiggleRippleDelay = 0.05f; //1マス離れるごとに揺れ始めが遅れる秒数
	inline constexpr float kJiggleDepth = 0.12f; //沈む深さ
	inline constexpr float kJiggleSquash = 0.35f; //縦につぶれる量
	inline constexpr float kJiggleWiden = 0.08f; //横にふくらむ量
	inline constexpr float kJiggleDamping = 7.0f; //ぷるぷるが収まる速さ
	inline constexpr float kJiggleSpeed = 32.0f; //ぷるぷるの速さ
	inline constexpr float kJigglePlayerMax = 1.8f; //プレイヤーの着地の強さの上限
	inline constexpr float kJiggleBossPower = 1.6f; //重いボスの着地の強さ
}
