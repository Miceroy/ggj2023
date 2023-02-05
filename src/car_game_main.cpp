/// CONTROLLER: Pelin funktiot ja agenttifunktiot:
#include <car_game/controller.h>
/// VIEW: Pelin tilan näyttäminen käyttäjälle käyttäen hungerland:
#include <car_game/hungerland_view.h>
namespace view = hungerland_view;
/// MODEL: Sovelluksen datan tietorakenteet:
#include <car_game/std_model.h>
#include <time.h>

// Määritä nimiavatuudet demo pelin nimiavaruuksiksi:
namespace env = car_env;
namespace game = car_game;
namespace model = car_model;


/// Demosovellus:
/// - Maalipohjainen peli.
/// - Ympäristö tilepohjainen
/// - Tilapohjainen mappi, 1 Pelaaja, 1 vihu, 1 itemi.
/// - Mapissa hieman seiniä, joiden päällä ei voi kävellä.
/// -
int selection = -1;
int aiDifficulty = -1;
typedef model::GameState < hungerland::texture::Texture, glm::vec2, hungerland::map::Map > Game;
auto DemoApplication(hungerland::window::Window& window, std::vector<Game::PolicyFunc> policies, std::vector<Game::EventFunc> eventHandlers) {
	srand((unsigned)time(0));
	typedef std::shared_ptr<hungerland::texture::Texture> TexturePtr;
	std::map<std::string, TexturePtr> textureCache;
	auto loadTexture = [&textureCache](const std::string& filename, bool black=false) -> TexturePtr{
		auto hash = filename + (black ? "_black" : "");
		auto it = textureCache.find(hash);
		if (it != textureCache.end()) {
			return it->second;
		}

		hungerland::window::Image image(filename);
		assert(image.data != 0);
		std::vector<uint8_t> imageData;
		imageData.resize(image.size.y * image.size.x * 4);
		bool hasTransparent = false;
		for (auto y = 0; y < image.size.y; ++y) {
			for (auto x = 0; x < image.size.x; ++x) {
				auto id = 4 * unsigned(y * image.size.x + x);
				auto is = image.bpp * unsigned(y * image.size.x + x);
				if (black) {
					imageData[id + 0] = 0;
					imageData[id + 1] = 0;
					imageData[id + 2] = 0;
					imageData[id + 3] = 80;
				}
				else {
					imageData[id + 0] = image.data[is + 0];
					imageData[id + 1] = image.data[is + 1];
					imageData[id + 2] = image.data[is + 2];
					imageData[id + 3] = 0xff;
				}
				if (image.data[is + 0] > 0xf0 && image.data[is + 1] <= 0x0f && image.data[is + 2] > 0xf0) {
					hasTransparent = true;
					imageData[id + 3] = 0x00;
				}
			}
		}
		printf("Loaded image: %s %s transparent pixels\n", filename.c_str(), hasTransparent ? "has" : "has not");
		//auto texture = std::make_shared<hungerland::texture::Texture>(width, height, 4, &data[0]);
		TexturePtr texture = std::make_shared<hungerland::texture::Texture>(image.size.x, image.size.y, 4, &imageData[0]);
		textureCache[hash] = texture;
		return texture;
	};


	/// Lisää player inputin lisäksi AI:n käyttäytymisiä:
	policies.push_back(car_ai::plannerDriver<Game::Action,Game::Event,Game::AgentId,Game>);
	// TODO: Lisää muita käyttäytymisfunktioita tähän...


	/// Entiteettien nimet:
	enum Classes {
		PLAYER, PLAYERAI, SEDAN, VAGON, TRAILER, LANTHU, PORCHANA, REDJUUR, ZIBAL,  NUM_CLASSES
	};

	/// Entiteettiluokat: (Luokka: visual id, act func, event func):
	typedef Game::MetaClass Class;
	std::array<Class, NUM_CLASSES> classes = {
		Class{PLAYER,			policies[0], 0},// PLAYER = 0
		Class{PLAYER,			policies[1], 0},// PLAYERAI = 0
		Class{SEDAN,			policies[1], 0},// SEDAN AI
		Class{VAGON,			policies[1], 0},// VAGON AI
		Class{TRAILER,			0, 0},			// TRAILER
		Class{LANTHU,			0, 0},			// LANTHU
		Class{PORCHANA,			0, 0},			// PORCHANA
		Class{REDJUUR,			0, 0},			// REDJUUR
		Class{ZIBAL,			0, 0},			// ZIBAL
		// TODO: Lisää muita luokkia tähän, jos haluat:
	};


	// Create map:
	auto tileMap = hungerland::map::load<hungerland::map::Map>(loadTexture, "assets/race2.tmx", false);

	// Varsinaiset pelaajainstanssit (agentit):
	Game::VecType posC(7.5f, 9.0f);
	Game::VecType posT = posC - Game::VecType(1.0, 0);
	std::vector<Game::PrefabAgent> agents;
	auto aiType = rand() % 2;
	if (selection == 1) {
		agents = {
		{classes[PLAYERAI],	{{posC}, {classes[TRAILER].visualId, posT}}}, // Pelaaja AI
		{aiType==0?classes[SEDAN]: classes[VAGON],	{{posC + Game::VecType(0.0f, 1)}, {classes[TRAILER].visualId, posT + Game::VecType(0.0f, 1)}}}, // AI
		};
	}
	else if (selection == 2) {
		agents = {
		{classes[PLAYER],	{{posC}, {classes[TRAILER].visualId, posT}}}, // Pelaaja
		{aiType == 0 ? classes[SEDAN] : classes[VAGON],	{{posC + Game::VecType(0.0f, 1)}, {classes[TRAILER].visualId, posT + Game::VecType(0.0f, 1)}}}, // AI
		};
	}

	// Itemit:
	std::vector<Game::PrefabItem> items =  {
		// TODO: Lisää/poista itemeitä.
	};

	// Projectiles:
	std::vector<Game::PrefabProjectile> projectiles =  {
		{classes[LANTHU], 0 },
		{classes[PORCHANA],0 },
		{classes[REDJUUR], 0 },
		{classes[ZIBAL], 0 },
		{classes[LANTHU], 0 },
		{classes[PORCHANA],0 },
		{classes[REDJUUR], 0 },
		{classes[ZIBAL], 0 },
		{classes[LANTHU], 1 },
		{classes[PORCHANA], 1 },
		{classes[REDJUUR], 1 },
		{classes[ZIBAL], 1 },
		{classes[LANTHU], 1 },
		{classes[PORCHANA],1 },
		{classes[REDJUUR], 1 },
		{classes[ZIBAL], 1 },
		// TODO: Lisää/poista itemeitä.
	};

	// Projectiles:
	std::vector<Game::Goal> goals = {
		{Game::VecType(tileMap->getMapSize().x - 30, tileMap->getMapSize().y - 10)},
	};


	std::vector<Game::TexturePtr> textures = {
		loadTexture("assets/Player/car1.png", false),
		loadTexture("assets/Player/car1.png", false),
		loadTexture("assets/Enemies/Enemy_Sedan.png", false),
		loadTexture("assets/Enemies/Enemy_Wagon.png", false),
		loadTexture("assets/Player/trailer.png", false),
		loadTexture("assets/Vegetables/Lanthu.png", false),
		loadTexture("assets/Vegetables/Porchana.png", false),
		loadTexture("assets/Vegetables/Redjuur.png", false),
		loadTexture("assets/Vegetables/Zibal.png", false),
		loadTexture("assets/UI/trafficlight_off.png", false),
		loadTexture("assets/UI/trafficlight_red.png", false),
		loadTexture("assets/UI/trafficlight_green.png", false),
	};
	assert(textures.size()==(NUM_CLASSES+3));

	std::vector<Game::TexturePtr> texturesBlack = {
		loadTexture("assets/Player/car1.png", true),
		loadTexture("assets/Player/car1.png", true),
		loadTexture("assets/Enemies/Enemy_Sedan.png", true),
		loadTexture("assets/Enemies/Enemy_Wagon.png", true),
		loadTexture("assets/Player/trailer.png", true),
	};
	printf("INFO: Ai Difficulty:%d\n", aiDifficulty);
	// Palauta uusi pelin alkutila:
	return Game {
		aiDifficulty,
		model::genClasses(classes), textures, texturesBlack, tileMap,
		model::genEntities<Game::EntityAgent>(agents),
		model::genEntities<Game::EntityItem>(items),
		model::genEntities<Game::EntityProjectile>(projectiles),
		goals,
	};
};

///
/// \brief SFML-pääsovellus
/// \return int
///
int main() {
	static const auto updateFunc = env::update<Game,Game::AgentId,Game::Action,Game::Events,Game::VecType,float>;
	static const auto isEndFunc = game::isGameOver<Game>;
	
	while (selection != 1 && selection != 2) {
		printf("\nGlobal Game Jam 2023 Game Menu:\n");
		printf("1. Run demo\n");
		printf("2. Run single player\n-> ");
		char buf[255] = "";
		scanf("%s", &buf[0]);
		selection = atoi(buf);
		if (selection != 1 && selection != 2) {
			printf("\nInvalid selection: %d!\n", selection);
		}
	}

	while (aiDifficulty <= 0 || aiDifficulty > 5) {
		printf("\nSelect AI difficulty: 1-5\n-> ");
		char buf[255] = "";
		scanf("%s", &buf[0]);
		aiDifficulty = atoi(buf);
		if (aiDifficulty <= 0 || aiDifficulty > 5) {
			printf("\nInvalid selection: %d!\n", aiDifficulty);
		}
	}
	// Aja view namespacen runApp funktio ja anna sille Demosovelluksen luontifunktio, ympäristön update ja gamen loppuehtofunktio argumenttina:
	int res= view::runApp<Game,Game::AgentId,Game::Action>(DemoApplication, updateFunc, isEndFunc);

	return res;
}
