#ifndef BOID_H
#define BOID_H
#include "glm.hpp"
#include "collision.h"
#include <vector>

class Boid {
public:
    Boid(glm::vec3 position, int groupId, int id);
    void update(const std::vector<Boid>& boids, const std::vector<CollidableObject>& collidableObjects);
    void checkBounds();
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    int getGroupId() const;
    int getid() const;
    bool checkCollision(const BoundingBox& bbox1, const BoundingBox& bbox2);
    void handleCollision(const BoundingBox& obstacle);
    void applyForce(const glm::vec3& force, const std::vector<CollidableObject>& collidableObjects);

    float getVerticalAngle() const;
    float getHorizontalAngle() const;
    BoundingBox getBoundingBox() const;
    void setBoundingBox(const BoundingBox& bbox);

private:
    glm::vec3 position;
    glm::vec3 velocity;
    int groupId;
    int id;

    float horizontalAngle = 0.0f;
    float verticalAngle = 0.0f;
    glm::vec3 align(const std::vector<Boid>& boids);
    glm::vec3 cohesion(const std::vector<Boid>& boids);
    glm::vec3 separation(const std::vector<Boid>& boids);
    BoundingBox bbox;
};


#endif // BOID_H