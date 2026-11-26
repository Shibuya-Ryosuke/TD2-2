#pragma once
#include <RyoEngine.h>
#include <cstdint>

//仮のモデルと積み木のモデル
namespace App::ModelConfig
{
	//仮のモデル
	inline constexpr const char* kCubeModel = "cube"; //Kari/Model/cube.obj
	inline constexpr float kCubeModelSize = 2.0f; //cube.objの1辺の長さ

	//足場のほかの積み木
	inline constexpr const char* kTsumikiModel = "Model/Tsumiki/Tsumiki.obj"; //木目の積み木で1辺はcube.objと同じ
	inline constexpr float kTsumikiCenterY = 0.92f; //原点から積み木のまん中までの高さで、回すときはまん中を中心にする
}
