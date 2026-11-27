#include "BossFallParticles.h"
#include "../../Config/BossFallConfig.h"

using namespace RyoEngine;

namespace App::BossFallParticles
{
	//エディターで保存した設定を残して、未登録の効果に初期値を入れる
	void Initialize()
	{
		using namespace BossFallConfig;

		//床にそって輪の形に外へ広がる、まぶしくない淡い煙
		ParticleConfig puff;
		puff.maxParticles = kSlamPuffMaxParticles;
		puff.burstCount = kSlamPuffCount;
		puff.spawnShape = ParticleSpawnShape::Ring;
		puff.radius = kSlamPuffRadius;
		puff.innerRatio = kSlamPuffInnerRatio;
		puff.velocityMode = ParticleVelocityMode::Radial;
		puff.speedMin = kSlamPuffSpeedMin;
		puff.speedMax = kSlamPuffSpeedMax;
		puff.drag = kSlamPuffDrag;
		puff.lifeMin = kSlamPuffLifeMin;
		puff.lifeMax = kSlamPuffLifeMax;
		puff.scaleMin = kSlamPuffScaleMin;
		puff.scaleMax = kSlamPuffScaleMax;
		puff.startScale = kSlamPuffStartScale;
		puff.endScale = kSlamPuffEndScale;
		puff.look = ParticleLook::SoftCircle;
		puff.blend = ParticleBlend::Alpha;
		puff.colorA = kSlamPuffColorA;
		puff.colorB = kSlamPuffColorB;
		puff.intensity = kSlamPuffIntensity;
		if (!ParticleManager::Contains(kSlamPuffName))
		{
			ParticleManager::Create(kSlamPuffName, puff);
		}

		//床にそって一気に外へ走る光の粒で、輪に見せる
		ParticleConfig ring;
		ring.maxParticles = kHammerRingMaxParticles;
		ring.burstCount = kHammerRingCount;
		ring.spawnShape = ParticleSpawnShape::Ring;
		ring.radius = kHammerRingRadius;
		ring.innerRatio = 1.0f;
		ring.velocityMode = ParticleVelocityMode::Radial;
		ring.speedMin = kHammerRingSpeedMin;
		ring.speedMax = kHammerRingSpeedMax;
		ring.drag = kHammerRingDrag;
		ring.lifeMin = kHammerRingLifeMin;
		ring.lifeMax = kHammerRingLifeMax;
		ring.scaleMin = kHammerRingScaleMin;
		ring.scaleMax = kHammerRingScaleMax;
		ring.endScale = kHammerRingEndScale;
		ring.look = ParticleLook::SoftCircle;
		ring.blend = ParticleBlend::Additive;
		ring.colorA = kHammerRingColorA;
		ring.colorB = kHammerRingColorB;
		ring.intensity = kHammerRingIntensity;
		if (!ParticleManager::Contains(kHammerRingName))
		{
			ParticleManager::Create(kHammerRingName, ring);
		}
	}

	//ボスの足元からピンクの煙を外へ広げる
	void SlamPuff(const Vector3& feet)
	{
		ParticleManager::Play(BossFallConfig::kSlamPuffName, feet + Vector3{ 0.0f, BossFallConfig::kParticleLift, 0.0f });
	}

	//ハンマーが当たった所から光の輪を広げる
	void HammerRing(const Vector3& contact)
	{
		ParticleManager::Play(BossFallConfig::kHammerRingName, contact + Vector3{ 0.0f, BossFallConfig::kParticleLift, 0.0f });
	}
}
