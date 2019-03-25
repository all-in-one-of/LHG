#include "directlightingintegrator.h"


Color3f DirectLightingIntegrator::LightingFuction(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, Intersection &isect, int light_id) const
{
    const std::shared_ptr<Light> light =  scene.lights[light_id];

    // first ray, w_i
    Vector3f wiW;
    Vector3f woW = -ray.direction;
    float pdf;
    Point2f xi = sampler->Get2D();
    Color3f Li = light->Sample_Li(isect, xi ,&wiW ,&pdf);
    //pdf /= scene.lights.size();
    Color3f f = isect.bsdf->f(woW, wiW, BxDFType::BSDF_ALL);

    Intersection temp;
    Ray ray_i = isect.SpawnRay(wiW);
    bool hit2 = scene.Intersect(ray_i, &temp);
    float visible = 1.0f;
    if(hit2 && !temp.objectHit->GetAreaLight()){
        visible = 0.f;
    }

    f = f * visible * Li * AbsDot(wiW, isect.normalGeometric) / pdf;
    return f;
}


Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //DONE
    Intersection isect;
    bool hit = scene.Intersect(ray, &isect);
    if(!hit) return Color3f(0.f);

    if(depth==0 || !isect.ProduceBSDF()){
        Color3f lit_col = isect.Le(-ray.direction);
        return lit_col;
    }

    int nLights =  scene.lights.size();
    Color3f L(0);
    for(int i=0; i<nLights; i++) {
        L += LightingFuction(ray, scene, sampler, isect, i);
    }

    Color3f Le =  isect.Le(-ray.direction) ;
    return L + Le;

}
