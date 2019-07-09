#pragma once

#include "EntityComponentSystem/object.hpp"

namespace ph {

class CollisionDebugSettings;
class CollisionBody;

class CollisionDebugRect : public Object
{
public:
    CollisionDebugRect(GameData* gameData, sf::FloatRect rect, CollisionBody* owner);

    void move(sf::Vector2f velocity) { mShape.move(velocity); }
    void setPosition(sf::Vector2f position) { mShape.setPosition(position); }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void setColor(sf::Color color) { mShape.setFillColor(color); }

    void updateColor() const;
    bool shouldDisplay() const;

private:
    mutable sf::RectangleShape mShape;
    CollisionBody* mOwner;
};

}
