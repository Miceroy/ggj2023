#pragma once
///
/// CONTROLLER: Toiminnallisuuden määrittelyt:
#include <gridsearch.h>
#include <apply.h> // apply::entities
#include <hungerland/map.h>
#include <hungerland/util.h>

namespace tile_map {
	template<typename Map, typename VecType>
	auto getTileRef(Map& map, const VecType& pos) -> auto& {
		int x = int(pos.x);
		int y = int(pos.y);
		assert(y<map.size());
		assert(x<map[y].size());
		return map[y][x];
	};

	template<typename Map, typename VecType, typename VisualId>
	auto setVisual(Map& map, const VecType& pos, VisualId visualId) {
		getTileRef(map, pos).visualId = visualId;
	};
}

///
/// GAME CONTROLLER: Pelin funktiot
/// - game::isLegalState(const GameState&, const AgentState&) -> bool
/// - game::isGameOver(const GameState&) -> bool
/// - game::update(GameState&) -> void
/// - game::event(GameState&, const Event& event) -> void
///
namespace car_game {
	///
	/// \brief game::isLegalState(const GameState&, const AgentState&) -> bool
	/// \param game		= Game state.
	/// \param agent	= Agent to test legalness.
	/// \return true, if is legal agent state.
	///
	template<typename AgentState, typename GameState>
	bool isLegalState(const GameState& game, const AgentState& agent) {
		// TODO:
		return true;
	}

	///
	/// \brief game::isGameOver(const GameState&) -> bool
	/// \param game		= Game state.
	/// \return true, if some agent winned the game
	///
	template<typename GameState>
	bool isGameOver(const GameState& game) {
		for (size_t i = 0; i < game.agents.size(); ++i) {
			const auto agent = game.agents[i];
			if (agent.state.car.position.x > 261.0f) {
				printf("Game over %s wins!\n", i==0 ? "Player":"AI");
				return true;
			}
		}
		return false;
	}

	///
	/// \brief game::update(GameState& game) -> void
	/// \param game		= Game state.
	///
	template<typename GameState, typename Events>
	void update(GameState& game, Events events) {
		// TODO: Lisää pelilogiikan päivittäminen tähän kohtaa...
	}

	///
	/// \brief game::event(GameState& game) -> void
	/// \param game				= Game state.
	/// \param senderAgentId	= Sender agent id.
	/// \param event			= Event send by agent.
	///
	template<typename GameState, typename SenderId, typename Event>
	void event(GameState& game, const Event& event) {
		// TODO: Lisää eventtien käsittely tähän kohtaan...
	}
} // End - namespace demo_game



///
/// ENVIRONMENT CONTROLLER: Ympäristön funktiot.
/// - update(GameState& gameState, DeltaType delta) -> const GameState&
///
namespace car_env {
	template<typename Body, typename ImpulseFunc, typename ReactFunc, typename VecType, typename DeltaType>
	auto integrateBody(const Body& oldBody, ImpulseFunc getImpulse, ReactFunc reactFunc, VecType F, VecType I, DeltaType dt) {
		const VecType ZERO = VecType(0);
		const float TIME_TOL = 0.005f;
		const auto stepEuler = [&](Body body, DeltaType dt) {
			// Integrate velocity from forces and position from velocity:
			auto i = dt * F;
			body.velocity += I + i;
			body.position += body.velocity * dt;
			body.angle += body.angularVel * dt;
			return body;
		};

		// Run integrator:
		assert(0 == glm::length(getImpulse(oldBody)));
		Body b = oldBody;
		VecType resImpulse = ZERO;
		while(dt > TIME_TOL) {
			Body resBody = b;
			auto deltaTime = dt;
			float usedTime = 0;
			for(size_t i=0; i<4; ++i){
				const auto step = 0.5f*deltaTime;
				const auto newBody = stepEuler(b, deltaTime);
				const auto impulse = getImpulse(newBody);
				if(glm::length(impulse) != 0) {
					// If collides at first step, set impulse:
					if(0 == glm::length(resImpulse)) {
						resImpulse = impulse;
					}
					// Adjust delta time to be smaller:
					deltaTime -= step;
				} else {
					// Not collides, adjust delta time to be bigger:
					usedTime = deltaTime;
					b = resBody = newBody;
					deltaTime += step;
				}
				if(deltaTime<=TIME_TOL || deltaTime >= dt ) {
					break;
				}
			}
			if(usedTime<=TIME_TOL) {
				break;
			}
			I = VecType(0);
			F = VecType(0);
			dt -= usedTime;
		}
		// Make reaction.
		assert(0 == glm::length(getImpulse(b)));
		b = reactFunc(oldBody, b, resImpulse);
		assert(0 == glm::length(getImpulse(b)));
		return b;
	}

	template<typename Body>
	auto getRotationMat(const Body& body) {
		return glm::rotate(glm::mat4(1), body.angle, glm::vec3(0, 0, 1));
	}

	template<typename Body>
	auto getModelMat(const Body& body) {
		const auto scale = glm::scale(glm::mat4(1), glm::vec3(body.sx, body.sy, 0));
		const auto translate = glm::translate(glm::mat4(1), glm::vec3(body.position.x, body.position.y, 0));
		const auto rotate = getRotationMat(body);
		return translate * rotate * scale;
	}

	template<typename MapCollision>
	bool isPenetrating(const MapCollision& col) {
		for (size_t i = 0; i < col.size(); ++i) {
			for (size_t j = 0; j < col[i].size(); ++j) {
				if (col[i][j].x > 0.0f || col[i][j].y > 0.0f) {
					return true;
				}
			}
		}
		return false;
	}

	template<typename VecType, typename MapCollision>
	auto getNormalVec(const MapCollision& col) {
		auto res = VecType(0, 0);
		if (col[0][1].x > 0.0f || col[0][1].y > 0.0f) {
			res += VecType(0, 1);
		}
		if (col[2][1].x > 0.0f || col[2][1].y > 0.0f) {
			res += VecType(0, -1);
		}
		if (col[1][0].x > 0.0f || col[1][0].y > 0.0f) {
			res += VecType(1, 0);
		}
		if (col[1][2].x > 0.0f || col[1][2].y > 0.0f) {
			res += VecType(-1, 0);
		}
		return res;
	}

	template<typename GameState, typename VecType>
	auto getTileId(const GameState& game, const std::string layerName, VecType pos) {
		auto tid = game.tileMap->getTileId(game.tileMap->getLayerIndex(layerName), size_t(pos.x+0.5f), size_t(pos.y + 0.5f));
		printf("Tileid: %d at <%d, %d>\n", tid, int(pos.x + 0.5f), int(pos.y + 0.5f));
		return tid;
	}


	///
	/// \brief car_env::stepCar
	/// \param game
	/// \param cars
	/// \param id
	/// \param actionId
	/// \param delta
	/// \return
	///
	template<typename GameState, typename Entities, typename AgentId, typename ActionId, typename VecType, typename DeltaType>
	bool stepCar(GameState& game, Entities& cars, AgentId id, ActionId actionId, DeltaType delta) {
		auto& car = cars[id].state.car;
		auto& trailer = cars[id].state.trailer;

		if(actionId.turn > 0) {
			car.angularVel = 2.0f * actionId.turn;
		} else if(actionId.turn < 0) {
			car.angularVel = 2.0f * actionId.turn;
		} else {
			car.angularVel = 0;
		}
		auto irotM = glm::rotate(glm::mat4(1), -car.angle, glm::vec3(0,0,1));
		auto vel = irotM * glm::vec4(car.velocity.x, car.velocity.y, 0, 0);

		auto friction = -glm::vec4(0.4f,2.0f,0,1) * vel;

		auto collides = [&game](auto& body) {
			// TODO: Check collision and return impulse
			auto collisions = game.tileMap->checkCollision("CollisionLayer", glm::vec3(body.position.x, body.position.y, 0), glm::vec3(body.sx/3.0f, body.sy / 4.0f, 0));
			return getNormalVec<VecType>(collisions);
		};

		auto react = [](const auto& oldBody, auto newBody, const VecType& impulse) {
			// TODO: React body according to impulse
			if (glm::length(impulse) == 0) {
				return newBody;
			}
			auto newVelocity = 0.8f * glm::reflect(glm::vec3(oldBody.velocity.x, oldBody.velocity.y, 0), glm::vec3(impulse.x, impulse.y, 0));
			newBody.velocity.x = newVelocity.x;
			newBody.velocity.y = newVelocity.y;
			return newBody;
		};

		typedef hungerland::map::Map::MapCollision MapCollision;

		// Update player car body:
		{
			auto rotM = glm::rotate(glm::mat4(1), car.angle, glm::vec3(0, 0, 1));
			auto gas = 4.0f * glm::vec4(actionId.gas, 0, 0, 0);
			auto F = rotM * friction + rotM * gas;
			car = integrateBody(car, collides, react, VecType(F.x, F.y), VecType(0), delta);
		}

		auto cross = [](const glm::vec4& r, const glm::vec4& f) {
			return r.x * f.y - r.y * f.x;
		};

		auto dot = [](const glm::vec4& a, const glm::vec4& b) {
			return a.x * b.x + a.y * b.y;
		};

		// Vetokoulun (hitch) paikka mailmassa
		{
			auto carMat = getModelMat(car);
			auto trailerMat = getModelMat(trailer);
			auto hitchPos = carMat * glm::vec4(-0.5f, 0, 0, 1);
			auto hitchCoupler = trailerMat * glm::vec4(0.5f, 0, 0, 1);

			auto fwdCar = getRotationMat(car) * glm::vec4(1.0f, 0, 0, 1);
			auto fwdTrailer = getRotationMat(trailer) * glm::vec4(1.0f, 0, 0, 1);

			auto F = 150.0f * (hitchPos - hitchCoupler);
			//float angle = std::atan2(cross(fwdTrailer, fwdTrailer), dot(fwdCar, fwdTrailer));
			//printf("Trailer pos: <%2.2f, %2.2f>\n", (hitchPos - hitchCoupler).x, (hitchPos - hitchCoupler).y);
			float torque = cross(glm::normalize(fwdTrailer), F);
			//printf("torque: %2.2f\n", torque);
			trailer.angularVel = 0.9f*torque;
			trailer.velocity *= 0.9f;
			trailer = integrateBody(trailer, collides, react, VecType(F.x, F.y), VecType(0), delta);
		}

		return true; // Legal action
	}

	///
	/// \brief car_env::stepNull
	/// \param game
	/// \param entities
	/// \param id
	/// \param delta
	/// \return
	///
	template<typename GameState, typename Entities, typename AgentId, typename DeltaType>
	bool stepNull(GameState& game, Entities& entities, AgentId id, DeltaType delta) {
		return true;
	}

	///
	/// \brief car_env::stepJuures
	/// \param game
	/// \param entities
	/// \param id
	/// \param dt
	/// \return
	///
	template<typename GameState, typename Entities, typename AgentId, typename DeltaType>
	bool stepJuures(GameState& game, Entities& entities, AgentId id, DeltaType dt) {
		enum State {
			START, TRAILER, FLYING, GROUNDED, DEAD
		};

		auto randomVec = [](float r) {
			return r * (glm::vec2(1.0f) - 2.0f*glm::vec2(float(rand())/float(RAND_MAX),float(rand())/float(RAND_MAX)));
		};

		auto& e = entities[id];
		auto& s = e.state;
		const auto& owner = game.agents[s.owner].state;

		if(s.state == START) {
			s.offset = randomVec(0.25f);
			auto pos = getRotationMat(owner.trailer) * glm::vec4(s.offset.x,s.offset.y,0.0f,1.0f);
			s.position.x = owner.trailer.position.x+pos.x;
			s.position.y = owner.trailer.position.y+pos.y;
			s.state = TRAILER;
		} else if(TRAILER) {
			auto endPos = getRotationMat(owner.trailer) * glm::vec4(s.offset.x,s.offset.y,0.0f,1.0f);
			auto end = glm::vec2(owner.trailer.position.x+endPos.x,owner.trailer.position.y+endPos.y);
			auto cur = s.position;
			auto d = glm::length(end-cur);
			auto v = (end-cur)/dt;
			auto dv = glm::length(v-s.velocity);
			auto a = dv/dt;
			//printf("d=%2.3f, v=%2.3f, dv=%2.3f, a=%2.3f\n", d, glm::length(v), dv, a);
			//auto vel = d;


			s.position.x = owner.trailer.position.x+endPos.x;
			s.position.y = owner.trailer.position.y+endPos.y;
			s.velocity = v;
		} else if(FLYING) {
		} else if(GROUNDED) {
		} else if(DEAD) {
		} else {
			assert(0);
		}
		return true;
	}

	///
	/// \brief car_env::update
	/// \param game
	/// \param delta
	/// \return
	///
	template<typename GameState, typename AgentId, typename ActionId, typename Events, typename VecType, typename DeltaType>
	const GameState& update(GameState& game, DeltaType delta) {
		game.totalTime += delta;
		if (game.totalTime >= 4.0f) {
			game.isRunning = true;
		}
		// Functions to use:
		auto stepCar		= car_env::stepCar<GameState, decltype(game.agents), AgentId, ActionId, VecType, DeltaType>;
		auto stepItem	= car_env::stepNull<GameState, decltype(game.items), AgentId, DeltaType>;
		auto stepProjectile	= car_env::stepJuures<GameState, decltype(game.projectiles), AgentId, DeltaType>;
		// Run updates for entities:
		apply::agents(game, game.agents, delta, -1, stepCar);
		apply::entities(game, game.items, delta, stepItem);
		apply::entities(game, game.projectiles, delta, stepProjectile);

		// TODO: Check (collision) events:
		Events evs;

		// Update game logic
		car_game::update(game, evs);
		return game;
	}
} // End - namespace car_env

namespace car_ai {
	///
	/// \brief plannerDriver
	/// \param agentId
	/// \param gameState
	/// \return
	///
	template<typename Action, typename EventType, typename AgentId, typename GameState>
	Action plannerDriver(AgentId agentId, const GameState& gameState) {
		auto cross = [](const glm::vec2& r, const glm::vec2& f) {
			return r.x * f.y - r.y * f.x;
		};

		const auto& map = *gameState.tileMap;
		auto agentPos = gameState.agents[agentId].state.car.position;
		auto& car = gameState.agents[agentId].state.car;
		auto isLegalState = [&map](const auto& pos) {
			auto roadTileId = map.getTileId(map.getLayerIndex("RoadTiles"), pos.x, pos.y);
			auto collisionTileId = map.getTileId(map.getLayerIndex("CollisionLayer"), pos.x, pos.y);
			return roadTileId > 0 && collisionTileId == 0;
		};

		if (gameState.isRunning) {
			std::vector<glm::vec2> waypoints = gridsearch::searchWaypoints(agentPos, gameState.goals[0].state, isLegalState, 90);
			if (waypoints.size() < 7) {
				hungerland::util::WARN("AI could not find waypoints!\n");
				return Action{ 0, 0 };
			}
			glm::vec2 targetPos = waypoints[5];// +waypoints[5] + waypoints[4] + waypoints[3]);
			//glm::vec2 targetPos = 0.25f * (waypoints[6] + waypoints[5] + waypoints[4] + waypoints[3]);
			//targetPos.x -= 0.5f;
			auto d = targetPos - agentPos;
			auto fordard = car_env::getRotationMat(car) * glm::vec4(1, 0, 0, 1);
			glm::vec2 fwd(fordard.x, fordard.y);
			float cr = cross(fwd, d);

			int steer = 1.0;
			if (std::abs(cr) < 0.5f) steer = 0;
			return Action{ rand() % ((2+ gameState.aiDifficulty)-agentId) != 0, cr < 0 ? -steer : +steer };
		}
		else {
			return Action{0, 0};
		}
	}
}
