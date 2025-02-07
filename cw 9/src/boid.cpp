#include "Boid.h"
#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <ctime>

const float MAX_SPEED = 1.0f;
const float NEIGHBOR_RADIUS = 2.0f;
const float AVOID_RADIUS = 0.5f;

const float ALIGNMENT_WEIGHT = 3.0f;
const float COHESION_WEIGHT = 4.0f;
const float SEPARATION_WEIGHT = 5.0f;

Boid::Boid(glm::vec3 position, int groupId, int id)
    : position(position), velocity(glm::vec3(1.0f)), groupId(groupId), id(id), horizontalAngle(0.0f), verticalAngle(0.0f) {}
void Boid::checkBounds() {
    float boundary = 5.0f; 
    float boundarydown = 0.0f;

    if (position.x > boundary) {
        position.x = boundary;
        velocity.x *= -1;
    } else if (position.x < boundarydown) {
        position.x = -boundary;
        velocity.x *= -1;
    }

    if (position.y > boundary) {
        position.y = boundary;
        velocity.y *= -1;
    } else if (position.y < -boundary) {
        position.y = -boundary;
        velocity.y *= -1;
    }

    if (position.z > boundary) {
        position.z = boundary;
        velocity.z *= -1;
    } else if (position.z < -boundary) {
        position.z = -boundary;
        velocity.z *= -1;
    }
}

void Boid::update(const std::vector<Boid>& boids) {
    glm::vec3 alignment = align(boids);
    glm::vec3 cohesionent = cohesion(boids);
    glm::vec3 separationent = separation(boids);

    glm::vec3 steering = alignment + cohesionent + separationent;

    float maxSteeringForce = 5.0f; 
    if (glm::length(steering) > maxSteeringForce) {
        steering = glm::normalize(steering) * maxSteeringForce;
    }

    velocity += steering;

    float maxSpeed = 0.01f; 
    if (glm::length(velocity) > maxSpeed) {
        velocity = glm::normalize(velocity) * maxSpeed;
    }

    position += velocity;
    checkBounds();

    float targetHorizontalAngle = atan2(velocity.x, velocity.z);

    float targetVerticalAngle = atan2(-velocity.y, glm::length(glm::vec2(velocity.x, velocity.z)));
    float rotationDamping = 0.03f; 
    horizontalAngle += (targetHorizontalAngle - horizontalAngle) * rotationDamping;
    verticalAngle += (targetVerticalAngle - verticalAngle) * rotationDamping;
}

glm::vec3 Boid::getPosition() const {
    return position;
}

float Boid::getHorizontalAngle() const {
    return horizontalAngle;
}

float Boid::getVerticalAngle() const {
    return verticalAngle;
}
glm::vec3 Boid::getVelocity() const {
    return velocity;
}

int Boid::getGroupId() const {
    return groupId;
}

glm::vec3 Boid::align(const std::vector<Boid>& boids) {
    glm::vec3 avgVelocity(0.0f);
    int count = 0;

    for (const auto& boid : boids) {
        if (&boid == this || boid.groupId != groupId) continue;



        if (glm::distance(position, boid.position) < NEIGHBOR_RADIUS) {
            avgVelocity += boid.velocity;
            count++;
        }
    }

    if (count > 0) {
        avgVelocity /= count;
        if (glm::length(avgVelocity) > 0.0f) {
            avgVelocity = glm::normalize(avgVelocity) * MAX_SPEED;
        }
    }

    return avgVelocity - velocity;
}

glm::vec3 Boid::cohesion(const std::vector<Boid>& boids) {
    glm::vec3 centerOfMass(0.0f);
    int count = 0;

    for (const auto& boid : boids) {
        if (&boid == this || boid.groupId != groupId) continue;


        if (glm::distance(position, boid.position) < NEIGHBOR_RADIUS) {
            centerOfMass += boid.position;
            count++;
        }
    }

    if (count > 0) {
        centerOfMass /= count;
        glm::vec3 direction = centerOfMass - position;
        if (glm::length(direction) > 0.0f) {
            return glm::normalize(direction);
        }
    }

    return glm::vec3(0.0f);
}

glm::vec3 Boid::separation(const std::vector<Boid>& boids) {
    glm::vec3 avoidVector(0.0f);

    for (const auto& boid : boids) {
        if (&boid == this || boid.groupId != groupId) continue;

        float distance = glm::distance(position, boid.position);

        if (distance < AVOID_RADIUS) {
            avoidVector += (position - boid.position) / distance;
        }
    }

    return avoidVector;
}
