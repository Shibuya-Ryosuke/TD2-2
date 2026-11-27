#pragma once

namespace App
{
	//パーツに分けたボスの姿勢で、角度は度、前は顔の向き
	struct BossPose
	{
		float sink = 0.0f; //体全体を下げる高さで、倍率をかける前の長さ
		float bodyLean = 0.0f; //上半身を前へ倒して、マイナスで後ろへそる
		float bodyRoll = 0.0f; //上半身を右へ傾ける
		float bodyTwist = 0.0f; //上半身を右へひねる
		float headNod = 0.0f; //頭を前へ倒して、マイナスで上を向く
		float headRoll = 0.0f; //頭を右へかしげる
		float armForwardL = 0.0f; //左腕を前へ振り上げて、180で真上
		float armForwardR = 0.0f; //右腕を前へ振り上げて、180で真上
		float armOutL = 0.0f; //左腕を外へ開く
		float armOutR = 0.0f; //右腕を外へ開く
		float legForwardL = 0.0f; //左足を前へ振り上げて、90で前へ投げ出す
		float legForwardR = 0.0f; //右足を前へ振り上げて、90で前へ投げ出す
		float legOutL = 0.0f; //左足を外へ開く
		float legOutR = 0.0f; //右足を外へ開く
	};

	//aからbへtの割合だけ近づけた姿勢
	BossPose LerpBossPose(const BossPose& a, const BossPose& b, float t);
}
