#pragma once
#include <RyoEngine.h>
#include <memory>
#include "../../Object/Player/Rig/PlayerRig.h"

namespace App
{
	//タイトルでプレイヤーが積み木を持ってゆらゆらし、スタートで積み木を落としてあせり、床ごと落ちて仰向けでじたばたする
	class TitleDrop
	{
	public:
		TitleDrop() = default;
		~TitleDrop() = default;

		//モデルを持つのでコピーを禁止する
		TitleDrop(const TitleDrop&) = delete;
		TitleDrop& operator=(const TitleDrop&) = delete;

		//このフレームで起きたこと
		struct Result
		{
			bool isRumbleNow = false; //床をがたがた揺らし始める
			bool isFloorDropNow = false; //床を落とし始める
		};

		void Initialize(); //プレイヤーと持っている積み木を作る
		Result Update(float deltaTime); //ゆらゆらか、落としてから落ちるまでを1フレーム進める
		void Draw();

		//積み木を落とす流れを始める
		void Start();
		bool IsStarted() const { return isStarted_; }
		//スタートしてからの秒数
		float GetStartTime() const { return startTimer_; }
		//プレイヤーの足元のワールドの位置
		const RyoEngine::Vector3& GetFeet() const { return feet_; }
		//落とした積み木のまん中のワールドの位置
		const RyoEngine::Vector3& GetBlockPosition() const { return blockPosition_; }

	private:
		void UpdateBlock(float deltaTime); //手の中か、落ちてはねる積み木
		RigPose MakePose() const; //びっくり、あせる、宙で固まる、じたばた落ちる、仰向けでばたばたの姿勢

		PlayerRig rig_; //積み木を持つプレイヤー
		std::unique_ptr<RyoEngine::Model> block_{}; //持っている積み木
		RyoEngine::Vector3 blockPosition_{};
		RyoEngine::Vector3 blockVelocity_{};
		RyoEngine::Vector3 blockRotate_{}; //度
		RyoEngine::Vector3 blockSpin_{}; //1秒に回る度
		RyoEngine::Vector3 feet_{}; //プレイヤーの足元
		float playerVelocityY_ = 0.0f;
		float faceYaw_ = 0.0f; //カメラの方を向く度
		float timer_ = 0.0f; //始まってからの秒数
		float startTimer_ = 0.0f; //スタートしてからの秒数
		bool isStarted_ = false;
		bool isFloorDropped_ = false; //床を消し始めたか
	};
}
