#include "FloorCrumble.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/DirectionConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//タイルの数に合わせて用意する
	void FloorCrumble::Initialize(size_t tileCount)
	{
		wasShattering_.assign(tileCount, 0);
		shards_.Initialize();
	}

	//シェーダーに値を渡して砕け始めなら欠片を飛ばす
	void FloorCrumble::Apply(size_t index, Model& model, const State& state, const Floor& floor, const Cell& cell)
	{
		const FloorTile& tile = floor.GetTile(cell);

		//見えている床が砕け始めた瞬間にピンクの欠片を飛ばす
		const bool isShattering = state.shatter > 0.0f;
		if (isShattering && wasShattering_[index] == 0 && tile.isVisible)
		{
			shards_.Burst(tile.position, floor.GetTileSizeX());
		}
		wasShattering_[index] = static_cast<uint8_t>(isShattering);

		//シェーダーの0から7番に演出の値とハートの向きを渡す
		const float seed = static_cast<float>(cell.x) * 7.13f + static_cast<float>(cell.z) * 3.71f;
		//値が足りないので4番は砕けているときは砕けた割合、そうでなければマイナスにした細いヒビの濃さ
		float shatterOrWear = -state.wear;
		if (isShattering)
		{
			shatterOrWear = state.shatter;
		}
		model.SetShaderParams(0, { state.heartScale, state.heartCrack, state.heartBreak, state.tileCrack });
		model.SetShaderParams(1, { shatterOrWear, seed, state.flash, tile.heartAngle });
		//影は砕けないので砕けかけの床は影を落とさない
		model.SetCastShadow(state.shatter <= 0.0f);
	}

	//飛び散る欠片を動かす
	void FloorCrumble::Update()
	{
		shards_.Update();
	}

	//飛び散る欠片を描く
	void FloorCrumble::Draw()
	{
		shards_.Draw();
	}

	//今の崩れる演出の値
	FloorCrumble::State FloorCrumble::Compute(const Floor& floor, const Cell& cell) const
	{
		State effect{};
		const FloorTile& tile = floor.GetTile(cell);
		const float pi = std::numbers::pi_v<float>;

		//切り離された床は揺れている間にハートが割れて落ちながら砕ける
		if (tile.isIsolated)
		{
			float progress = std::clamp(tile.shakeRate, 0.0f, 1.0f);
			if (!tile.isSolid)
			{
				progress = 1.0f;
			}
			ApplyHammer(effect, progress, FloorConfig::kEscapeDelay);
			const float shatterStart = DirectionConfig::kIsolatedDissolveStart;
			effect.shatter = std::clamp((tile.dropRate - shatterStart) / (1.0f - shatterStart), 0.0f, 1.0f);
			return effect;
		}

		//穴になって落ちている床はぽんっとふくらんでから砕ける
		if (!tile.isSolid)
		{
			const float drop = std::clamp(tile.dropRate, 0.0f, 1.0f);
			ApplyHammer(effect, 1.0f, FloorConfig::kCollapseDelay);
			effect.shatter = drop;
			const float pop = std::clamp(drop / DirectionConfig::kDropPopTime, 0.0f, 1.0f);
			effect.scale = 1.0f + std::sin(pi * pop) * DirectionConfig::kDropPop;
			return effect;
		}

		//崩す予定の床はハンマーの演出をして直前にガタガタぷくっ
		if (tile.remaining >= 0.0f && floor.IsForeground())
		{
			ApplyHammer(effect, std::clamp(1.0f - tile.remaining / FloorConfig::kCollapseDelay, 0.0f, 1.0f), FloorConfig::kCollapseDelay);
			if (tile.remaining < DirectionConfig::kCrumbleShakeTime)
			{
				const float shake = 1.0f - tile.remaining / DirectionConfig::kCrumbleShakeTime; //だんだん強くなる揺れの割合
				const float time = tile.remaining * DirectionConfig::kCrumbleShakeSpeed + static_cast<float>(cell.x * 3 + cell.z * 5);
				const float distance = DirectionConfig::kCrumbleShakeDistance * shake;
				effect.shake = { std::sin(time) * distance, effect.shake.y, std::cos(time * 1.3f) * distance };
				effect.scale = 1.0f + DirectionConfig::kCrumblePuff * shake * shake;
			}
		}
		return effect;
	}

	//ハートがポンっと出てガン！でヒビ、ガン！でぱっかーん
	void FloorCrumble::ApplyHammer(State& effect, float progress, float duration)
	{
		const float time = progress * duration; //崩し始めてからの秒数

		//ハートがポンっと大きく出てぽよんと落ち着く
		const float pop = std::clamp(time / DirectionConfig::kHeartPopTime, 0.0f, 1.0f);
		effect.heartScale = 0.0f;
		if (progress > 0.0f)
		{
			effect.heartScale = 1.0f + std::sin(pop * std::numbers::pi_v<float> * 2.5f) * (1.0f - pop) * 0.35f;
			effect.heartScale = std::max(effect.heartScale * std::min(pop * 4.0f, 1.0f), 0.0f);
		}

		//ガン！からの秒数でまだならマイナス
		const float sinceCrack = time - DirectionConfig::kHammerHits[0] * duration;
		const float sinceBreak = time - DirectionConfig::kHammerHits[1] * duration;
		effect.heartCrack = std::clamp(sinceCrack / DirectionConfig::kHeartCrackTime, 0.0f, 1.0f);
		effect.heartBreak = std::clamp(sinceBreak / DirectionConfig::kHeartBreakTime, 0.0f, 1.0f);
		effect.tileCrack = std::clamp(sinceBreak / DirectionConfig::kTileCrackTime, 0.0f, 1.0f);

		//叩いた瞬間がいちばん強いガン！の光とビクッ
		for (const float since : { sinceCrack, sinceBreak })
		{
			if (since >= 0.0f)
			{
				effect.flash = std::max(effect.flash, std::exp(-since * DirectionConfig::kHammerFlashDecay));
			}
		}
		effect.shake.y = -DirectionConfig::kHammerJolt * effect.flash;
	}
}
