#include "physicsEngine.hpp"

#include "collisionBody.hpp"

#include <iostream>

using PopHead::Physics::PhysicsEngine;
using PopHead::Physics::CollisionBody;

void PhysicsEngine::addStaticBody(CollisionBody* staticBodyPtr)
{
    mStaticBodies.emplace_back(std::move(staticBodyPtr));
}

void PhysicsEngine::addKinematicBody(CollisionBody* kinematicBodyPtr)
{
    mKinematicBodies.emplace_back(std::move(kinematicBodyPtr));
}

void PhysicsEngine::removeStaticBody(CollisionBody* staticBodyPtr)
{
    for(auto it = mStaticBodies.begin(); it != mStaticBodies.end(); ++it){
        if(*it == staticBodyPtr){
            mStaticBodies.erase(it);
            break;
        }
    }
}

void PhysicsEngine::removeKinematicBody(CollisionBody* kinematicBodyPtr)
{
    for(auto it = mKinematicBodies.begin(); it != mKinematicBodies.end(); ++it){
        if(*it == kinematicBodyPtr){
            mKinematicBodies.erase(it);
            break;
        }
    }
}

void PhysicsEngine::clear() noexcept
{
	mStaticBodies.clear();
	mKinematicBodies.clear();
}

void PhysicsEngine::update(sf::Time delta)
{
    for(auto kinematicBody : mKinematicBodies)
    {
		handleStaticCollisionsFor(kinematicBody);
		kinematicBody->updateOwnerPosition();
		kinematicBody->setPreviousPositionToCurrentPosition();
    }
}

void PhysicsEngine::handleStaticCollisionsFor(CollisionBody* kinematicBody)
{
	for (const auto& staticBody : mStaticBodies) {
		if (isThereCollision(kinematicBody->mRect, staticBody->mRect))
			mStaticCollisionHandler.handleStaticCollision(kinematicBody, staticBody);
	}
}

bool PhysicsEngine::isThereCollision(sf::FloatRect A, sf::FloatRect B)
{
	//AABB collision detection algorithm
	return(
	A.left < B.left + B.width &&
	A.left + A.width > B.left &&
	A.top < B.top + B.height &&
	A.top + A.height > B.top);
}