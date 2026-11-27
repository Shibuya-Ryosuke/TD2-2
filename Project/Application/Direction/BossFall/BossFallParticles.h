#pragma once
#include <RyoEngine.h>

//ボスが叩きつけられたときの煙と、ハンマーのドンで地面に広がる光の輪
namespace App::BossFallParticles
{
	//エディターで保存した設定を残して、未登録の効果に初期値を入れる
	void Initialize();
	//ボスの足元からピンクの煙を外へ広げる
	void SlamPuff(const RyoEngine::Vector3& feet);
	//ハンマーが当たった所から光の輪を広げる
	void HammerRing(const RyoEngine::Vector3& contact);
}
