#pragma once

#include "ECS/system.hpp"

namespace ph {
	class AIManager;
	class ThreadPool;
}

namespace ph::system {

	class ZombieSystem : public System 
	{
	public:
		ZombieSystem(entt::registry&, const AIManager*, ThreadPool&);

		void update(float dt) override;

		inline static bool freezeZombies = false;

	private:
		const AIManager* mAIManager;
		ThreadPool& mThreadPool;
	};
}
