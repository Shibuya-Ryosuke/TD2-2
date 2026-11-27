#include "TransitionPiece.h"
#include "TransitionView.h"
#include "../../Config/ModelConfig.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/Tsumiki/Tsumiki.h"

using namespace RyoEngine;

namespace App
{
	//カメラを親にして、積み木なら木目のTsumikiで、紙吹雪なら仮の箱でモデルを作る
	void TransitionPiece::Initialize(WorldTransform* frame, uint32_t shader, bool isTsumiki)
	{
		isTsumiki_ = isTsumiki;
		if (isTsumiki)
		{
			model_ = Tsumiki::Create();
		}
		else
		{
			model_ = Model::Create(ModelConfig::kCubeModel);
		}
		model_->SetParent(frame);
		model_->SetShader(shader);
		model_->SetCastShadow(false);
		model_->SetEnableLighting(false);
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

		//1辺2のモデルに大きさを合わせ、平行投影では近いほど大きくする
		const float viewScale = view.GetScaleAt(position.z);
		if (isTsumiki_)
		{
			Tsumiki::Place(*model_, view.ToLocalPosition(position), size * viewScale, rotate);
		}
		else
		{
			const float scale = viewScale / ModelConfig::kCubeModelSize;
			model_->SetScale({ size.x * scale, size.y * scale, size.z * scale });
			model_->SetRotate(rotate);
			model_->SetTranslate(view.ToLocalPosition(position));
		}

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
