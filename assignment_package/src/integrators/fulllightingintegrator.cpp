#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &r, Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f L(0.f), beta(1.f);

    Ray ray(r);

    bool specularBounce = false;

    for (int bounces = 0; ; ++bounces) {
 // <<Find next path vertex and accumulate contribution>>
       // <<Intersect ray with scene and store intersection in isect>>
        Intersection isect;
        bool foundIntersection = scene.Intersect(ray, &isect);

        // <<Terminate path if ray escaped or maxDepth was reached>>
        if (!foundIntersection || bounces >= depth)
           break;

//      <<Possibly add emitted light at intersection>>
        if (bounces == 0 ||specularBounce) {
//            <<Add emitted light at path vertex or from the environment>>
            L += (beta * isect.Le(-ray.direction));
        }

//           <<Compute scattering functions and skip over medium boundaries>>
             if (!isect.ProduceBSDF()) {
                 break;
             }

//           <<Sample illumination from lights to find path contribution>>
             L += beta * UniformSampleOneLight(isect, scene, *sampler, ray);


//           <<Sample BSDF to get new path direction>>
             Vector3f wo = -ray.direction, wi;
             Float pdf;
             BxDFType flags;
             Point2f xi = sampler->Get2D();

             Color3f f = isect.bsdf->Sample_f(wo, &wi, xi, &pdf, BSDF_ALL, &flags);

             if (IsBlack(f) || pdf <= FLT_MIN)
                 break;

             beta *= f * AbsDot(wi, isect.normalGeometric) / pdf;
             specularBounce = (flags & BSDF_SPECULAR) != 0;
             ray = isect.SpawnRay(wi);


//           <<Possibly terminate the path with Russian roulette>>
             if (bounces > 3) {
                 float q = std::max(beta.x, std::max(beta.y, beta.z) );
                 if (sampler->Get1D() < q)
                     break;
                 beta /= q;
             }
    }


    return L;
}


Color3f FullLightingIntegrator::UniformSampleOneLight(const Intersection &isect,
                                  Scene &scene, Sampler &sampler, const Ray &ray) const
{
    // randomly select a light
    int nLights = scene.lights.size();
    int light_id = ((float)rand())/RAND_MAX * nLights;

    light_id = std::min(light_id, scene.lights.size()-1);
    const std::shared_ptr<Light> light =  scene.lights[light_id];

    // first ray, w_i
    Vector3f wiW;
    Vector3f woW = -ray.direction;
    float pdf;
    Point2f xi = sampler.Get2D();
    Color3f Li = light->Sample_Li(isect, xi ,&wiW ,&pdf);
    pdf /= nLights;
    Color3f f = isect.bsdf->f(woW, wiW, BxDFType::BSDF_ALL);
    float pdf1_2 = isect.bsdf->Pdf(woW, wiW);

    Intersection temp;
    Ray ray_i = isect.SpawnRay(wiW);
    bool hit2 = scene.Intersect(ray_i, &temp);
    float visible = 1.0f;
    if(hit2 && !temp.objectHit->GetAreaLight()){
        visible = 0.f;
    }

    // second ray, wi
    Vector3f wiW2;
    Point2f xi2 = sampler.Get2D();
    float pdf2;
    BxDFType sampledType2;
    Color3f f2 = isect.bsdf->Sample_f(woW, &wiW2, xi2, &pdf2, BxDFType::BSDF_ALL, &sampledType2);

    Intersection temp2;
    Ray ray_i2 = isect.SpawnRay(wiW2);
    bool hit_light = scene.Intersect(ray_i2, &temp2);
    float visible2 = 0.0f;
    Vector3f Li2(0);
    if(hit_light && temp2.objectHit->areaLight == light ){
        visible2 = 1.f;
        Li2 = temp2.Le(-wiW2);
    }

    float pdf2_2 = light->Pdf_Li(isect, wiW2);
    pdf2_2 /= scene.lights.size();

    float weight1 = PowerHeuristic(1, pdf, 1, pdf1_2);
    float weight2 = PowerHeuristic(1, pdf2, 1, pdf2_2);

    f = f * visible * Li * AbsDot(wiW, isect.normalGeometric) / pdf;
    f2 =  f2 * visible2 * Li2 * AbsDot(wiW2, isect.normalGeometric) / pdf2;

    Color3f col = f*weight1 + f2*weight2;

    return col;

}
