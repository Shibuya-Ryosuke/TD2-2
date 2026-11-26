#include "ShowcaseStage.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/ModelConfig.h"
#include "../../Config/ShowcaseConfig.h"
#include "../../Direction/Transition/TransitionCamera.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//eyeからtargetを見るカメラと空と床を作る
	void ShowcaseStage::Initialize(const Vector3& eye, const Vector3& target)
	{
		//最初に作ったカメラが自動でアクティブになる
		const uint32_t cameraIndex = CameraManager::CreateCamera<TransitionCamera>("Showcase");
		camera_ = CameraManager::GetCamera<TransitionCamera>(cameraIndex);
		camera_->SetFovY(ShowcaseConfig::kCameraFovY);
		LookAt(eye, target);

		sky_.Initialize();
		toneMap_.Apply();
		nearFade_.Disable();

		//足元を原点にしたピンクの床を並べる
		tiles_.clear();
		isRumbling_ = false;
		rumbleTimer_ = 0.0f;
		isDropping_ = false;
		dropTimer_ = 0.0f;
		const float size = ShowcaseConfig::kTileSize;
		for (int32_t z = 0; z < ShowcaseConfig::kFloorTilesZ; ++z)
		{
			for (int32_t x = 0; x < ShowcaseConfig::kFloorTilesX; ++x)
			{
				Tile tile;
				tile.model = Model::Create(ModelConfig::kCubeModel);
				tile.model->SetColor(ShowcaseConfig::kFloorColor);
				tile.position = {
					(static_cast<float>(x) - static_cast<float>(ShowcaseConfig::kFloorTilesX - 1) * 0.5f) * size,
					-FloorConfig::kTileThickness * 0.5f,
					(static_cast<float>(z) - static_cast<float>(ShowcaseConfig::kFloorTilesZ - 1) * 0.5f) * size + size * 0.5f };
				PlaceTile(tile);
				tiles_.push_back(std::move(tile));
			}
		}
	}

	//ACESとNearFadeを元に戻す
	void ShowcaseStage::Finalize()
	{
		toneMap_.Restore();
		nearFade_.Restore();
	}

	//カメラの位置と見る所を変える
	void ShowcaseStage::LookAt(const Vector3& eye, const Vector3& target)
	{
		//前は奥のZから、Xで下を向いてからYで横を向く
		const Vector3 to = target - eye;
		const float flat = std::sqrt(to.x * to.x + to.z * to.z);
		camera_->SetTranslate(eye);
		camera_->SetRotate({ std::atan2(-to.y, flat), std::atan2(to.x, to.z), 0.0f });
		camera_->Update();
	}

	//ワールドの位置を1920x1080が基準のデバッグ表示の画面の位置にする
	Vector2 ShowcaseStage::ToScreen(const Vector3& world) const
	{
		const Matrix4x4& m = camera_->GetViewProjectionMatrix();
		const float x = world.x * m.m[0][0] + world.y * m.m[1][0] + world.z * m.m[2][0] + m.m[3][0];
		const float y = world.x * m.m[0][1] + world.y * m.m[1][1] + world.z * m.m[2][1] + m.m[3][1];
		float w = world.x * m.m[0][3] + world.y * m.m[1][3] + world.z * m.m[2][3] + m.m[3][3];
		if (std::abs(w) < 0.0001f)
		{
			w = 1.0f;
		}
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		return { (x / w * 0.5f + 0.5f) * width, (0.5f - y / w * 0.5f) * height };
	}

	//originに近い床ほど強くがたがた揺らし始める
	void ShowcaseStage::RumbleFloor(const Vector3& origin)
	{
		isRumbling_ = true;
		rumbleTimer_ = 0.0f;
		for (size_t i = 0; i < tiles_.size(); ++i)
		{
			Tile& tile = tiles_[i];
			const float dx = tile.position.x - origin.x;
			const float dz = tile.position.z - origin.z;
			const float distance = std::sqrt(dx * dx + dz * dz) / ShowcaseConfig::kTileSize;
			tile.rumble = 1.0f / (1.0f + distance);
			tile.phase = static_cast<float>(i) * 1.7f;
		}
	}

	//originに近い床から順にぴょこっと浮いて下へ落としていく
	void ShowcaseStage::DropFloor(const Vector3& origin)
	{
		isDropping_ = true;
		dropTimer_ = 0.0f;
		for (Tile& tile : tiles_)
		{
			const float dx = tile.position.x - origin.x;
			const float dz = tile.position.z - origin.z;
			const float distance = std::sqrt(dx * dx + dz * dz) / ShowcaseConfig::kTileSize;
			tile.delay = distance * ShowcaseConfig::kFloorFallStagger;
			tile.velocityY = 0.0f;
			tile.spin = 0.0f;
		}
	}

	//揺れる床と落ちる床を進めて、空をカメラに合わせて雲を流す
	void ShowcaseStage::Update(float deltaTime)
	{
		if (isRumbling_)
		{
			rumbleTimer_ += deltaTime;
			UpdateRumble();
		}
		if (isDropping_)
		{
			dropTimer_ += deltaTime;
			for (Tile& tile : tiles_)
			{
				const float time = dropTimer_ - tile.delay;
				if (time < 0.0f)
				{
					continue;
				}

				//ぴょこっと浮いてから、傾きながらだんだん速く落ちる
				if (time < ShowcaseConfig::kFloorFallPop)
				{
					tile.velocityY = 3.0f;
				}
				tile.velocityY -= ShowcaseConfig::kFloorFallGravity * deltaTime;
				tile.position.y += tile.velocityY * deltaTime;
				tile.spin += deltaTime * 40.0f;
				PlaceTile(tile);
			}
		}
		sky_.UpdatePerspective(*camera_);
	}

	//まだ落ちていない床をだんだん強くがたがた揺らす
	void ShowcaseStage::UpdateRumble()
	{
		const float ramp = std::min(rumbleTimer_ / ShowcaseConfig::kFloorRumbleRamp, 1.0f);
		for (Tile& tile : tiles_)
		{
			const float wave = rumbleTimer_ * ShowcaseConfig::kFloorRumbleSpeed + tile.phase;
			const float power = tile.rumble * ramp;
			tile.shake = { 0.0f, std::sin(wave) * ShowcaseConfig::kFloorRumbleHeight * power, 0.0f };
			tile.shakeTilt = std::sin(wave * 1.3f) * ShowcaseConfig::kFloorRumbleTilt * power;
			PlaceTile(tile);
		}
	}

	//行列を確定する
	void ShowcaseStage::PlaceTile(Tile& tile)
	{
		const float size = ShowcaseConfig::kTileSize;
		const float modelScale = 1.0f / ModelConfig::kCubeModelSize;
		tile.model->SetScale({ size * 0.96f * modelScale, FloorConfig::kTileThickness * modelScale, size * 0.96f * modelScale });
		tile.model->SetRotate({ tile.spin + tile.shakeTilt, 0.0f, tile.spin * 0.6f - tile.shakeTilt * 0.7f });
		tile.model->SetTranslate(tile.position + tile.shake);
		tile.model->TransferMatrix();
	}

	//空と床を描く
	void ShowcaseStage::Draw()
	{
		sky_.Draw();
		for (Tile& tile : tiles_)
		{
			tile.model->Draw();
		}
	}
}
