#pragma once
#include <RyoEngine.h>

namespace App
{
	class Floor;
	class Player;
	class Boss;
	class StageCamera;

	// ボスが下の層へ落ちて、プレイヤーとカメラも下の層へ降りる流れ (Game から分けたもの)
	// ・ボス: 切り離された床と一緒に下の層まで落ちる
	// ・プレイヤー: 跳ねながら下の層の最初の位置へ (マスの線にそって)
	// ・カメラ: 下の層の高さまで下がる
	class DescentSequence
	{
	public:
		// このフレームで起きたこと
		struct Result
		{
			bool isBossLandedNow = false; // ボスが下の層に着いた (落下ダメージを入れる)
			bool isFinished = false;      // ボスもプレイヤーも着いた (層を1つ進める)
		};

		// 降り始める
		// bossPosition: ボスが落ち始めた位置 / playerStart: プレイヤーの今の位置 / playerEnd: プレイヤーが降りる先
		void Begin(const RyoEngine::Vector3& bossPosition, const RyoEngine::Vector3& playerStart, const RyoEngine::Vector3& playerEnd);

		// 1フレーム進める
		// current: 今の層 (ボスと一緒に床が落ちる) / below: 下の層
		Result Advance(float deltaTime, Floor& current, const Floor& below, Player& player, Boss& boss, StageCamera& camera);

	private:
		RyoEngine::Vector3 dropOrigin_{};   // ボスが落ち始めた位置
		RyoEngine::Vector3 descentStart_{}; // プレイヤーが降り始めた位置
		RyoEngine::Vector3 descentEnd_{};   // プレイヤーが降りる先
		bool isBossLanded_ = false;
		float timer_ = 0.0f;
	};
}
