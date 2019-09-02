#include "mouseManager.hpp"
#include "gameData.hpp"
#include "eventLoop.hpp"

namespace ph {

auto MouseManager::getMousePosition() const -> sf::Vector2i
{
    return sf::Mouse::getPosition(mGameData->getRenderWindow());
}

bool MouseManager::hasMouseJustMoved() const
{
    return EventLoop::hasMouseJustMoved();
}

bool MouseManager::isMouseButtonJustPressed(sf::Mouse::Button button) const
{
    return (EventLoop::isMouseButtonJustPressed() && EventLoop::getMousePressedButton() == button);
}

bool MouseManager::isMouseButtonJustReleased(sf::Mouse::Button button) const
{
	return (EventLoop::isMouseButtonJustReleased() && EventLoop::getMouseReleasedButton() == button);
}

}