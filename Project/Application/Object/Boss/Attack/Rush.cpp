#include "Rush.h"
#include <algorithm>

using namespace RyoEngine;

namespace App {
	void Rush::Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) {
        // 初期化
        origin_ = bossPos;
        position_ = bossPos;
        traveled_ = 0.0f;
        hasHit_ = false;
		
        //向きはプレイヤーへの横向き
        Vector3 toTarget = target - bossPos;
        toTarget.y = 0.0f;
        if (Dot(toTarget, toTarget) < 0.0001f) {
            toTarget = { 0.0f, 0.0f, 1.0f };
        }
        forward_ = Normalize(toTarget);

        //始めた時点のルートで警告を出す
        reach_ = BuildRoute(floor, origin_, origin_ + forward_, tuning.rushMaxDistance, tuning.rushBodyRadius, route_);
        StoreRouteCenters(floor);

        announcementTime_ = tuning.rushWindup;
        recoveryTime_ = tuning.rushRecovery;
        phase_ = AttackPhase::Announcement;
	}

	int32_t Rush::Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) {
		switch (phase_) {
		case AttackPhase::Announcement:
            //ためている間に穴をあけられたら、ルートも短くなる
            reach_ = BuildRoute(floor, origin_, origin_ + forward_, tuning.rushMaxDistance, tuning.rushBodyRadius, route_);
            StoreRouteCenters(floor);
            // 予告時間の減少
			TimeDecrease(announcementTime_, AttackPhase::Attack);
			break;

        case AttackPhase::Attack: {
            //途中で穴があいたら、進める距離を短くする
            std::vector<Cell> route;
            reach_ = std::min(reach_, BuildRoute(floor, origin_, origin_ + forward_, tuning.rushMaxDistance, tuning.rushBodyRadius, route));

            //進めるだけ進む
            traveled_ = std::min(traveled_ + tuning.rushSpeed * deltaTime, reach_);
            position_ = origin_ + forward_ * traveled_;

            //1回の突進で当たるのは1回だけ
            int32_t damage = 0;
            Vector3 toTarget = target - position_;
            toTarget.y = 0.0f;
            if (!hasHit_ && Length(toTarget) <= tuning.rushHitRadius) {
                hasHit_ = true;
                damage = tuning.damage;

                return damage;
            }

            //行けるところまで行ったら止まる
            if (traveled_ >= reach_) {
                phase_ = AttackPhase::Recovery;
            }
            break;
        }

		case AttackPhase::Recovery:
			// 硬直時間の減少
			TimeDecrease(recoveryTime_, AttackPhase::None);
            //終わったら警告を消す
            if (phase_ == AttackPhase::None) {
                route_.clear();
                routeCenters_.clear();
            }
			break;

		default:
			break;
		}
        return 0;
	}

	float Rush::BuildRoute(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& targetPos, float maxDistance, float bodyRadius, std::vector<Cell>& route) {
        // 古い道を削除
        route.clear();

        //横の向きだけ見る
        Vector3 toTarget = targetPos - bossPos;
        toTarget.y = 0.0f;
        if (Dot(toTarget, toTarget) < 0.0001f) {
            return 0.0f;
        }
        const Vector3 forward = Normalize(toTarget);
        //体の幅を調べるための横向き
        const Vector3 side = Cross(Vector3{ 0.0f, 1.0f, 0.0f }, forward);

        //1マスの1/4ずつ進めて調べる
        const float step = std::min(floor.GetTileSizeX(), floor.GetTileSizeZ()) * 0.25f;
        const Cell startCell = floor.GetCellAt(bossPos);

        float distance = 0.0f;
        while (distance + step <= maxDistance) {
            const float next = distance + step;
            const Vector3 center = bossPos + forward * next;

            //体の真ん中と左右の端が触るマス
            const Cell cells[3] = {
                floor.GetCellAt(center),
                floor.GetCellAt(center + side * bodyRadius),
                floor.GetCellAt(center - side * bodyRadius),
            };

            //どれか1つでも穴か外なら、ここで止める
            bool isBlocked = false;
            for (const Cell& cell : cells) {
                if (!floor.Contains(cell) || !floor.IsSolid(cell)) {
                    isBlocked = true;
                    break;
                }
            }
            if (isBlocked){
                break;
            }

            //まだ入れていないマスだけ記録する。ボスが今いるマスは入れない
            for (const Cell& cell : cells) {
                if (cell == startCell || std::find(route.begin(), route.end(), cell) != route.end()) {
                    continue;
                }
                route.push_back(cell);
            }
            distance = next;
        }
        return distance;
	}

    void Rush::Cancel() {
        AttackBase::Cancel();
        routeCenters_.clear();
    }

    void Rush::StoreRouteCenters(const Floor& floor) {
        routeCenters_.clear();
        for (const Cell& cell : route_) {
            routeCenters_.push_back(floor.GetCellCenter(cell));
        }
        tileSize_ = { floor.GetTileSizeX(), floor.GetTileSizeZ() };
    }
}