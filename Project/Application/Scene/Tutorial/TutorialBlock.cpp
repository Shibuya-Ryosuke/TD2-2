#include "TutorialBlock.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/TutorialConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Tsumiki/Tsumiki.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float SmoothStep(float t)
		{
			return t * t * (3.0f - 2.0f * t);
		}
	}

	//仮のcube.objをパステルピンクにした積み木
	void TutorialBlock::Initialize()
	{
		model_ = Tsumiki::Create();
		model_->SetColor(TutorialConfig::kBlockColor);
		state_ = State::Hidden;
	}

	//見上げたカメラから降らせ始める
	void TutorialBlock::Drop(const Floor& floor, const Cell& cell)
	{
		cell_ = cell;
		size_ = std::min(floor.GetTileSizeX(), floor.GetTileSizeZ()) * TutorialConfig::kBlockSizeRate;
		position_ = floor.GetCellCenter(cell);
		position_.y += TutorialConfig::kBlockDropHeight;
		scaleRate_ = 1.0f;
		timer_ = 0.0f;
		state_ = State::Falling;
	}

	//状態ごとに1フレーム進める
	void TutorialBlock::Update(float deltaTime, Floor& floor, StageCamera& camera, GameDirection& direction)
	{
		timer_ += deltaTime;
		if (state_ == State::Falling)
		{
			UpdateFalling(floor, camera, direction);
		}
		else if (state_ == State::CameraBack)
		{
			//寄って傾けたカメラをいつもの映し方へ戻す
			const float back = SmoothStep(std::clamp(timer_ / TutorialConfig::kBlockCameraBackTime, 0.0f, 1.0f));
			const Vector3 center = { position_.x, position_.y + size_ * 0.5f, position_.z };
			camera.SetFocusOffset(camera.GetCenteringOffset(center) * (1.0f - back));
			camera.SetOrthoSize(TutorialConfig::kBlockZoomOrthoSize + (camera.GetBaseOrthoSize() - TutorialConfig::kBlockZoomOrthoSize) * back);
			if (back >= 1.0f)
			{
				camera.ResetShot();
				state_ = State::Resting;
			}
		}
		if (state_ == State::Resting || state_ == State::CameraBack)
		{
			//乗っているマスが揺れたら一緒に揺れ、穴か切り離されて落ち始めたら一緒に落ちる
			const FloorTile& tile = floor.GetTile(cell_);
			position_ = tile.position;
			position_.y += FloorConfig::kTileThickness * 0.5f;
			if (!tile.isSolid || tile.dropRate > 0.0f)
			{
				camera.ResetShot();
				state_ = State::Sinking;
				timer_ = 0.0f;
			}
		}
		if (state_ == State::Sinking)
		{
			UpdateSinking(deltaTime, floor);
		}

		//底の真ん中に1辺size_の箱を置く
		const float size = size_ * scaleRate_;
		Tsumiki::Place(*model_, { position_.x, position_.y + size * 0.5f, position_.z }, { size, size, size }, { 0.0f, 0.0f, 0.0f });
		model_->TransferMatrix();
	}

	//見上げたカメラが積み木を追いかけて、床にドン！
	void TutorialBlock::UpdateFalling(Floor& floor, StageCamera& camera, GameDirection& direction)
	{
		const float t = std::clamp(timer_ / TutorialConfig::kBlockDropTime, 0.0f, 1.0f);
		const Vector3 ground = floor.GetCellCenter(cell_);
		position_ = { ground.x, ground.y + TutorialConfig::kBlockDropHeight * (1.0f - t * t), ground.z };

		//見上げた向きから下へ傾けながら、積み木がいつも画面の真ん中に来るようにする
		const float look = SmoothStep(t);
		camera.SetPitchOffset(TutorialConfig::kBlockLookUpPitch * (1.0f - look));
		camera.SetOrthoSize(TutorialConfig::kBlockZoomOrthoSize);
		camera.SetFocusOffset(camera.GetCenteringOffset({ position_.x, position_.y + size_ * 0.5f, position_.z }));
		if (t < 1.0f)
		{
			return;
		}

		//ドン！で足場が揺れて画面も揺れる
		position_ = ground;
		camera.SetPitchOffset(0.0f);
		direction.Shake(TutorialConfig::kBlockLandShake);
		floor.Jiggle(cell_, TutorialConfig::kBlockLandJiggle);
		state_ = State::CameraBack;
		timer_ = 0.0f;
	}

	//床と一緒に下がって、床がなくなったらそのまま落ちて小さくなる
	void TutorialBlock::UpdateSinking(float deltaTime, const Floor& floor)
	{
		const FloorTile& tile = floor.GetTile(cell_);
		if (tile.isVisible)
		{
			position_ = tile.position;
			position_.y += FloorConfig::kTileThickness * 0.5f;
		}
		else
		{
			position_.y -= TutorialConfig::kBlockSinkSpeed * deltaTime;
		}
		scaleRate_ = std::clamp(1.0f - timer_ / TutorialConfig::kBlockSinkTime, 0.0f, 1.0f);
		if (scaleRate_ <= 0.0f)
		{
			state_ = State::Gone;
		}
	}

	//出ている間だけ描く
	void TutorialBlock::Draw()
	{
		if (state_ == State::Hidden || state_ == State::Gone)
		{
			return;
		}
		model_->Draw();
	}
}
