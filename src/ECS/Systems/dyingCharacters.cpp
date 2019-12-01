#include "dyingCharacters.hpp"
#include "ECS/Components/charactersComponents.hpp"
#include "ECS/Components/graphicsComponents.hpp"
#include "ECS/Components/physicsComponents.hpp"
#include "ECS/Components/animationComponents.hpp"

namespace ph::system {

	void DyingCharacters::update(float seconds)
	{
		makeCharactersDie();
		playDyingAnimation(seconds);
	}

	void DyingCharacters::makeCharactersDie() const
	{
		auto view = mRegistry.view<component::Health>();
		for(auto entity : view)
		{
			const auto& health = view.get(entity);
			if(health.healthPoints <= 0) 
			{
				if(mRegistry.has<component::Color>(entity))
				{
					bool isPlayer = mRegistry.has<component::Player>(entity);

					mRegistry.assign<component::TimeToFadeOut>(entity);

					mRegistry.remove<component::Health>(entity);
					mRegistry.remove<component::Killable>(entity);
					mRegistry.remove<component::KinematicCollisionBody>(entity);
					if(!isPlayer)
						mRegistry.remove<component::Damage>(entity);

					auto& z = mRegistry.get<component::Z>(entity);
					z.z = isPlayer ? 96 : 97;

					if(isPlayer) {
						auto deathCameraEntity = mRegistry.create();
						component::Camera camera;
						camera.camera = mRegistry.get<component::Camera>(entity).camera;
						camera.priority = 2;
						mRegistry.assign<component::Camera>(deathCameraEntity, camera);
					}
					
					auto& animation = mRegistry.get<component::AnimationData>(entity);
					animation.currentStateName = "dead";
				}
				else
					mRegistry.assign<component::TaggedToDestroy>(entity);
			}
		}
	}

	void DyingCharacters::playDyingAnimation(float seconds) const
	{
		auto view = mRegistry.view<component::TimeToFadeOut, component::Color>();
		for(auto entity : view)
		{
			auto& [timeToFadeOut, color] = view.get<component::TimeToFadeOut, component::Color>(entity);
			timeToFadeOut.seconds += seconds;
			if(timeToFadeOut.seconds > 10.f)
				mRegistry.assign<component::TaggedToDestroy>(entity);
			color.color.a = static_cast<unsigned char>(255.f - (timeToFadeOut.seconds * 25.5f));
		}
	}
}
