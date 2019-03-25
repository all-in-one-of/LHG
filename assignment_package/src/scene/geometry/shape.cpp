#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //DONE
  //  *pdf = 1.0f / Area();
    Intersection isectLight = Sample(xi, pdf);
    Vector3f wi = glm::normalize(isectLight.point - ref.point);

    // convert the PDF
    float tmp = AbsDot(isectLight.normalGeometric, -wi) * Area();
    if(tmp==0.f) {
        *pdf = 0.f;
    }else{
        *pdf = glm::distance2(ref.point, isectLight.point) / tmp;
    }

    return isectLight;
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi) const
{
    // convert the PDF
    Ray ray =ref.SpawnRay(wi);
    Intersection it;
    bool hit = Intersect(ray, &it);
    if(!hit) return 0.f;
    if( (AbsDot(it.normalGeometric, -wi) * Area()) == 0.0f ) return 0.0f;

    float pdf = glm::distance2(ref.point, it.point) / (AbsDot(it.normalGeometric, -wi) * Area());

    return pdf;
}
