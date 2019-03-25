#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

glm::vec3 WarpFunctions::squareToDiskUniform(const glm::vec2 &sample)
{
    float radius = glm::sqrt(sample.x);
    float angle = sample.y * Pi*2.0f;
    return glm::vec3( radius * glm::cos(angle),  radius * glm::sin(angle), 0.0f);
}

glm::vec3 WarpFunctions::squareToDiskConcentric(const glm::vec2 &sample)
{
    float a = sample.x*2-1, b = sample.y*2-1;
    float phi, r, u, v;
    if (a > -b) {// region 1 or 2
        if (a > b) { // region 1, also |a| > |b|
            r = a;
            phi = (Pi/4.0f ) * (b/a);
        }else { // region 2, also |b| > |a|
            r = b;
            phi = (Pi/4.0f) * (2 - (a/b));
        }
    }
    else { // region 3 or 4
        if (a < b) { // region 3, also |a| >= |b|, a != 0
            r = -a;
            phi = (Pi/4) * (4 + (b/a));
        } else {  // region 4, |b| >= |a|, but a==0 and b==0 could occur.
            r = -b;
            if (b != 0) phi = (Pi/4) * (6 - (a/b));
            else phi = 0;
        }
    }

    u=r* cos( phi );
    v=r* sin( phi );

    return glm::vec3(u, v , 0);
}

float WarpFunctions::squareToDiskPDF(const glm::vec3 &sample)
{
    return 1.0f/ Pi;
}

glm::vec3 WarpFunctions::squareToSphereUniform(const glm::vec2 &sample)
{
    float z = 1.0f - 2*sample.x;
    float x = glm::cos(2*Pi*sample.y) * glm::sqrt((1-z*z));
    float y = glm::sin(2*Pi*sample.y) * glm::sqrt((1-z*z));

    return glm::vec3(x,y,z);
}

float WarpFunctions::squareToSphereUniformPDF(const glm::vec3 &sample)
{

     return 1.0f/ (Pi * 4);
}

glm::vec3 WarpFunctions::squareToSphereCapUniform(const glm::vec2 &sample, float thetaMin)
{

    float z = 1.0f - sample.x;
    z = 1.0f - (float)thetaMin / 180.0f * z;
    float x = glm::cos(2*Pi*sample.y) * glm::sqrt((1-z*z));
    float y = glm::sin(2*Pi*sample.y) * glm::sqrt((1-z*z));

    return glm::vec3(x,y,z);

}

float WarpFunctions::squareToSphereCapUniformPDF(const glm::vec3 &sample, float thetaMin)
{

    float phi = glm::radians(180-thetaMin);
    return 1.0f / ((1-glm::cos(phi)) * Pi*2);
}

glm::vec3 WarpFunctions::squareToHemisphereUniform(const glm::vec2 &sample)
{
    float pi2 = Pi*2;
    float z = sample.x;
    float x = glm::cos(pi2*sample.y) * glm::sqrt(1-z*z);
    float y = glm::sin(pi2*sample.y) * glm::sqrt(1-z*z);
    return glm::vec3(x,y,z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const glm::vec3 &sample)
{
      return 1.0f/ (Pi * 2);
}

glm::vec3 WarpFunctions::squareToHemisphereCosine(const glm::vec2 &sample)
{
    auto temp = squareToDiskConcentric(sample);
    float x = temp.x;
    float y = temp.y;

    float z = glm::sqrt(1-x*x-y*y);
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const glm::vec3 &sample)
{
    return glm::abs(sample.z) * InvPi;
}
