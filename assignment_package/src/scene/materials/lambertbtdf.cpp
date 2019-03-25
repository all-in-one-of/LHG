#include "lambertbtdf.h"
#include <warpfunctions.h>


Color3f LambertBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //DONE
    return R * InvPi;
}

Color3f LambertBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //DONE
    // *wi = -WarpFunctions::squareToHemisphereCosine(u);
     *wi = -WarpFunctions::squareToHemisphereUniform(u);
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float LambertBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //DONE
    return wi.z * InvPi; // cos(theta) / pi
}
