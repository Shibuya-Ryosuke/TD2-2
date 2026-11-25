#pragma once
#include <RyoEngine.h>

namespace App
{
	//床の1マスぶんの情報
	struct FloorTile
	{
		bool isSolid = false; //今床があるか
		bool isSolidAtStart = false; //Resetで戻すCSVでの形
		bool isIsolated = false; //切り離された床か
		bool isVisible = false; //描くか
		bool isPreview = false; //崩すマスの予告を出しているか
		float remaining = -1.0f; //崩れるまでの残り時間でマイナスは予定なし
		float fallTime = 0.0f; //穴になってから落ちている時間
		RyoEngine::Vector3 position{}; //揺れや落下や飛んでいく途中も入れた今の位置
		RyoEngine::Vector3 rotate{}; //飛んでいく途中だけ回る度数の回転
		float scaleRate = 1.0f; //飛んでいく途中だけ小さくなる大きさの割合
		float shakeRate = 0.0f; //ひびの演出用の切り離されてから落ちるまでの進み具合
		float dropRate = 0.0f; //砕ける演出用の穴になってから消えるまでの進み具合
		float heartAngle = 0.0f; //崩したときのプレイヤーの向きのハートの向き
	};
}
