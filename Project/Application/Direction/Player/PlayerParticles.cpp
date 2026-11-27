#include "PlayerParticles.h"
#include "../../Config/DirectionConfig.h"

using namespace RyoEngine;

namespace App::PlayerParticles
{
	//エディターで保存した設定を残して未登録の効果に初期値を入れる
	void Initialize()
	{
		ParticleConfig landing;
		landing.maxParticles = DirectionConfig::kLandingMaxParticles;
		landing.burstCount = DirectionConfig::kLandingSparkCount;
		landing.spawnShape = ParticleSpawnShape::Ring;
		landing.radius = DirectionConfig::kLandingRadius;
		landing.innerRatio = DirectionConfig::kLandingInnerRatio;
		landing.velocityMin = DirectionConfig::kLandingVelocityMin;
		landing.velocityMax = DirectionConfig::kLandingVelocityMax;
		landing.gravity = DirectionConfig::kLandingGravity;
		landing.drag = DirectionConfig::kLandingDrag;
		landing.lifeMin = DirectionConfig::kLandingLifeMin;
		landing.lifeMax = DirectionConfig::kLandingLifeMax;
		landing.scaleMin = DirectionConfig::kLandingScaleMin;
		landing.scaleMax = DirectionConfig::kLandingScaleMax;
		landing.endScale = DirectionConfig::kLandingEndScale;
		landing.look = ParticleLook::Spark;
		landing.blend = ParticleBlend::Additive;
		landing.colorA = DirectionConfig::kLandingColorA;
		landing.colorB = DirectionConfig::kLandingColorB;
		landing.intensity = DirectionConfig::kLandingIntensity;
		if (!ParticleManager::Contains(DirectionConfig::kLandingSparkName))
		{
			ParticleManager::Create(DirectionConfig::kLandingSparkName, landing);
		}

		//丸い光を短く重ねて火花の周りをふんわり光らせる
		ParticleConfig landingGlow = landing;
		landingGlow.burstCount = DirectionConfig::kLandingGlowCount;
		landingGlow.look = ParticleLook::SoftCircle;
		landingGlow.scaleMin = DirectionConfig::kLandingGlowScaleMin;
		landingGlow.scaleMax = DirectionConfig::kLandingGlowScaleMax;
		landingGlow.lifeMin = DirectionConfig::kLandingGlowLifeMin;
		landingGlow.lifeMax = DirectionConfig::kLandingGlowLifeMax;
		landingGlow.colorA.w = DirectionConfig::kLandingGlowAlphaA;
		landingGlow.colorB.w = DirectionConfig::kLandingGlowAlphaB;
		if (!ParticleManager::Contains(DirectionConfig::kLandingGlowName))
		{
			ParticleManager::Create(DirectionConfig::kLandingGlowName, landingGlow);
		}

		//肩の外側に小さな白い星を散らして点滅させる
		ParticleConfig victory;
		victory.maxParticles = DirectionConfig::kVictoryStarMaxParticles;
		victory.burstCount = DirectionConfig::kVictoryStarCount;
		victory.spawnShape = ParticleSpawnShape::Ring;
		victory.axisFacesCamera = true;
		victory.radius = DirectionConfig::kVictoryStarRadius * DirectionConfig::kVictoryParticleScale;
		victory.innerRatio = DirectionConfig::kVictoryStarInnerRatio;
		victory.velocityMode = ParticleVelocityMode::Radial;
		victory.speedMin = DirectionConfig::kVictoryStarSpeedMin;
		victory.speedMax = DirectionConfig::kVictoryStarSpeedMax;
		victory.drag = DirectionConfig::kVictoryStarDrag;
		victory.lifeMin = DirectionConfig::kVictoryStarLifeMin;
		victory.lifeMax = DirectionConfig::kVictoryStarLifeMax;
		victory.scaleMin = DirectionConfig::kVictoryStarScaleMin * DirectionConfig::kVictoryParticleScale;
		victory.scaleMax = DirectionConfig::kVictoryStarScaleMax * DirectionConfig::kVictoryParticleScale;
		victory.endScale = DirectionConfig::kVictoryStarEndScale;
		victory.rotationSpeedMin = -DirectionConfig::kVictoryStarSpin;
		victory.rotationSpeedMax = DirectionConfig::kVictoryStarSpin;
		victory.look = ParticleLook::Star;
		victory.blend = ParticleBlend::Additive;
		victory.death = ParticleDeath::Blink;
		victory.colorA = DirectionConfig::kVictoryStarColorA;
		victory.colorB = DirectionConfig::kVictoryStarColorB;
		victory.intensity = DirectionConfig::kVictoryStarIntensity;
		if (!ParticleManager::Contains(DirectionConfig::kVictoryStarName))
		{
			ParticleManager::Create(DirectionConfig::kVictoryStarName, victory);
		}
	}

	//床に光が埋まらないよう足元より少し上で弾けさせる
	void Landing(const Vector3& feet)
	{
		const Vector3 origin = feet + Vector3{ 0.0f, DirectionConfig::kLandingLift, 0.0f };
		ParticleManager::Play(DirectionConfig::kLandingSparkName, origin);
		ParticleManager::Play(DirectionConfig::kLandingGlowName, origin);
	}

	//指定の位置で小さな白い星をまとめて弾けさせる
	void VictoryStars(const Vector3& origin)
	{
		ParticleManager::Play(DirectionConfig::kVictoryStarName, origin);
	}
}
