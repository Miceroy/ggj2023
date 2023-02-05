#pragma once
#include <cmath>

namespace utils {
	template<typename Entities, typename Func>
	auto forEach(Entities& entities, Func func) {
		for(auto agentId = 0u; agentId<entities.size(); ++agentId) {
			func(agentId, entities[agentId]);
		}
	};

	template<typename Obj1, typename Obj2, typename Scalar>
	bool isCollisionSphereSphere(const Obj1& o1, const Obj2& o2, Scalar r1, Scalar r2) {
		auto dx = o2.x - o1.x;
		auto dy = o2.y - o1.y;
		auto d = std::sqrt(dx*dx + dy*dy);
		auto r = r1+r2;
		return d <= r;
	}

	template<typename Entities, typename CheckFunc>
	auto checkOr(Entities& entities, CheckFunc check) {
		for(auto agentId = 0u; agentId<entities.size(); ++agentId) {
			if(true == check(agentId, entities[agentId])) {
				return true;
			}
		}
		return false;
	};

	template<typename Entities, typename CheckFunc>
	auto checkAnd(Entities& entities, CheckFunc check) {
		for(auto agentId = 0u; agentId<entities.size(); ++agentId) {
			if(false == check(agentId, entities[agentId])) {
				return false;
			}
		}
		return true;
	};

	template<typename Entities, typename CheckFunc>
	auto checkErase(Entities& entities, CheckFunc check) {
		// Iterate each entity and call function. If function returns false, kill object.
		for(auto agentId = 0u; agentId<entities.size(); ++agentId) {
			if(false == check(agentId, entities[agentId])) {
				// Kill entity
				entities.erase(entities.begin()+agentId);
				--agentId;
			}
		}
	};

	///
	/// \brief checkLine Line travelling algorithm based on Bresenham Line-Drawing.
	/// \param x1 = start y
	/// \param y1 = start y
	/// \param x2 = end x
	/// \param y2 = end y
	/// \param isWalkable = f(x,y) -> bool. Shall return true, if x,y is walkable.
	/// \return true if there is no obstacles in line from start to end.
	///
	template<typename CheckFunc>
	bool checkLine(int x1, int y1, const int x2, const int y2, CheckFunc check) {
		const int dx = abs(x2 - x1);
		const int dy = abs(y2 - y1);
		const int sx = (x1 < x2) ? 1 : -1;
		const int sy = (y1 < y2) ? 1 : -1;
		int err = dx - dy;
		int e2;

		while (true) {
			if (!check(x1, y1)) {
				return false;
			}

			if (x1 == x2 && y1 == y2) {
				break;
			}

			e2 = 2 * err;
			if (e2 > -dy) {
				err = err - dy;
				x1 = x1 + sx;
			}
			if (e2 < dx) {
				err = err + dx;
				y1 = y1 + sy;
			}
		}
		return true;
	}

} // End namespace utils


