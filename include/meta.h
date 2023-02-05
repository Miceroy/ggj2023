#pragma once

///
/// META: Metaluokat mallin käyttöön:
/// - Event<AgentId,EventId,EventData>
/// - Class<VisualType, PolicyFunc, EventFunc>
/// - Entity<ClassType, State, PolicyFunc, EventFunc>
/// - Prefab<ClassType, State>
///

namespace meta {
	template<typename AgentId, typename EventId, typename EventData>
	struct Event {
		EventId		id = -1;
		AgentId		sender = -1;
		AgentId		receiver = -1;
		EventData	data;
	};


	template<typename VisualType, typename PolicyFunc, typename EventFunc>
	struct Class {
		VisualType		visualId	= -1;
		PolicyFunc		policy		= 0;
		EventFunc		event		= 0;
	};

	template<typename ClassType, typename State, typename PolicyFunc, typename EventFunc>
	struct Entity {
		ClassType		type;
		State			state;
		PolicyFunc		policy		= 0;
		EventFunc		event		= 0;
		bool			isAlive		= true;
		float			coolDownTimer = 0.0f; // When cooldown timer > 0, object policy is not called.
	};

	template<typename ClassType, typename State>
	struct Prefab {
		ClassType		type;
		State			state;
	};

	// Prefab{ClassType{}, State{}}
}
