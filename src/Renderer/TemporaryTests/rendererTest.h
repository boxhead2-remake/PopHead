#pragma once

#include "Renderer/renderer.hpp"

#define MORE_THEN_32_TEXTURES 1

namespace ph {

void rendererTest()
{
	/* 1  */ static Texture faceTex("resources/textures/test/awesomeface.png");
	/* 2  */ static Texture containerTex("resources/textures/test/container.jpg");
	/* 3  */ static Texture container2Tex("resources/textures/test/container2.png");
	/* 4  */ static Texture container2SpecTex("resources/textures/test/container2_specular.png");
	/* 5  */ static Texture floorTex("resources/textures/test/floor.jpg");
	/* 6  */ static Texture grassTex("resources/textures/test/grass.png");
	/* 7  */ static Texture sb1Tex("resources/textures/test/sb1.jpg");
	/* 8  */ static Texture sb2Tex("resources/textures/test/sb2.jpg");
	/* 9  */ static Texture sb3Tex("resources/textures/test/sb3.jpg");
	/* 10 */ static Texture sb4Tex("resources/textures/test/sb4.jpg");
	/* 11 */ static Texture sb5Tex("resources/textures/test/sb5.jpg");
	/* 12 */ static Texture wallTex("resources/textures/test/wall.jpg");
	/* 13 */ static Texture awsdTex("resources/textures/others/awsdHint.png");
	/* 14 */ static Texture backslashTex("resources/textures/others/backslashHint.png");
	/* 15 */ static Texture barTex("resources/textures/others/bar.png");
	/* 16 */ static Texture baseballTex("resources/textures/others/baseballStick.png");
	/* 17 */ static Texture bulletTex("resources/textures/others/bullet.png");
	/* 18 */ static Texture bulletGroundTex("resources/textures/others/bulletGround.png");
	/* 19 */ static Texture enterTex("resources/textures/others/enterHint.png");
	/* 20 */ static Texture gateTex("resources/textures/others/gate.png");
	/* 21 */ static Texture heartTex("resources/textures/others/heart.png");
	/* 22 */ static Texture leverTex("resources/textures/others/lever.png");
	/* 23 */ static Texture medkitTex("resources/textures/others/medkit.png");
	/* 24 */ static Texture pistolTex("resources/textures/others/pistol.png");
	/* 25 */ static Texture spaceTex("resources/textures/others/pressSpaceHint.png");
	/* 26 */ static Texture zombieHeadTex("resources/textures/others/zombieHead.png");
	/* 27 */ static Texture arcadeCounBackTex("resources/textures/gui/arcadeCountersBackground.png");
	/* 28 */ static Texture arenaButtonTex("resources/textures/gui/arenaButton.png");
	/* 29 */ static Texture bunkerTex("resources/textures/gui/bunker.jpg");
	/* 30 */ static Texture creditsButtonTex("resources/textures/gui/creditsButton.png");
	/* 31 */ static Texture exitButtonTex("resources/textures/gui/exitButton.png");
	/* 32 */ static Texture mirrorSpTex("resources/textures/gui/mirrorSpeechBubble.png");
	/* 33 */ static Texture optionsButtonTex("resources/textures/gui/optionsButton.png");
	/* 34 */ static Texture sewageTex("resources/textures/gui/sewage.jpg");

	Renderer::submitQuad(&sb5Tex, nullptr, nullptr, sf::Vector2f(-1500.f, 0.f), sf::Vector2f(3000.f, 3000.f), 100, 0.f);

	for(int i = 0; i < 2500; ++i) {
		Renderer::submitQuad(&faceTex, nullptr, nullptr, sf::Vector2f(-550.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 45.f);
		Renderer::submitQuad(&containerTex, nullptr, &sf::Color::Red, sf::Vector2f(-500.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 123.f);
		Renderer::submitQuad(&container2Tex, nullptr, nullptr, sf::Vector2f(-450.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&container2SpecTex, nullptr, nullptr, sf::Vector2f(-400.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&floorTex, nullptr, nullptr, sf::Vector2f(-350.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&grassTex, nullptr, nullptr, sf::Vector2f(-300.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&sb1Tex, &IntRect(5, 5, 10, 10), nullptr, sf::Vector2f(-250.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&sb2Tex, nullptr, nullptr, sf::Vector2f(-200.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10,0.f);
		Renderer::submitQuad(&sb3Tex, nullptr, nullptr, sf::Vector2f(-150.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10,0.f);
		Renderer::submitQuad(&sb4Tex, nullptr, nullptr, sf::Vector2f(-100.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10,0.f);
		Renderer::submitQuad(&sb5Tex, nullptr, nullptr, sf::Vector2f(-50.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&wallTex, nullptr, nullptr, sf::Vector2f(0.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&awsdTex, nullptr, nullptr, sf::Vector2f(50.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&backslashTex, nullptr, nullptr, sf::Vector2f(100.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&barTex, nullptr, &sf::Color::Yellow, sf::Vector2f(150.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&baseballTex, nullptr, nullptr, sf::Vector2f(200.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&bulletTex, nullptr, nullptr, sf::Vector2f(250.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&bulletGroundTex, nullptr, nullptr, sf::Vector2f(300.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&enterTex, nullptr, nullptr, sf::Vector2f(350.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
#if MORE_THEN_32_TEXTURES
		Renderer::submitQuad(&gateTex, nullptr, nullptr, sf::Vector2f(400.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&heartTex, nullptr, nullptr, sf::Vector2f(450.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&leverTex, nullptr, nullptr, sf::Vector2f(500.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&medkitTex, nullptr, nullptr, sf::Vector2f(550.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&pistolTex, nullptr, nullptr, sf::Vector2f(600.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&spaceTex, nullptr, nullptr, sf::Vector2f(650.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&zombieHeadTex, nullptr, nullptr, sf::Vector2f(700.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&arcadeCounBackTex, nullptr, nullptr, sf::Vector2f(750.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&arenaButtonTex, nullptr, nullptr, sf::Vector2f(800.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&bunkerTex, nullptr, nullptr, sf::Vector2f(850.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&creditsButtonTex, nullptr, nullptr, sf::Vector2f(900.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&exitButtonTex, nullptr, nullptr, sf::Vector2f(950.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&mirrorSpTex, nullptr, nullptr, sf::Vector2f(1050.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&optionsButtonTex, nullptr, nullptr, sf::Vector2f(1100.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
		Renderer::submitQuad(&sewageTex, nullptr, nullptr, sf::Vector2f(1000.f, ( float) i * 50.f), sf::Vector2f(20.f, 30.f), 10, 0.f);
#endif
	}
}

}