#include "Tsumiki.h"
#include "../../Config/ModelConfig.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App::Tsumiki
{
	//木目の積み木のモデルを作る
	std::unique_ptr<Model> Create()
	{
		return Model::Create(ModelConfig::kTsumikiModel);
	}

	//まん中の位置と大きさと度の回転から行列の値を決めて、原点が下にずれているモデルをまん中で回す
	void Place(Model& model, const Vector3& center, const Vector3& size, const Vector3& rotateDeg)
	{
		//モデルの1辺はcube.objと同じなので大きさの比をかける
		const Vector3 scale = {
			size.x / ModelConfig::kCubeModelSize,
			size.y / ModelConfig::kCubeModelSize,
			size.z / ModelConfig::kCubeModelSize };

		//原点からまん中までの上向きの線を、X→Y→Zの順に回して引く
		const float toRad = std::numbers::pi_v<float> / 180.0f;
		const float cx = std::cos(rotateDeg.x * toRad);
		const float sx = std::sin(rotateDeg.x * toRad);
		const float cy = std::cos(rotateDeg.y * toRad);
		const float sy = std::sin(rotateDeg.y * toRad);
		const float cz = std::cos(rotateDeg.z * toRad);
		const float sz = std::sin(rotateDeg.z * toRad);
		const float up = ModelConfig::kTsumikiCenterY * scale.y;
		const Vector3 afterX = { 0.0f, up * cx, up * sx };
		const Vector3 afterY = { afterX.z * sy, afterX.y, afterX.z * cy };
		const Vector3 afterZ = { afterY.x * cz - afterY.y * sz, afterY.x * sz + afterY.y * cz, afterY.z };

		model.SetScale(scale);
		model.SetRotate(rotateDeg);
		model.SetTranslate({ center.x - afterZ.x, center.y - afterZ.y, center.z - afterZ.z });
	}
}
