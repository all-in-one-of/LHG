#include "hirerarchyintegrator.h"

Color3f Hirerarchyintegrator::LightingFuctionLGH(const Ray &ray, Scene &scene, std::shared_ptr<Sampler> sampler,
                                                     Intersection &isect, int light_id, int level) const
{
    const std::shared_ptr<Light> light =  scene.lightsMap.at(level).at(light_id);

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

Color3f Hirerarchyintegrator::Li(const Ray &ray, Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //DONE
    Intersection isect;
    bool hit = scene.Intersect(ray, &isect);
    if(!hit) return Color3f(0.f);

    if(depth==0 || !isect.ProduceBSDF()){
        Color3f lit_col = isect.Le(-ray.direction);
        return lit_col;
    }

    Color3f L(0);

    auto lightingFunc = [&](int level, int light_id, const cy::Point3f &light_position, const cy::Color &light_intensity)
    {

        if( scene.lightsMap.find(level) == scene.lightsMap.end() ||
             scene.lightsMap.at(level).find(light_id) ==   scene.lightsMap.at(level).end() )
        {
            Point3f pos(light_position.x, light_position.y, light_position.z);
            Color3f col(light_intensity.r, light_intensity.g, light_intensity.b);

            scene.CreateLight(level, light_id, pos, col);
        }

        L+=LightingFuctionLGH(ray, scene, sampler, isect, light_id, level);

    };

    cy::Point3f shadingPoint(isect.point.x, isect.point.y, isect.point.z);


    scene.LGH.Light(shadingPoint, 1.01f, lightingFunc);

    Color3f Le =  isect.Le(-ray.direction) ;
    return L + Le;

}
