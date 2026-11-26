#pragma once
#include <RyoEngine.h>
#include <array>
#include <memory>
#include "RigPose.h"

namespace App
{
	//頭と体と両腕と両足のモデルを関節の位置を親にしてつなぎ、SRTだけで姿勢を作るプレイヤー
	class PlayerRig
	{
	public:
		PlayerRig() = default;
		~PlayerRig() = default;

		//モデルと親子のつながりを持つのでコピーを禁止する
		PlayerRig(const PlayerRig&) = delete;
		PlayerRig& operator=(const PlayerRig&) = delete;

		void Initialize(); //6つのパーツを読みこんで関節につなぐ
		//足元の位置と向きと倍率と姿勢から全部のパーツの行列を確定する
		void Apply(const RigPose& pose, const RyoEngine::Vector3& feet, float yawDeg, float scale);
		void Draw();

		//今の姿勢での手の先のワールドの位置
		RyoEngine::Vector3 GetHandPosition(bool isRight) const;
		//今の姿勢での頭のてっぺんあたりのワールドの位置
		RyoEngine::Vector3 GetHeadPosition() const;
		//右手の先で、持たせるモデルの親にする
		RyoEngine::WorldTransform* GetRightHand() { return &rightHand_; }

	private:
		//関節の番号
		enum Joint
		{
			kWaist, //腰
			kNeck, //首
			kShoulderL, //左の肩
			kShoulderR, //右の肩
			kHipL, //左の足のつけ根
			kHipR, //右の足のつけ根
			kJointCount,
		};

		//パーツの番号
		enum Part
		{
			kBody,
			kHead,
			kArmL,
			kArmR,
			kLegL,
			kLegR,
			kPartCount,
		};

		RyoEngine::WorldTransform root_; //足元の位置と向きと倍率
		RyoEngine::WorldTransform tilt_; //腰の位置で体全体を後ろへ倒す
		std::array<RyoEngine::WorldTransform, kJointCount> joints_; //関節ごとの回転
		RyoEngine::WorldTransform rightHand_; //右肩の子で右手の先
		std::array<std::unique_ptr<RyoEngine::Model>, kPartCount> parts_; //関節を親にしたパーツ
	};
}
