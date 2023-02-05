#pragma once
#include <random>
#include <memory>
#include <map>

/// <summary>
///
/// </summary>
/// <typeparam name="State"></typeparam>
template<typename State>
struct SearchNode {
	std::shared_ptr<SearchNode> prevNode;
	State state;
	unsigned actionId;
	float cost;
};

/// <summary>
///
/// </summary>
/// <typeparam name="AgentId"></typeparam>
/// <typeparam name="NodeType"></typeparam>
/// <typeparam name="CostFunc"></typeparam>
/// <param name="agentId"></param>
/// <param name="currentNode"></param>
/// <param name="getCost"></param>
/// <returns></returns>
template<typename AgentId, typename NodeType, typename CostFunc>
auto makeAllActions(AgentId agentId, std::shared_ptr<NodeType> currentNode, CostFunc getCost) {
	std::vector< std::shared_ptr<NodeType> > nextStates;
	for (auto actionId = 0u; actionId < currentNode->state.numActions; ++actionId) {
		auto newState = currentNode->state;
		if (true == newState.predict(newState, agentId, actionId)) {
			// Laske etäisyys maaliin ja valitse se actionId, jolla päästään lähimmäksi maalia.
			auto distance = getCost(currentNode, agentId, newState);
			auto n = NodeType{ currentNode, newState, actionId, distance };
			auto newNode = std::make_shared<NodeType>(n);
			nextStates.push_back(newNode);
		}
	}
	return nextStates;
}

/// <summary>
///
/// </summary>
/// <typeparam name="NodeType"></typeparam>
/// <typeparam name="AgentId"></typeparam>
/// <typeparam name="GameState"></typeparam>
/// <typeparam name="CostFunc"></typeparam>
/// <param name="agentId"></param>
/// <param name="gameState"></param>
/// <param name="getCost"></param>
/// <returns></returns>
template<typename NodeType, typename AgentId, typename GameState, typename CostFunc>
auto search(int maxIters, AgentId agentId, GameState gameState, CostFunc getCost) {
	// Lambda, joka poistaa openlistasta pienimmän kustannuksen noden.
	auto popBest = [](std::vector< std::shared_ptr<NodeType> >& openList, AgentId agentId) {
		// Find minimum distance node...
		unsigned minActionSoFar = 0;
		float minDistance = openList[0]->cost;
		for (auto i = 1u; i < openList.size(); ++i) {
			// Laske etäisyys maaliin ja valitse se actionId, jolla päästään lähimmäksi maalia.
			auto distance = openList[i]->cost;
			if (distance < minDistance) {
				minDistance = distance;
				minActionSoFar = i;
			}
		}
		auto res = openList[minActionSoFar];
		openList.erase(openList.begin() + minActionSoFar);
		return res;
	};

	auto reverseRoute = [](std::shared_ptr<NodeType> node) {
		std::vector< std::shared_ptr<NodeType> > plan;
		while (node != 0) {
			plan.push_back(node);
			node = node->prevNode;
		}
		std::reverse(plan.begin(), plan.end());
		return plan;
	};


	// Kanditaattisolmuja, joita ei ole vielä "laajennettu"
	std::vector< std::shared_ptr<NodeType> > openList;
	std::map< std::vector<float>, bool > closedList;

	// Lisää alkutila ensimmäiseksi laajennettavaksi tilaksi open listiin.
	auto n = NodeType{ 0, gameState, unsigned(-1), getCost(0, agentId, gameState) };
	auto initialState = std::make_shared<NodeType>(n);
	openList.push_back(initialState);

	// Hakulooppi, joka käy openlistiä läpi..
	std::shared_ptr<NodeType> curNode = 0;
	while (maxIters >= 0 && openList.empty() == false) {
		// Ota pienimmän kustannuksen node pois open lististä
		// ja käsittele se
		curNode = popBest(openList, agentId);
		auto h = gameState.getHash(agentId, curNode->state);
		closedList[h] = true;
		//auto h = curNode->state.getHash(agentId, curNode->state);
		//printf("CurNode: %f %f}\n", (float)h[0], (float)h[1]);
		auto gameState = curNode->state;
		if (gameState.isGameOver(gameState)) {
			//printf("  Maali loytyi\n");
			return reverseRoute(curNode);
		}
		// Tee kaikki actionit "current nodelle" (branch -osuus)
		for (auto newNode : makeAllActions(agentId, curNode, getCost)) {
			auto h = gameState.getHash(agentId, newNode->state);

			if (closedList.find(h) != closedList.end()) {
				// Löytyi closed lististä, skippaa tämä solmu (bound)
				continue;
			}

			auto it = std::find_if(openList.begin(), openList.end(), [&](auto n) {
				return h == gameState.getHash(agentId, n->state);
			});
			if (it != openList.end()) {
				// Löytyi open listasta, skippaa tämä solmu (bound)

				if (newNode->cost < (*it)->cost) {
					// Vertaa kustannuksia ja jos uusi somlu on halvenmpipi, kuin open
					// listasta jo löytyvä solmu, niin korvaa aiempi solmu uudella solmulla,
					// jonka kustannus on siis lyhempi, kuin aiemman solmun.
					*it = newNode;
					//throw "TODO: Implementation"; // TODO: Poista tämä sitten kun ensimmäisen kerran joskus softa kaatuu tähän.
				}
				continue;
			}

			//printf("  NewNode to add to openList: %f %f}\n", (float)h[0], (float)h[1]);
			--maxIters;
			openList.push_back(newNode);
		}
	}
	//printf("  Ei ole reittia loppuun. Palauta keskenerainen plan\n");
	openList.push_back(curNode);
	return	reverseRoute(curNode);
}

