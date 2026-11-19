#pragma once
#include <RyoEngine.h>

namespace App {
	/// <summary>
	/// ゲームオーバーシーン (仮。文字はデバッグ表示で、Debug のときだけ出る)
	/// Enter / A で タイトル へ
	/// </summary>
	class GameOver : public RyoEngine::BaseScene {
	public:
		GameOver() = default;
		~GameOver() override = default;

		GameOver(const GameOver&) = delete;
		GameOver& operator=(const GameOver&) = delete;

		void Initialize() override; // シーンに来たとき
		void Update() override; // Enter / A でタイトルへ
		void Draw() override; // 文字を出す
	};
}
