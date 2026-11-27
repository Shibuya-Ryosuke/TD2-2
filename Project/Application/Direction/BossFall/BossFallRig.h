#pragma once
#include <RyoEngine.h>
#include <array>
#include <memory>
#include "BossPose.h"

namespace App
{
	//ボスが落ちてから起き上がるまでだけ見せる、6つのパーツに分けたボスで、関節をSRTで回して動かす
	class BossFallRig
	{
	public:
		BossFallRig() = default;
		~BossFallRig() = default;

		//モデルと親子のつながりを持つのでコピーを禁止する
		BossFallRig(const BossFallRig&) = delete;
		BossFallRig& operator=(const BossFallRig&) = delete;

		void Initialize(); //6つのパーツを読みこんで、関節の位置を原点にずらして関節につなぐ
		//足元の位置と向きの度数と姿勢と、縦につぶす量から全部のパーツの行列を確定する
		void Apply(const BossPose& pose, const RyoEngine::Vector3& feet, float yawDeg, float squash);
		//当たった瞬間の白い光の強さを入れる
		void SetFlash(float flash);
		void Draw(); //6つのパーツを描く

	private:
		//関節の番号
		enum Joint : size_t
		{
			kPelvis, //胴体の下
			kNeck, //首
			kShoulderL, //左の肩
			kShoulderR, //右の肩
			kHipL, //左の足のつけ根
			kHipR, //右の足のつけ根
			kJointCount,
		};
		static constexpr size_t kPartCount = 6; //胴体と頭と両腕と両足

		//関節の回転を変える
		void SetJointRotate(Joint joint, const RyoEngine::Vector3& rotate);

		RyoEngine::WorldTransform root_; //足元の位置と向きと倍率
		std::array<RyoEngine::WorldTransform, kJointCount> joints_; //関節ごとの回転
		std::array<std::unique_ptr<RyoEngine::Model>, kPartCount> parts_; //関節を親にしたパーツ
	};
}
