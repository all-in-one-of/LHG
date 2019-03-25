#include "diffusearealight.h"


Color3f DiffuseAreaLight::R() const
{
    return emittedLight;
}
Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //DONE
    if(twoSided) {
        return emittedLight;
    }else{
        if(  glm::dot(w, isect.normalGeometric) > 0 ) {
            // same side
            return emittedLight;
        }
    }
    return Color3f(0.f);
}


Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    // DONE

//  Get an Intersection on the surface of its Shape by invoking shape->Sample.
    Intersection it = shape->Sample(ref, xi, pdf);

//  Check if the resultant PDF is zero or
//  that the reference Intersection and the resultant Intersection are the same point in space
    if(*pdf == 0.f || glm::length(it.point-ref.point)<FLT_MIN ) {
        return Color3f(0);
    }

//  Set ωi to the normalized vector
//  from the reference Intersection's point to the Shape's intersection point.
    *wi = glm::normalize( it.point - ref.point );

//  Return the light emitted along ωi from our intersection point.
    return L(it, -*wi);
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const {

    return shape->Pdf(ref, wi);
}

