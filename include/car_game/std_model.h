#pragma once
///
/// MODEL: Sovelluksen datan tietorakenteiden määrittelyt:
#include <meta.h> // Meta classes for model
#include <functional>	// std::function
#include <memory>		// std::shared_ptr

namespace car_model {
	///
	/// \brief The CarState class
	///
	template<typename VecType>
	struct CarState {
		VecType	position = VecType(0);
		float angle = 0;
		float sx = 1.0f;
		float sy = 1.0f;
		VecType	velocity = VecType(0);
		float angularVel = 0;
	};

	///
	/// \brief The TrailerState class
	///
	template<typename VisualType, typename VecType>
	struct TrailerState {
		VisualType visualId = -1;
		VecType	position = VecType(0);
		float angle = 0.0f;
		float sx = 1.0f;
		float sy = 1.0f;
		VecType	velocity = VecType(0);
		float angularVel = 0;
	};

	///
	/// \brief The AgentState class
	///
	template<typename VisualType, typename VecType>
	struct AgentState {
		CarState<VecType> car;
		TrailerState<VisualType,VecType> trailer;
		size_t targetId = 0;
	};

	///
	/// \brief The ItemState class
	///
	template<typename VecType>
	struct ItemState {
		VecType	position = VecType(0);
		float angle = 0.0f;
		float sx = 0.5f;
		float sy = 0.5f;
	};

	///
	/// \brief The ProjectileState class
	///
	template<typename VecType>
	struct ProjectileState {
		int owner = 0;
		VecType	position = VecType(0);
		VecType	velocity = VecType(0);
		VecType	offset = VecType(0);
		float angle = 0.0f;
		float sx = 0.25f;
		float sy = 0.25f;
		int state;
	};

	template<typename Scalar>
	struct Action {
		Scalar gas = 0;
		Scalar turn = 0;
	};

	///
	/// \brief The GameState class
	///
	template<typename Texture, typename VecT, typename MapType>
	struct GameState {
		/// Some typedefs using std data types and meta classes:
		typedef std::string	EventData;
		typedef int8_t		EventId;
		typedef int16_t		AgentId;
		typedef meta::Event<AgentId,EventId,EventData> Event;
		typedef car_model::Action<int>		Action;
		typedef int8_t		VisualType;
		typedef VecT		VecType;
		typedef std::shared_ptr<Texture>	TexturePtr;

		/// Policy and event func types
		typedef std::function<Action(AgentId, const GameState&)>	PolicyFunc;
		typedef std::function<void(GameState&, const Event&)>		EventFunc;
		typedef meta::Class<VisualType, PolicyFunc, EventFunc>		MetaClass;

		/// Game State types
		typedef std::vector<Event>									Events;
		typedef car_model::AgentState<VisualType,VecType>			AgentState;
		typedef car_model::ItemState<VecType>						ItemState;
		typedef car_model::ProjectileState<VecType>					ProjectileState;

		/// \brief The Goal entity.
		struct Goal {
			VecType	state;
		};

		/// Meta types:
		/// Entity = Class, State, Policy, Event
		/// Prefab = Class, State
		// Agent
		typedef meta::Entity<MetaClass,AgentState,PolicyFunc,EventFunc>		EntityAgent;
		typedef meta::Prefab<MetaClass,AgentState>							PrefabAgent;
		// Item
		typedef meta::Entity<MetaClass, ItemState,PolicyFunc,EventFunc>		EntityItem;
		typedef meta::Prefab<MetaClass, ItemState>							PrefabItem;
		// Projectile
		typedef meta::Entity<MetaClass,ProjectileState,PolicyFunc,EventFunc>EntityProjectile;
		typedef meta::Prefab<MetaClass,ProjectileState>						PrefabProjectile;

		/// Constant attributes in game state:
		// Entiteettiluokat
		int aiDifficulty = 0;
		const std::vector<MetaClass>	classes;
		const std::vector<TexturePtr>	textures;
		const std::vector<TexturePtr>	texturesBlack;

		/// "Static" entities:
		// Pelin kartta
		std::shared_ptr<MapType>		tileMap;

		/// "Dynamic" entities:
		std::vector<EntityAgent>		agents;
		// Itemit
		std::vector<EntityItem>			items;

		/// "Dynamic" entities, initially empty:
		std::vector<EntityProjectile>	projectiles;
		std::vector<Goal>				goals;
		float totalTime = 0.0f;
		bool isRunning = false;
	};


	///
	/// \brief model::genClasses
	/// \param prefabs	= Entiteettien uokat
	///
	template<size_t N, typename Entity>
	auto genClasses(const std::array<Entity,N>& in) {
		std::vector<Entity> res;
		for(const auto& e : in) {
			res.push_back(e);
		}
		return res;
	}

	///
	/// \brief model::genEntities
	/// \param prefabs	= Instanssoitavat prefabit
	///
	template<typename Entity, typename Prefab>
	auto genEntities(const std::vector<Prefab>& prefabs) {
		std::vector<Entity> res;
		for(const auto& prefab : prefabs) {
			res.push_back(Entity{prefab.type, prefab.state, prefab.type.policy, prefab.type.event});
		}
		return res;
	}

	///
	/// \brief model::genTileMap
	/// \param sx		= Mapin x koko.
	/// \param sy		= Mapin y koko.
	/// \param value	= Mapin elementin oletusarvo.
	///
	template<typename TileType>
	auto genTileMap(std::size_t sx, std::size_t sy, const TileType& value) {
		typedef std::vector< std::vector<TileType> > MapType;
		MapType map;
		for(std::size_t y=0; y<sy; ++y) {
			map.emplace_back();
			map[y].resize(sx, value);
		}
		return std::make_shared<MapType>(map);
	}

	template<typename Map, typename Func>
	void visitRow(Map& map, size_t startX, size_t y, int dx, Func f) {
		const auto MAX = startX+dx;
		if(dx < 0) {
			for(auto x=startX; x>=MAX && x<map[y].size(); --x){
				map[y][x] = f(x,y,map[y][x]);
			}
		} else {
			for(auto x=startX; x<MAX; ++x) {
				map[y][x] = f(x,y,map[y][x]);
			}
		}
	}

	template<typename Map, typename Func>
	void visitCol(Map& map, size_t x, size_t startY, int dy, Func f) {
		const auto MAX = startY+dy;
		if(dy < 0) {
			for(auto y=startY; y>=MAX && y<map.size(); --y) {
				map[y][x] = f(x,y,map[y][x]);
			}
		} else {
			for(auto y=startY; y<MAX; ++y) {
				map[y][x] = f(x,y,map[y][x]);
			}
		}
	}

	template<typename Map, typename Func>
	void visitSquare(Map& map, size_t startX, size_t startY, int dx, int dy, Func f) {
		const auto MAX_X = startX+dx;
		const auto MAX_Y = startY+dy;
		for(auto y=startY; y<MAX_Y; ++y) {
			for(auto x=startX; x<MAX_X; ++x) {
				map[y][x] = f(int(x), int(y), map[y][x]);
			}
		}
	}

	template<typename Map, typename Func>
	void visitSphere(Map& map, size_t x, size_t y, size_t r, Func f) {
		// TODO:
	}


} // End - namespace model
