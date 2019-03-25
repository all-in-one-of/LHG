#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //DONE
    Intersection isect;
    bool hit = scene.Intersect(ray, &isect);
    if(!hit) return Color3f(0.f);

    if(depth==0 || !isect.ProduceBSDF()){
        Color3f lit_col = isect.Le(-ray.direction);
        return lit_col;
    }

    // get w_i
    Vector3f wiW;
    Point2f xi = sampler->Get2D();
    float pdf;
    BxDFType sampledType;
    Color3f col = isect.bsdf->Sample_f(-ray.direction, &wiW, xi, &pdf, BxDFType::BSDF_ALL, &sampledType);
    if( pdf < FLT_MIN ) {
        return col;
    }

    Ray ray_i = isect.SpawnRay(wiW);
    col *= Li(ray_i, scene, sampler, depth-1) * AbsDot(wiW, isect.normalGeometric);
    col /= pdf;

    return col + isect.Le(-ray.direction);
}
