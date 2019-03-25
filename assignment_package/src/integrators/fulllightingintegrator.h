#pragma once
#include "integrator.h"

class FullLightingIntegrator : public Integrator
{
public:
    FullLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to
    // its origin using multiple importance sampling
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;
    inline Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const{
        Float f = nf * fPdf, g = ng * gPdf;
        if((f * f + g * g) < FLT_MIN ) return 0.f;
        return (f * f) / (f * f + g * g);

    }

    Color3f UniformSampleOneLight(const Intersection &it,
                                  const Scene &scene, Sampler &sampler, const Ray &ray) const;

};

