#pragma once
#include <RyoEngine.h>

namespace App
{
	// タイトルシーン (仮。文字はデバッグ表示で、Debug のときだけ出る)
	// Enter / A で ゲームシーン へ
	class Title : public RyoEngine::BaseScene
	{
	public:
		Title() = default;
		~Title() override = default;

		Title(const Title&) = delete;
		Title& operator=(const Title&) = delete;

		void Initialize() override; // シーンに来たとき
		void Update() override; // Enter / A でゲームへ
		void Draw() override; // 文字を出す
	};
}
