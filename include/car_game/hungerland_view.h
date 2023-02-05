#pragma once
///
/// VIEW: Pelin tilan näyttäminen käyttäjälle.
#include <car_game/controller.h>
#include <hungerland/window.h>
#include <hungerland/texture.h>
#include <hungerland/util.h>
#include <hungerland/map.h>
namespace hungerland_view {
using namespace hungerland;

namespace render {
	template<typename Body>
	auto getShadowModelMat(const Body& body) {
		glm::vec3 shadowOffset(-0.15f, 0.05f, 0);
		const auto scale = glm::scale(glm::mat4(1), glm::vec3(body.sx, body.sy, 0));
		const auto translate = glm::translate(glm::mat4(1), shadowOffset+glm::vec3(body.position.x, body.position.y, 0));
		const auto rotate = car_env::getRotationMat(body);
		return translate * rotate * scale;
	}

	template<typename F, typename Entity, typename GameState>
	auto agent(F f, const GameState& state, const Entity& e) {
		if(e.type.visualId < 0) return; // No visual, skip
		{
			// Render trailer:
			auto mat = car_env::getModelMat(e.state.trailer);
			auto visualId = e.state.trailer.visualId;
			auto shadowMat = getShadowModelMat(e.state.trailer);
			f(shadowMat, *state.texturesBlack[visualId]);
			f(mat, *state.textures[visualId]);
		}
		{
			// Render car:
			auto mat = car_env::getModelMat(e.state.car);
			auto visualId = e.type.visualId;
			auto shadowMat = getShadowModelMat(e.state.car);
			// Render:
			f(shadowMat, *state.texturesBlack[visualId]);
			f(mat, *state.textures[visualId]);
		}
	};

	template<typename F, typename Entity, typename GameState>
	auto entity(F f, const GameState& state, const Entity& e) {
		if (e.type.visualId < 0) return; // No visual, skip
		{
			auto mat = car_env::getModelMat(e.state);
			auto visualId = e.type.visualId;
			// Render:
			f(mat, *state.textures[visualId]);
		}
	};

} // End - namespace render


	///
	/// \brief renderState
	/// \param screen
	/// \param state
	///
	template<typename Screen, typename GameState>
	void renderState(Screen& screen, const GameState& state) {
		auto matProj = screen.setScreen(0.0f, 1280.0f, 768.0f, 0.0f);
		screen.clear(0.5f, 0.0f, 0.5f, 1.0f);
		auto cameraPosition = glm::vec3(0);
		cameraPosition.x = state.agents[0].state.car.position.x;
		cameraPosition.y = state.agents[0].state.car.position.y;

		auto camOffset = glm::vec3(9.5f, 5.5f, 0);
		const auto renderer = [&](const glm::mat4& M, const hungerland::texture::Texture& texture) {
			glm::vec3 tileSize(state.tileMap->getTileSize().x, state.tileMap->getTileSize().y, 0.0f);
			auto scaleMat = glm::scale(glm::mat4(1), { 64.0f, 64.0f, 1.0f });
			auto mat = glm::translate(glm::mat4(1), camOffset - cameraPosition + glm::vec3(0.5f, 0.5f, 0.0f));
			screen.drawSprite(scaleMat*mat*M, /*getTexture(texture)*/texture);
		};

		auto renderAgent = [&state,&renderer](auto agentId, const auto& agent) {
			render::agent(renderer, state, agent);
		};

		auto renderEntity = [&state, &renderer](auto agentId, const auto& agent) {
			render::entity(renderer, state, agent);
		};

		auto renderMapLayers = [&](const hungerland::map::Map& map, glm::mat4 matProj, glm::vec3 cameraPosition) {
			glm::vec3 tileSize(map.getTileSize().x, map.getTileSize().y, 0.0f);
			auto mat = glm::scale(glm::mat4(1), { 1,1,1 });
			mat = glm::translate(mat, (camOffset- cameraPosition) * tileSize);
			hungerland::map::draw(map, matProj * mat, glm::vec3(0.0f));
			return matProj;
		};


		///
		/// RENDERING:

		// Render map:
		renderMapLayers(*state.tileMap, matProj, cameraPosition);

		// Render agent cars:
		utils::forEach(state.agents, renderAgent);
		utils::forEach(state.items, renderEntity);
		utils::forEach(state.projectiles, renderEntity);
		auto scaleMat = glm::scale(glm::mat4(1), { 4.0f, 4.0f, 1.0f });
		//auto mat = glm::translate(glm::mat4(1), camOffset - cameraPosition + glm::vec3(0.5f, 0.5f, 0.0f));
		auto mat = glm::translate(glm::mat4(1), cameraPosition - glm::vec3(8.0f, 4.0f, 0.0f));
		if (state.totalTime < 4.0f) {
			renderer(mat* scaleMat, *state.textures[state.textures.size() - 2]);
		} else if (state.totalTime < 7.0f) {
			renderer(mat* scaleMat, *state.textures[state.textures.size()-1]);
		}
	}



	///
	/// \brief runApp
	/// \param resetApp
	/// \param updateApp
	/// \param isAppEnd
	/// \return
	///
	template<typename GameState, typename AgentId, typename ActionId, typename ResetFunc, typename UpdateFunc, typename IsEndFunc>
	int runApp(ResetFunc resetApp, UpdateFunc updateApp, IsEndFunc isAppEnd) {
		// Tee pelisovelluksen ikkuna:
		static const unsigned long	SCREEN_SIZE_X = 1280;
		static const unsigned long	SCREEN_SIZE_Y = 768;
		hungerland::window::Window window({ SCREEN_SIZE_X, SCREEN_SIZE_Y }, "Global Game Jam 2023 Game by Miceroy & Filthsu", true);
		struct InputButtons {
			const int r;
			const int l;
			const int u;
			const int d;
		};
		auto getButtonAction = [&window](const InputButtons& b) {
			auto& input = window.getInput();
			return ActionId {
				input.getKeyState(b.u)-input.getKeyState(b.d),
				input.getKeyState(b.r)-input.getKeyState(b.l),
			};
		};

		// Player 1 input (policy) funktio:
		using namespace hungerland::window;
		auto player1 = [&getButtonAction](AgentId agentId, const GameState& game) {
			if (game.isRunning) {
				return getButtonAction({
					KEY_RIGHT,KEY_LEFT,
					KEY_UP,KEY_DOWN });
			}
			else {
				return ActionId{ 0, 0 };
			}
		};

		// Tee pelin tila erilaisista politiikkafunktioista:
		auto gameState = resetApp(window, {player1}, {});

		// Run the application as long as the window is open:
		int n=0;
		float totalTime=0;
		int soundPlaying = -1;
		int res = window.run([&](auto& window, float dt) {
			//printf("\nFrame=%d, totalTime=%2.2f\n", n++, totalTime);
			totalTime += dt;
			int soundIndex = int(totalTime / 100.0f);
			if (soundIndex != soundPlaying) {
				window.playSound("assets/Sounds/hardbass_"+std::to_string((rand()%4)+1)+".wav");
				soundPlaying = soundIndex;
			}
			if (car_game::isGameOver(updateApp(gameState, dt))) {
				window.screenshot("end_state.png");
				return false;
			}
			return true;
		}, [&](auto& screen) {
			renderState(screen, gameState);
		});

#if defined(_WIN32) ||defined(WIN32)
		if (car_game::isGameOver(gameState)) {
			system("start end_state.png &");
		}
#endif
		return res;
	}
} // End - namespace hungerland_view
