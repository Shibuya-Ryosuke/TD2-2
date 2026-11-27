#pragma once
#include <RyoEngine.h>
#include <optional>
#include "DefeatBossFade.h"

namespace App
{
	class Boss;
	class Floor;
	class Player;
	class StageCamera;

	//時を止めてプレイヤーが吹っ飛び、倒れてから切り替える
	class DefeatSequence
	{
	public:
		//時を止めて、吹っ飛ぶ先とカメラの回り込む向きを決める
		void Begin(const Floor& floor, const Floor* below, StageCamera& camera, Player& player, Boss& boss);
		//寄って回り込み、吹っ飛んで倒れて二度跳ね、終わったらtrue
		bool Update(float deltaTime, StageCamera& camera, Player& player, Boss& boss);
		//止めた時の速さを元に戻す
		void RestoreTime();

	private:
		RyoEngine::Vector3 from_{}; //吹っ飛び始める足元の位置
		RyoEngine::Vector3 landing_{}; //倒れる足元の位置
		RyoEngine::Vector3 startFocus_{}; //寄り始める前に映す所をずらしていた分
		float startSize_ = 0.0f; //寄り始める前の映す広さ
		float yaw_ = 0.0f; //吹っ飛ぶ向きの度数
		float orbitYaw_ = 0.0f; //寄りきったときにカメラが回り込むラジアン
		float elapsed_ = 0.0f; //始めてからの秒数
		std::optional<float> previousTimeScale_; //止める前の時の速さ
		DefeatBossFade bossFade_; //手前で重なったボスを透かす
	};
}
