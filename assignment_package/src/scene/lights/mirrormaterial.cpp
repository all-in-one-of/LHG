#include "mirrormaterial.h"
#include "specularbrdf.h"

void MirrorMaterial::ProduceBSDF(Intersection *isect) const
{
    // Important! Must initialize the intersection's BSDF!
    isect->bsdf = std::make_shared<BSDF>(*isect);

    Color3f color = Kr;
//    if(this->textureMap)
//    {
//        color *= Material::GetImageColor(isect->uv, this->textureMap.get());
//    }
//    if(this->normalMap)
//    {
//        isect->bsdf->normal = isect->bsdf->tangentToWorld *  Material::GetImageColor(isect->uv, this->normalMap.get());
//        //Update bsdf's TBN matrices to support the new normal
//        Vector3f tangent, bitangent;
//        CoordinateSystem(isect->bsdf->normal, &tangent, &bitangent);
//        isect->bsdf->UpdateTangentSpaceMatrices(isect->bsdf->normal, tangent, bitangent);
//    }

    isect->bsdf->Add(new SpecularBRDF(color, new FresnelNoOp()));
}
