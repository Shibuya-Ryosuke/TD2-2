#include "DefeatSequence.h"
#include "DefeatPoses.h"
#include "SpawnPlanner.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>; //ラジアンから度へ
		constexpr float kDegToRad = std::numbers::pi_v<float> / 180.0f; //度からラジアンへ
		constexpr float kZoomEnd = DirectionConfig::kDefeatZoomTime; //寄りきる秒
		constexpr float kFlightEnd = kZoomEnd + DirectionConfig::kDefeatFlightTime; //吹っ飛び終わる秒
		constexpr float kImpact = kFlightEnd + DirectionConfig::kDefeatTipTime; //倒れきって床に当たる秒
		constexpr float kSecondBounce = kImpact + DirectionConfig::kDefeatBounceTime; //2回目に跳ね始める秒
		constexpr float kRest = kSecondBounce + DirectionConfig::kDefeatSecondBounceTime; //止まる秒
		constexpr float kEnd = kRest + DirectionConfig::kDefeatHoldTime; //終わる秒

		//startからendまでの進み具合
		float Progress(float time, float start, float end)
		{
			return std::clamp((time - start) / (end - start), 0.0f, 1.0f);
		}

		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			return t * t * (3.0f - 2.0f * t);
		}

		//0から1へ進む間に一度だけ山なりに上がる高さ
		float Arc(float t)
		{
			return 4.0f * t * (1.0f - t);
		}

		//床のない所でHPが尽きたら近くの見える足場から演出を始める
		const Floor& ChooseFloor(const Floor& floor, const Floor* below, const Vector3& position, Vector3& start)
		{
			const Floor* support = &floor;
			start = position;
			if (!floor.IsSolid(floor.GetCellAt(position)))
			{
				start = SpawnPlanner::FindNearestStand(floor, position);
				if (!floor.IsSolid(floor.GetCellAt(start)) && below != nullptr)
				{
					support = below;
					start = SpawnPlanner::FindNearestStand(*below, position);
				}
			}
			start.y = std::max(start.y, support->GetHeight());
			return *support;
		}

		//攻撃された方から離れるマスを優先し、穴や外周では別の足場を選ぶ
		Vector3 ChooseLanding(const Floor& floor, const Vector3& start, const Cell& away)
		{
			const Cell cell = floor.GetCellAt(start);
			const std::array<Cell, 4> directions = {{
				away, { -away.z, away.x }, { away.z, -away.x }, { -away.x, -away.z }
			}};
			for (const Cell& step : directions)
			{
				const Cell candidate = cell + step;
				if (floor.IsSolid(candidate))
				{
					return floor.GetCellCenter(candidate);
				}
			}
			return { start.x, floor.GetHeight(), start.z };
		}

		//yawだけ回り込んだときの水平な正面の向きと吹っ飛ぶ向きがどれだけそろっているかで、0なら直角
		float Alignment(const StageCamera& camera, float yaw, const Vector3& travel, float length)
		{
			const Vector3 forward = camera.GetOrbitForward(yaw);
			const float flat = std::sqrt(forward.x * forward.x + forward.z * forward.z);
			if (flat < 0.001f)
			{
				return 0.0f;
			}
			return std::abs(forward.x * travel.x + forward.z * travel.z) / (flat * length);
		}

		//yawだけ回り込んだときに、カメラの水平な向きがマスの線の向きから何度ずれているかで、45がいちばん斜め
		float AxisOffsetDeg(const StageCamera& camera, float yaw)
		{
			const Vector3 forward = camera.GetOrbitForward(yaw);
			const float headingDeg = std::abs(std::atan2(forward.x, forward.z) * kRadToDeg);
			const float offset = std::fmod(headingDeg, 90.0f);
			return std::min(offset, 90.0f - offset);
		}

		//タイルのすき間がまっすぐ見えない向きの中から、吹っ飛ぶ向きができるだけ横から見える方へ回り込む度数を選ぶ
		float ChooseOrbit(const StageCamera& camera, const Vector3& travel)
		{
			const float orbit = DirectionConfig::kDefeatOrbitDeg * kDegToRad;
			const bool isPlusSafe = AxisOffsetDeg(camera, orbit) >= DirectionConfig::kDefeatOrbitAxisMargin;
			const bool isMinusSafe = AxisOffsetDeg(camera, -orbit) >= DirectionConfig::kDefeatOrbitAxisMargin;
			//片方だけすき間が見えない向きならそちらにする
			if (isPlusSafe && !isMinusSafe)
			{
				return orbit;
			}
			if (isMinusSafe && !isPlusSafe)
			{
				return -orbit;
			}
			//どちらも見えない、またはどちらも見えるときは、どちらが横から見えるかで選ぶ
			const float length = std::sqrt(travel.x * travel.x + travel.z * travel.z);
			if (length < 0.001f)
			{
				return orbit;
			}
			if (Alignment(camera, -orbit, travel, length) < Alignment(camera, orbit, travel, length))
			{
				return -orbit;
			}
			return orbit;
		}
	}

	//時を止めて、吹っ飛ぶ先とカメラの回り込む向きを決める
	void DefeatSequence::Begin(const Floor& floor, const Floor* below, StageCamera& camera, Player& player, Boss& boss)
	{
		const Vector3& threat = boss.GetPosition();
		elapsed_ = 0.0f;
		startFocus_ = camera.GetFocusOffset();
		startSize_ = camera.GetOrthoSize();
		const Vector3 position = player.GetPosition();
		const Floor& support = ChooseFloor(floor, below, position, from_);

		const Vector3 separation = position - threat;
		Cell away = GridDirection::FromVector(separation);
		if (separation.x * separation.x + separation.z * separation.z < 0.001f)
		{
			const Cell& facing = player.GetFacing();
			away = { -facing.x, -facing.z };
		}
		landing_ = ChooseLanding(support, from_, away);
		Vector3 travel = landing_ - from_;
		if (travel.x * travel.x + travel.z * travel.z < 0.001f)
		{
			travel = support.StepOffset(away);
		}
		yaw_ = std::atan2(-travel.x, -travel.z) * kRadToDeg;
		orbitYaw_ = ChooseOrbit(camera, travel);
		bossFade_.Reset(boss);

		//エンジンが自動で進める骨格やパーティクルも止める
		previousTimeScale_ = GetTimeScale();
		SetTimeScale(0.0f);
		Update(0.0f, camera, player, boss);
	}

	//寄って回り込み、吹っ飛んで倒れて二度跳ね、終わったらtrue
	bool DefeatSequence::Update(float deltaTime, StageCamera& camera, Player& player, Boss& boss)
	{
		const float step = std::clamp(deltaTime, 0.0f, DirectionConfig::kDefeatMaxDeltaTime);
		elapsed_ = std::min(kEnd, elapsed_ + step);
		const float zoom = Smooth(Progress(elapsed_, 0.0f, kZoomEnd));
		const float flight = Progress(elapsed_, kZoomEnd, kFlightEnd);
		const float tip = Progress(elapsed_, kFlightEnd, kImpact);
		const float travel = 1.0f - (1.0f - flight) * (1.0f - flight);
		Vector3 position = from_ + (landing_ - from_) * travel;

		//寄りながら身構えて、山なりに吹っ飛ぶ
		const RigPose flinch = DefeatPoses::FlinchPose(flight);
		position.y += DirectionConfig::kDefeatHopHeight * Arc(flight);
		RigPose pose = LerpPose(RigPose{}, flinch, zoom);
		if (elapsed_ >= kFlightEnd)
		{
			pose = LerpPose(flinch, DefeatPoses::DownPose(), tip * tip);
			//着地したら少し跳ね返り、次は小さく跳ねて止まる
			const float first = Arc(Progress(elapsed_, kImpact, kSecondBounce));
			const float second = Arc(Progress(elapsed_, kSecondBounce, kRest));
			position.y += DirectionConfig::kDefeatBounceHeight * first + DirectionConfig::kDefeatSecondBounceHeight * second;
			pose.bodyTilt -= DirectionConfig::kDefeatBounceTilt * first + DirectionConfig::kDefeatSecondBounceTilt * second;
		}
		player.ApplyDefeatPose(pose, position, yaw_);

		//胸を画面の真ん中へ寄せながら、その点を中心に斜め横へ回り込む
		const float focusHeight = std::lerp(DirectionConfig::kDefeatFocusHeight, DirectionConfig::kDefeatDownFocusHeight, tip);
		const Vector3 target = position + Vector3{ 0.0f, focusHeight, 0.0f };
		const Vector3 centered = camera.GetCenteringOffset(target);
		camera.SetFocusOffset(startFocus_ + (centered - startFocus_) * zoom);
		camera.SetOrthoSize(std::lerp(startSize_, DirectionConfig::kDefeatOrthoSize, zoom));
		camera.SetOrbit(orbitYaw_ * zoom, target);

		//寄っている間にボスが画面の上で重なったらすーっと透かす
		bossFade_.Update(step, elapsed_ <= kZoomEnd, camera, position, boss);
		return elapsed_ >= kEnd;
	}

	//止めた時の速さを元に戻す
	void DefeatSequence::RestoreTime()
	{
		if (previousTimeScale_.has_value())
		{
			SetTimeScale(*previousTimeScale_);
			previousTimeScale_.reset();
		}
	}
}
