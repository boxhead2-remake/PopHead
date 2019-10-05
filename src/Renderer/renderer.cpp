#include <GL/glew.h>
#include "renderer.hpp"
#include "GameObjects/gameObject.hpp"
#include "Logs/logs.hpp"
#include "buffers.hpp"
#include "openglErrors.hpp"
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <iostream>

namespace ph {

Renderer::Renderer()
	:mViewProjectionMatrix(nullptr)
{
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
		PH_EXIT_GAME("GLEW wasn't initialized correctly!");

	/*GLCheck( const GLubyte* openglVersionInfo = glGetString(GL_VERSION) );
	std::cout << "OpenGL version: " << openglVersionInfo << std::endl;*/

	GLCheck( glEnable(GL_BLEND) );
	GLCheck( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
}

void Renderer::beginScene(Camera& camera)
{
	glClearColor(0.2, 0.3, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	mViewProjectionMatrix = camera.getViewProjectionMatrix4x4();
}

void Renderer::submit(std::shared_ptr<VertexArray>& vao, std::shared_ptr<Shader>& shader, const sf::Transform& transform)
{
	vao->bind();

	shader->bind();
	shader->setUniformMatrix4x4("modelMatrix", transform.getMatrix());
	shader->setUniformMatrix4x4("viewProjectionMatrix", mViewProjectionMatrix);

	GLCheck( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0) );
}

void Renderer::endScene()
{
}

}
