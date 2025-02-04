#include "pch.hpp"
#include "sliderWidget.hpp"
#include "Renderer/renderer.hpp"

namespace ph {

SliderWidget::SliderWidget(const char* name)
	:Widget(name)
	,mSliderValue(50.f)
	,mSliderMaxValue(100.f)
{
	auto* icon = new Widget("sliderIcon");
	icon->setSize({0.15f, 1.f});
	mIconWidget = addChildWidget(icon);
}

void SliderWidget::setIconSize(Vec2 size)
{
	mIconWidget->setSize(size);
}

void SliderWidget::setIconTexture(Texture* texture)
{
	mIconWidget->setTexture(texture);
}

void SliderWidget::updateCurrent(float dt, u8 z)
{
	mIconWidget->setCenterPosition({mSliderValue / (mSliderMaxValue - mSliderMinValue), 0.5f});

	if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) 
	{
		auto mousePos = Cast<Vec2>(sf::Mouse::getPosition(*sWindow));
		Vec2 barScreenPos = getScreenPosition();
		Vec2 barScreenSize = getScreenSize();
		bool isMouseOnSlider = FloatRect(barScreenPos, barScreenSize).contains(mousePos);
		if(isMouseOnSlider) 
		{
			if(mousePos.x <= barScreenPos.x) 
			{
				mSliderValue = mSliderMinValue;
			}
			else if(mousePos.x >= barScreenPos.x + getScreenSize().x) 
			{
				mSliderValue = mSliderMaxValue;
			}
			else 
			{
				float normalizedValue = (mousePos.x - barScreenPos.x) / getScreenSize().x;
				mSliderValue = mSliderMinValue + (normalizedValue * (mSliderMaxValue - mSliderMinValue));
			}
		}
	}
}

}

