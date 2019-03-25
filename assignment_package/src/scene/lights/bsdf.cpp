#include "bsdf.h"
#include <warpfunctions.h>
#include "stdlib.h"

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//DONE: Properly set worldToTangent and tangentToWorld
    : worldToTangent(glm::transpose(glm::mat3(isect.tangent, isect.bitangent, isect.normalGeometric))),
      tangentToWorld(glm::mat3(isect.tangent, isect.bitangent, isect.normalGeometric)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //DONE: Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld = glm::mat3(t,b,n);
    worldToTangent = glm::transpose(tangentToWorld);
}


//
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //DONE
    Color3f col(0);
    int numMatched = BxDFsMatchingFlags(flags);
    for(int i=0; i<numBxDFs; i++) {
        if( !bxdfs[i]->MatchesFlags(flags) ) continue; // not the matched type
        col += bxdfs[i]->f(worldToTangent*woW, worldToTangent*wiW);
    }
    col = col * (float)(1.0/ numMatched);
    return col;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //DONE
    // refer to
    // http://www.pbr-book.org/3ed-2018

    // Use the input random number to select
    // one of our BxDFs that matches the _type_ flags.
    int numMatched = BxDFsMatchingFlags(type);
    if (numMatched == 0) {
        *pdf = 0;
        return Color3f(0);
    }

    // <<Get BxDF pointer for chosen component>>
    float rand_u = (float)rand() / RAND_MAX; // [0,1)

    int comp = std::min((int)std::floor(rand_u * numMatched), numMatched - 1);
    BxDF *bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < numBxDFs; ++i) {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }
    if(!bxdf){
        bxdf = bxdfs[0];
    }
    *sampledType = bxdf->type;

    Vector3f wo = glm::normalize(worldToTangent * woW);
    Vector3f wi;
    // << Get wi from the chosen BxDF >>
    Color3f col = bxdf->Sample_f(wo, &wi, xi, pdf, sampledType);
    *wiW = glm::normalize(tangentToWorld * wi);

    if(*pdf == 0.f)
        return Color3f(0);

    // if the sampled is specular
    if( bxdf->MatchesFlags(BSDF_SPECULAR) ) {
        return col;
    }

    // Iterate over all BxDFs that we DID NOT select above
    for(int i=0; i<numBxDFs; i++) {
        if(comp==i) continue; // this is the chosen sampler
        if( !bxdfs[i]->MatchesFlags(type) ) continue; // not the matched type
        *pdf += bxdfs[i]->Pdf(wo, wi);
        col += bxdfs[i]->f(wo, wi);
    }
    *pdf = (*pdf) / numMatched;

    return col;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    float pdf = 0.f;
    int numMatched = BxDFsMatchingFlags(flags);
    for(int i=0; i<numBxDFs; i++) {
        if( !bxdfs[i]->MatchesFlags(flags) ) continue; // not the matched type
        pdf += bxdfs[i]->Pdf(worldToTangent*woW, worldToTangent*wiW);
    }
    pdf = pdf / numMatched;


    return pdf;
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //DONE
    *wi = glm::normalize(WarpFunctions::squareToHemisphereUniform(xi));
    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
