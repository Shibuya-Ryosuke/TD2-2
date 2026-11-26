#pragma once

namespace App
{
	//分けたパーツのプレイヤーの姿勢で、角度は度、0がまっすぐ立って腕を少し開いた形
	struct RigPose
	{
		float waistLean = 0.0f; //上半身を前へ傾ける
		float waistRoll = 0.0f; //上半身を右へ傾ける
		float waistTwist = 0.0f; //上半身を右へひねる
		float headNod = 0.0f; //頭を前へ倒す
		float headRoll = 0.0f; //頭を右へかしげる
		float headYaw = 0.0f; //頭を右へ向ける
		float armForwardL = 0.0f; //左腕を前へ振り上げる
		float armForwardR = 0.0f; //右腕を前へ振り上げる
		float armOutL = 0.0f; //左腕を外へ開く
		float armOutR = 0.0f; //右腕を外へ開く
		float legForwardL = 0.0f; //左足を前へ振り上げる
		float legForwardR = 0.0f; //右足を前へ振り上げる
		float legOutL = 0.0f; //左足を外へ開く
		float legOutR = 0.0f; //右足を外へ開く
		float bob = 0.0f; //体全体を上げる高さで、プレイヤーの倍率をかける前の長さ
		float squash = 1.0f; //体全体を縦につぶす倍率で、横はその分ふくらむ
		float spin = 0.0f; //体全体を右回りに回す
		float bodyTilt = 0.0f; //体全体を腰を中心に後ろへ倒して、90で仰向け
	};

	//aからbへtだけ近づけた姿勢
	RigPose LerpPose(const RigPose& a, const RigPose& b, float t);
}
