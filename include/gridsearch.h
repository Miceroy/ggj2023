#pragma once
#include <assert.h> // Ei vara venettä kaada, eikä assert toimivaa softaa...
#include <cmath>
#include <functional> // std::function
#include <array> // std::array
#include <vector> // std::array
#include <ai_algos.h>

namespace gridsearch {

///
/// \brief MODEL: The GameState data
///
struct GameState {
	/// predict(GameState&, AgentId, ActionId) -> bool
	typedef std::function<bool(GameState&, size_t, size_t)> PredictFunc;
	/// isLegalState(const GameState&, AgentId) -> bool
	typedef std::function<bool(GameState&, size_t)>			LegalStateFunc;
	/// isGameOver(const GameState&) -> bool
	typedef std::function<bool(GameState&)>					GameOverFunc;
	/// getHCost(const GameState&) -> float
	typedef std::function<float(size_t, const GameState&)>	HCostFunc;
	typedef std::function<float(size_t, const GameState&, size_t)>	QCostFunc;
	/// getHCost(const GameState&) -> std::vector<float>
	typedef std::function<std::vector<float>(size_t, const GameState&)>	HashFunc;

	/// PolicyFunc(AgentId, const GameState&) -> size_t
	typedef std::function<size_t(size_t, GameState&)> PolicyFunc;
	/// ActionFunc(GameState&,AgentId) -> void
	typedef std::function<void(GameState&,size_t)>			ActionFunc;

	/// Typedef for Position
	struct Position { int x, y; };

	/// \brief The Goal entity.
	struct Goal {
		Position	state;
	};

	struct AgentState {
		Position			position;
		std::vector<size_t> actions;
		// TODO: add open list, closed list and path positions here.
	};

	/// \brief The Agent entity consists of state and policy function.
	struct Agent {
		AgentState	state;
		PolicyFunc	policy;
	};
	/// Game data is Environment map and game agents and numActions
	//std::vector< std::vector<int> >	map;
	std::vector<Goal>				goals;
	std::vector<Agent>				agents;

	/// Actions
	std::vector<ActionFunc> actions;
	size_t			numActions=0;

	/// Functions for agent use:
	PredictFunc		predict;
	LegalStateFunc	isLegalState;
	GameOverFunc	isGameOver;
	HCostFunc		getHCost;
	HashFunc		getHash;

	QCostFunc		getQCost;
};

////
/// \brief searchWaypoints
/// \param start
/// \param end
/// \param isLegalState
/// \param MAX_ITERS
///
template<typename VecType, typename IsLegalStateFunc>
auto searchWaypoints(const VecType& start, const VecType& end, IsLegalStateFunc isLegalState, int MAX_ITERS) {
	typedef SearchNode<gridsearch::GameState> NodeType;
	auto createSearch = [](const VecType& start, const VecType& goal) {
		return gridsearch::GameState {
			// Single goal
			{{int(goal.x+0.5f),int(goal.y+0.5f)}},
			// Single agent at position 10,10
			{gridsearch::GameState::Agent{{int(start.x+0.5f),int(start.y+0.5f)}, 0}},
		};
	};
	// H(n) -> float
	auto getHCost = [](std::shared_ptr<NodeType> node, auto agentId, const auto& gameState) {
		return gameState.getHCost(agentId, gameState);
	};

	// G(n) -> float
	auto getGCost = [](std::shared_ptr<NodeType> node, auto agentId, const auto& gameState) {
		float gCost = 0;
		while (node != 0) {
			gCost += 1.0f;
			node = node->prevNode;
		}
		return gCost;
	};

	// F(n) = G + H  -> float
	auto getFCost = [&](std::shared_ptr<NodeType> node, auto agentId, const auto& gameState) {
		return getGCost(node, agentId, gameState) + 10.0f*getHCost(node, agentId, gameState);
	};
	auto costFuntion = [&](std::shared_ptr<NodeType> node, size_t agentId, const gridsearch::GameState& gameState) {
		return getFCost(node, agentId, gameState);
	};
	auto searchGame = createSearch(start,end);
	searchGame.actions = {
		// 0 = right
		[](auto& game, size_t actionId) {
			game.agents[0].state.position.x += 1;
		},
		// 1 = left
		[](auto& game, size_t actionId) {
			game.agents[0].state.position.x -= 1;
		},
		// 2 = up
		[](auto& game, size_t actionId) {
			game.agents[0].state.position.y -= 1;
		},
		// 3 = down
		[](auto& game, size_t actionId) {
			game.agents[0].state.position.y += 1;
		},
	};
	searchGame.numActions = searchGame.actions.size();
	searchGame.predict = [](auto& game, auto agentId, auto actionId) {
		// Check errorneus actionId
		if(actionId >= game.actions.size()) return false;
		// Apply agent action by actionId
		game.actions[actionId](game, actionId);
		// Return true, if agent made legal action, which leads to legal state
		return game.isLegalState(game, agentId);
	};
	searchGame.isGameOver = [](const auto& game){
		// If agent is in goal state (10,0), agent is winner
		auto agentState = game.agents[0].state.position;
		auto goal = game.goals[0].state;
		return agentState.x == goal.x && agentState.y == goal.y;
	};
	searchGame.getHash = [](size_t agentId, const auto& gameState) {
		const auto& agent = gameState.agents[agentId].state.position;
		return std::vector<float>{(float)agent.x, (float)agent.y};
	};
	searchGame.getQCost = [](auto agentId, auto gameState, auto actionId) {
		return 1.0f;
	};
	searchGame.getHCost = [](auto agentId, auto gameState) {
		const auto& agent = gameState.agents[agentId].state.position;
		const auto& goal = gameState.goals[0].state; // Ensimmäinen maali
		auto dx = goal.x - agent.x; // Loppupiste miinus alkupiste
		auto dy = goal.y - agent.y;
		return (float)std::sqrt(dx*dx + dy*dy); // Laske pythagooraan lauseella etäisyys maaliin
	};
	searchGame.isLegalState = [&isLegalState](const auto& game, auto agentId) {
		return isLegalState(game.agents[0].state.position);
	};

	// Do search:
	auto plan = search<NodeType>(MAX_ITERS, 0, searchGame, costFuntion);
	std::vector<VecType> waypoints;
	for (size_t i = 1; i < plan.size(); ++i) {
		auto p = plan[i]->state.agents[0].state.position;
		waypoints.push_back({p.x,p.y});
	}
	return waypoints;
}


} // End - namespace gridsearch

