#include "TransitionPiece.h"
#include "TransitionView.h"
#include "../Config/GameConfig.h"

using namespace RyoEngine;

namespace App
{
	// モデルを作る
	void TransitionPiece::Initialize(WorldTransform* frame, uint32_t shader)
	{
		model_ = Model::Create(GameConfig::kCubeModel);
		model_->SetParent(frame);
		model_->SetShader(shader);
		model_->SetCastShadow(false);
	}

	// 行列を確定する
	void TransitionPiece::Apply(const TransitionView& view)
	{
		// カメラに近すぎる (通り過ぎた) ブロックは描かない
		if (position.z < GameConfig::kTransitionNearHide)
		{
			isVisible = false;
		}
		if (!isVisible)
		{
			return;
		}

		// cube.obj は1辺 2 なので、半分にして大きさを合わせる。平行投影のときは近いほど大きくする
		const float scale = view.GetScaleAt(position.z) / GameConfig::kCubeModelSize;
		model_->SetScale({ size.x * scale, size.y * scale, size.z * scale });
		model_->SetRotate(rotate);
		model_->SetTranslate(view.ToLocalPosition(position));

		// シェーダーに渡す値 (0番: 縁の光 / 1番: 風の揺れ / 2番: 揺れ方をずらす値 / 3番: 木目を消す / 4〜6番: 縁の光の色)
		model_->SetShaderParams(0, { glow, wind, seed, flatColor });
		model_->SetShaderParams(1, { glowColor.x, glowColor.y, glowColor.z, 0.0f });
		model_->TransferMatrix();
	}

	// モデルの色
	void TransitionPiece::SetColor(const Vector4& color)
	{
		model_->SetColor(color);
	}

	// 見えているときだけ描く
	void TransitionPiece::Draw()
	{
		if (isVisible)
		{
			model_->Draw();
		}
	}
}
