#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Light {

	public:

		glm::vec3 position;

		glm::vec3 color;

		Light(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}

};

#endif