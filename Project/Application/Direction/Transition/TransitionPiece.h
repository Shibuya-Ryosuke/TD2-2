#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	struct TransitionView;

	//積み木と紙吹雪に使うブロック1個
	class TransitionPiece
	{
	public:
		TransitionPiece() = default;
		~TransitionPiece() = default;

		TransitionPiece(const TransitionPiece&) = delete;
		TransitionPiece& operator=(const TransitionPiece&) = delete;

		//カメラを親にして、積み木なら木目のTsumikiで、紙吹雪なら仮の箱でモデルを作る
		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader, bool isTsumiki);

		//viewに合わせて行列を確定する
		void Apply(const TransitionView& view);
		void Draw();
		//モデルの色
		void SetColor(const RyoEngine::Vector4& color);

		//カメラから見た空間で動かす値
		RyoEngine::Vector3 position{}; //zがカメラからの奥行きの位置
		RyoEngine::Vector3 rotate{}; //回転
		RyoEngine::Vector3 size = { 1.0f, 1.0f, 1.0f }; //箱の幅と高さと奥行き
		RyoEngine::Vector4 shaderParams0{}; //シェーダーに渡す0から3番の値
		RyoEngine::Vector4 shaderParams1{}; //シェーダーに渡す4から7番の値
		bool isVisible = false; //描くか

	private:
		std::unique_ptr<RyoEngine::Model> model_{};
		bool isTsumiki_ = false; //木目のTsumikiのモデルか
	};
}
