#pragma once
#include <RyoEngine.h>
#include <memory>

//足場のほかの積み木に使う木目のTsumikiのモデル
namespace App::Tsumiki
{
	//木目の積み木のモデルを作る
	std::unique_ptr<RyoEngine::Model> Create();
	//まん中の位置と大きさと度の回転から行列の値を決めて、原点が下にずれているモデルをまん中で回す
	void Place(RyoEngine::Model& model, const RyoEngine::Vector3& center, const RyoEngine::Vector3& size, const RyoEngine::Vector3& rotateDeg);
}
