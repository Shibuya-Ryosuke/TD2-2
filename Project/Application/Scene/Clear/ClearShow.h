#pragma once
#include <RyoEngine.h>
#include <memory>
#include "ClearBoss.h"
#include "../../Object/Player/Rig/PlayerRig.h"

namespace App
{
	//クリアでプレイヤーが倒れたボスへ歩いてハンマーでたたき、ボスがバラバラに吹っ飛んで、くるっと回ってギャルっぽく踊る流れ
	class ClearShow
	{
	public:
		ClearShow() = default;
		~ClearShow() = default;

		//モデルを持つのでコピーを禁止する
		ClearShow(const ClearShow&) = delete;
		ClearShow& operator=(const ClearShow&) = delete;

		void Initialize(); //プレイヤーとハンマーとボスを作る
		void Update(float deltaTime); //流れを1フレーム進める
		void Draw();

		//今の映し方のカメラの位置と見る所
		void GetShot(RyoEngine::Vector3& eye, RyoEngine::Vector3& target) const;
		//踊り始めたか
		bool IsDancing() const;

	private:
		RigPose MakePose(float& yaw, RyoEngine::Vector3& feet) const; //今の流れの姿勢と向きと足元
		void UpdateHammer(); //右手のハンマーの行列を確定する

		PlayerRig rig_; //たたいて踊るプレイヤー
		std::unique_ptr<RyoEngine::Model> hammer_{}; //右手に持つハンマー
		ClearBoss boss_; //倒れているボス
		float timer_ = 0.0f; //当たって止まる間は進めない流れの秒数
		float hitStop_ = 0.0f; //当たって止まる残り
		float shake_ = 0.0f; //カメラの揺れの残り
		bool isHammerHidden_ = false; //ハンマーをしまったか
	};
}
