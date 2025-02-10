#include "Boid.h"
#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <ctime>
#include "Collision.h"

const float MAX_SPEED = 1.0f;
const float NEIGHBOR_RADIUS = 2.0f;
const float AVOID_RADIUS = 0.5f;

const float ALIGNMENT_WEIGHT = 3.0f;
const float COHESION_WEIGHT = 4.0f;
const float SEPARATION_WEIGHT = 5.0f;

Boid::Boid(glm::vec3 position, int groupId, int id)
    : position(position), velocity(glm::vec3(1.0f)), groupId(groupId), id(id), horizontalAngle(0.0f), verticalAngle(0.0f) {
    bbox.min = glm::vec3(0.0f);
    bbox.max = glm::vec3(0.0f);
}
void Boid::checkBounds() {
    const glm::vec3 minBounds(-5.0f, 1.45f, -4.0f); // Dolne granice
    const glm::vec3 maxBounds(4.5f, 6.0f, 2.0f);    // Górne granice
    if (position.x >= maxBounds.x) {
        position.x = maxBounds.x;
        velocity.x *= -1;
    }
    else if (position.x <= minBounds.x) {
        position.x = minBounds.x;
        velocity.x *= -1;
    }

    if (position.y >= maxBounds.y) {
        position.y = maxBounds.y;
        velocity.y *= -1;
    }
    else if (position.y <= minBounds.y) {
        position.y = minBounds.y;
        velocity.y *= -1;
    }

    if (position.z >= maxBounds.z) {
        position.z = maxBounds.z;
        velocity.z *= -1;
    }
    else if (position.z <= minBounds.z) {
        position.z = minBounds.z;
        velocity.z *= -1;
    }
}


void Boid::update(const std::vector<Boid>& boids, const std::vector<CollidableObject>& collidableObjects) {
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
    for (const auto& obstacle : collidableObjects) {
        handleCollision(obstacle.bbox);
    }

    float targetHorizontalAngle = atan2(velocity.x, velocity.z);

    float targetVerticalAngle = atan2(-velocity.y, glm::length(glm::vec2(velocity.x, velocity.z)));
    float rotationDamping = 0.1f;
    horizontalAngle += (targetHorizontalAngle - horizontalAngle) * rotationDamping;
    verticalAngle += (targetVerticalAngle - verticalAngle) * rotationDamping;

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) *
        glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f));

}
BoundingBox Boid::getBoundingBox() const {
    return bbox;
}

void Boid::setBoundingBox(const BoundingBox& newBBox) {
    bbox = newBBox;
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

bool Boid::checkCollision(const BoundingBox& bbox1, const BoundingBox& bbox2) {
    return (bbox1.min.x-0.1f <= bbox2.max.x && bbox1.max.x+0.1f >= bbox2.min.x) &&
        (bbox1.min.y-0.1f <= bbox2.max.y+0.1f && bbox1.max.y+0.1f >= bbox2.min.y) &&
        (bbox1.min.z-0.1f <= bbox2.max.z+0.1f && bbox1.max.z+0.1f >= bbox2.min.z);
}
void Boid::handleCollision(const BoundingBox& obstacle) {
    if (!checkCollision(this->bbox, obstacle)) return;

    glm::vec3 penetrationDepth = glm::vec3(0.0f);
    penetrationDepth.x = std::min(std::abs(position.x - obstacle.min.x), std::abs(position.x - obstacle.max.x));
    penetrationDepth.y = std::min(std::abs(position.y - obstacle.min.y), std::abs(position.y - obstacle.max.y));
    penetrationDepth.z = std::min(std::abs(position.z - obstacle.min.z), std::abs(position.z - obstacle.max.z));

    if (penetrationDepth.x < penetrationDepth.y && penetrationDepth.x < penetrationDepth.z) {
        if (position.x < (obstacle.min.x + obstacle.max.x) / 2.0f) {
            position.x = obstacle.min.x - 0.1f;
        }
        else {
            position.x = obstacle.max.x + 0.1f;
        }
        velocity.x = -velocity.x;
    }
    else if (penetrationDepth.y < penetrationDepth.x && penetrationDepth.y < penetrationDepth.z) {
        if (position.y < (obstacle.min.y + obstacle.max.y) / 2.0f) {
            position.y = obstacle.min.y - 0.1f;
        }
        else {
            position.y = obstacle.max.y + 0.1f; 
        }
        velocity.y = -velocity.y;
    }
    else {
        if (position.z < (obstacle.min.z + obstacle.max.z) / 2.0f) {
            position.z = obstacle.min.z - 0.1f; 
        }
        else {
            position.z = obstacle.max.z + 0.1f;
        }
        velocity.z = -velocity.z;
    }
 


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

void Boid::applyForce(const glm::vec3& force,const std::vector<CollidableObject>& collidableObjects) {
    velocity += force;

    float maxSpeed = 0.01f;
    if (glm::length(velocity) > maxSpeed) {
        velocity = glm::normalize(velocity) * maxSpeed;
    }

    position += velocity;

    checkBounds();
    for (const auto& obstacle : collidableObjects) {
        handleCollision(obstacle.bbox);

    }

}