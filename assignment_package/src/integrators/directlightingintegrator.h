#pragma once
#include "integrator.h"

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to
    // its origin, which can only be the camera in a direct lighting
    // integrator (only rays from light sources are considered)
    virtual Color3f Li(const Ray &ray, Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

    Color3f LightingFuction(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, Intersection &isect, int lightId) const;


    inline Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const{
        return (nf * fPdf) / (nf * fPdf + ng * gPdf);
    }
};
