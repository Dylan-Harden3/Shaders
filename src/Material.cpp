#include "Material.h"


Material::Material() {

	this->ka = glm::vec3(0.0f, 0.0f, 0.0f);
	this->kd = glm::vec3(0.0f, 0.0f, 0.0f);
	this->ks = glm::vec3(0.0f, 0.0f, 0.0f);
	this->s = 0.0f;

}

Material::Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float S) : ka(KA), kd(KD), ks(KS), s(S) {}