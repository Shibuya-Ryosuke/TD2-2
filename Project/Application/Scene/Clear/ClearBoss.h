#pragma once
#include <RyoEngine.h>
#include <array>
#include <memory>
#include <random>

namespace App
{
	//クリアで仰向けに倒れて目を回しているボスで、ハンマーでたたかれるとパーツごとにバラバラに吹っ飛ぶ
	class ClearBoss
	{
	public:
		ClearBoss() = default;
		~ClearBoss() = default;

		//モデルを持つのでコピーを禁止する
		ClearBoss(const ClearBoss&) = delete;
		ClearBoss& operator=(const ClearBoss&) = delete;

		void Initialize(); //6つのパーツを読みこんで仰向けに寝かせる
		void Update(float deltaTime); //倒れてふらふらか、バラバラに飛ぶのを1フレーム進める
		void Draw();

		//impactから外へパーツを吹っ飛ばす
		void Scatter(const RyoEngine::Vector3& impact);
		bool IsScattered() const { return isScattered_; }

	private:
		static constexpr size_t kPartCount = 6;

		//1つのパーツ
		struct Part
		{
			//モデルと親子のつながりを持つのでコピーを禁止する
			Part() = default;
			~Part() = default;
			Part(const Part&) = delete;
			Part& operator=(const Part&) = delete;

			RyoEngine::WorldTransform holder; //パーツのまん中にあって回す親
			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 center{}; //モデルの中のまん中
			RyoEngine::Vector3 position{}; //飛んでいるときのワールドの位置
			RyoEngine::Vector3 rotate{}; //飛んでいるときの度の回転
			RyoEngine::Vector3 velocity{};
			RyoEngine::Vector3 spin{}; //1秒に回る度
			float radius = 0.4f; //床ではねる高さ
		};

		RyoEngine::WorldTransform root_; //倒れているボスの足元で仰向けの向き
		std::array<Part, kPartCount> parts_;
		RyoEngine::Vector3 rootRotate_{}; //仰向けの度の回転
		std::mt19937 random_{ 5u };
		float timer_ = 0.0f;
		bool isScattered_ = false;
	};
}
