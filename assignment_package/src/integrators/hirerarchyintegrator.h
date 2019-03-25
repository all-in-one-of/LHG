#ifndef HIRERARCHYINTEGRATOR_H
#define HIRERARCHYINTEGRATOR_H

#include "integrator.h"


class Hirerarchyintegrator : public Integrator
{
public:
    Hirerarchyintegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    Color3f LightingFuctionLGH(const Ray &ray, Scene &scene, std::shared_ptr<Sampler> sampler,
                               Intersection &isect, int light_id, int level) const;

    // Evaluate the energy transmitted along the ray back to
    // its origin, which can only be the camera in a direct lighting
    // integrator (only rays from light sources are considered)
    virtual Color3f Li(const Ray &ray, Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

};

#endif // HIRERARCHYINTEGRATOR_H
