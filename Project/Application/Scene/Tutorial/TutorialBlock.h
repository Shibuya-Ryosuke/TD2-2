#pragma once
#include <RyoEngine.h>
#include <memory>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;
	class StageCamera;
	class GameDirection;

	//空から降ってきてカメラが追いかけ、床が崩れると一緒に落ちて消える仮の積み木
	class TutorialBlock
	{
	public:
		TutorialBlock() = default;
		~TutorialBlock() = default;

		TutorialBlock(const TutorialBlock&) = delete;
		TutorialBlock& operator=(const TutorialBlock&) = delete;

		void Initialize(); //積み木のモデルを作る

		//見上げたカメラから、cellへ降ってくるのを始める
		void Drop(const Floor& floor, const Cell& cell);
		//降ってくる間とカメラが戻るまではtrueで、その間はプレイヤーを動かさない
		bool IsCinematic() const { return state_ == State::Falling || state_ == State::CameraBack; }
		//床の上にいるか
		bool IsResting() const { return state_ == State::Resting || state_ == State::CameraBack; }
		//床と一緒に落ちて消えたか
		bool IsGone() const { return state_ == State::Gone; }
		//落ち始めたか
		bool IsSinking() const { return state_ == State::Sinking || state_ == State::Gone; }
		//乗っているマス
		const Cell& GetCell() const { return cell_; }

		//1フレーム進める
		void Update(float deltaTime, Floor& floor, StageCamera& camera, GameDirection& direction);
		void Draw();

	private:
		//積み木の状態
		enum class State
		{
			Hidden, //まだ出ていない
			Falling, //空から降ってくる
			CameraBack, //ドン！のあとカメラがいつもに戻る
			Resting, //床の上にいる
			Sinking, //床と一緒に落ちていく
			Gone, //消えた
		};

		void UpdateFalling(Floor& floor, StageCamera& camera, GameDirection& direction);
		void UpdateSinking(float deltaTime, const Floor& floor);

		std::unique_ptr<RyoEngine::Model> model_{};
		State state_ = State::Hidden;
		Cell cell_{};
		RyoEngine::Vector3 position_{}; //積み木の底の真ん中
		float size_ = 1.0f; //1辺の長さ
		float timer_ = 0.0f; //今の状態に入ってからの秒数
		float scaleRate_ = 1.0f; //落ちて消えるときに小さくなる割合
	};
}
