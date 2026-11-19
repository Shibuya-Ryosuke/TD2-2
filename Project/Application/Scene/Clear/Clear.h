#pragma once
#include <RyoEngine.h>

namespace App
{
	// クリアシーン (仮。文字はデバッグ表示で、Debug のときだけ出る)
	// Enter / A で タイトル へ
	class Clear : public RyoEngine::BaseScene
	{
	public:
		Clear() = default;
		~Clear() override = default;

		Clear(const Clear&) = delete;
		Clear& operator=(const Clear&) = delete;

		void Initialize() override; // シーンに来たとき
		void Update() override; // Enter / A でタイトルへ
		void Draw() override; // 文字を出す
	};
}
