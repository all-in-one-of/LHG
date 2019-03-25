#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //DONE!
    *pdf = 1;

    // <<Figure out which  is incident and which is transmitted>>
    bool entering = CosTheta(wo) > 0;
    Float etaI = entering ? etaA : etaB;
    Float etaT = entering ? etaB : etaA;

    // <<Compute ray direction for specular transmission>>
    if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
    {
        *pdf = 1;
        *wi = Vector3f(-wo.x, -wo.y, wo.z);
        return Color3f(0.0f);
    }

    *sampledType = type;

    Color3f ft = Color3f(1.f) - fresnel->Evaluate(CosTheta(*wi));

    // <<Account for non-symmetry with transmission to different medium>>
   // return ft;
    return T * ft / AbsCosTheta(*wi);
}
