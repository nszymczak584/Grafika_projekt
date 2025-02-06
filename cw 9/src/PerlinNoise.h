#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <vector>
#include <glm.hpp>

class PerlinNoise {
public:
    PerlinNoise(unsigned int seed = 0);
    float noise(float x, float y, float z);

private:
    std::vector<int> p;
    float fade(float t);
    float lerp(float t, float a, float b);
    float grad(int hash, float x, float y, float z);
};

#endif // PERLINNOISE_H