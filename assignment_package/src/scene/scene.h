#pragma once
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/lights/light.h>

#include "lgh/cyLightingGrid.h"

#include <QMutex>


class Primitive;
class Material;
class Light;

class Scene
{
public:
    Scene();
    QList<std::shared_ptr<Primitive>> primitives;
    QList<std::shared_ptr<Material>> materials;

    QList<std::shared_ptr<Light>> lights;
    std::vector<std::vector<std::shared_ptr<Light>>> lightsLGH;

    std::map<int, std::map<int, std::shared_ptr<Light>>> lightsMap;

    QMutex mutex;
    Camera camera;
    Film film;

    void SetCamera(const Camera &c);

    void CreateTestScene();

    void Clear();

    void CreateManyLightsScene();
    void CreateLight(int level, int light_id, Point3f pos, Color3f intensity);

    bool Intersect(const Ray& ray, Intersection* isect) const;
    cy::LightingGridHierarchy LGH;

};
