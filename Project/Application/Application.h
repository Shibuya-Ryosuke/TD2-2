#pragma once
#include <RyoEngine.h>

namespace App
{
	//アプリ本体で、保存データの読み込みと最初のシーンの予約をする
	class Application : public RyoEngine::Framework
	{
	public:
		Application() : RyoEngine::Framework(L"Application") {}
		~Application() override = default;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

	protected:
		void Initialize() override;
		void Finalize() override;
	};
}
