#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

// Material class
class Material {

	public:
		
		// constructors
		Material(); // default
		
		// given ambient, diffuse, specular, and shininess
		Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float S);

		~Material() {}

		// Members
		glm::vec3 ka; // ambience
		glm::vec3 kd; // diffuse
		glm::vec3 ks; // specular
		float s; // shininess

};

#endif