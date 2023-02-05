#pragma once
#include <utils.h>

namespace apply {

	template<typename GameState, typename Entities, typename DeltaType, typename ApplyFunc>
	auto agents(GameState& game, Entities& entities, DeltaType delta, int skipAgent, ApplyFunc apply) {
		// Apply each entity policy
		utils::checkErase(entities, [&](auto agentId, auto& agent){
			if(agentId == skipAgent) {
				return true; // Skip given agent
			}
			if(false == agent.isAlive) {
				return false; // Remove agent since it is dead
			}
			// Update cooldown
			agent.coolDownTimer -= delta;
			if(agent.coolDownTimer < 0) agent.coolDownTimer = 0;

			auto policy = agent.policy;
			if(policy && agent.coolDownTimer <= 0) {
				// Make action according to entity policy:
				const auto actionId = policy(agentId, game);
				// Step env:
				if(false == apply(game, entities, agentId, actionId, delta)) {
					// TODO: Illegal action made
				} else {
					// TODO: Legal action made
				}
			}
			return true; // Don't remove agent
		});
	};

	template<typename GameState, typename Entities, typename DeltaType, typename ApplyFunc>
	auto entities(GameState& game, Entities& entities, DeltaType delta, ApplyFunc apply) {
		// Apply each entity policy
		utils::checkErase(entities, [&](auto entityId, auto& entity){
			if(false == entity.isAlive) {
				return false; // Remove agent since it is dead
			}
			// Update cooldown
			entity.coolDownTimer -= delta;
			if(entity.coolDownTimer < 0) entity.coolDownTimer = 0;

			// Step env:
			return apply(game, entities, entityId, delta);
		});
	};


} // End - namespace apply


