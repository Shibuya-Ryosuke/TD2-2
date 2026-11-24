#include "TransitionPiece.h"
#include "TransitionView.h"
#include "../../Config/GameConfig.h"
#include "../../Config/DirectionConfig.h"

using namespace RyoEngine;

namespace App
{
	//モデルを作る
	void TransitionPiece::Initialize(WorldTransform* frame, uint32_t shader)
	{
		model_ = Model::Create(GameConfig::kCubeModel);
		model_->SetParent(frame);
		model_->SetShader(shader);
		model_->SetCastShadow(false);
	}

	//行列を確定する
	void TransitionPiece::Apply(const TransitionView& view)
	{
		//カメラに近すぎるブロックは描かない
		if (position.z < DirectionConfig::kTransitionNearHide)
		{
			isVisible = false;
		}
		if (!isVisible)
		{
			return;
		}

		//1辺2のcube.objに大きさを合わせ、平行投影では近いほど大きくする
		const float scale = view.GetScaleAt(position.z) / GameConfig::kCubeModelSize;
		model_->SetScale({ size.x * scale, size.y * scale, size.z * scale });
		model_->SetRotate(rotate);
		model_->SetTranslate(view.ToLocalPosition(position));

		//持ち主が決めたシェーダーの値を渡す
		model_->SetShaderParams(0, shaderParams0);
		model_->SetShaderParams(1, shaderParams1);
		model_->TransferMatrix();
	}

	//モデルの色
	void TransitionPiece::SetColor(const Vector4& color)
	{
		model_->SetColor(color);
	}

	//見えているときだけ描く
	void TransitionPiece::Draw()
	{
		if (isVisible)
		{
			model_->Draw();
		}
	}
}
