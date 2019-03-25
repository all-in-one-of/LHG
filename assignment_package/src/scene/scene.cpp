#include <scene/scene.h>

#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/lights/diffusearealight.h>


Scene::Scene()
{}

void Scene::SetCamera(const Camera &c)
{
    camera = Camera(c);
    camera.create();
    film.SetDimensions(c.width, c.height);
}

bool Scene::Intersect(const Ray &ray, Intersection *isect) const
{
    bool result = false;
    for(std::shared_ptr<Primitive> p : primitives)
    {
        Intersection testIsect;
        if(p->Intersect(ray, &testIsect))
        {
            if(testIsect.t < isect->t || isect->t < 0)
            {
                *isect = testIsect;
                result = true;
            }
        }
    }
    return result;
}

void Scene::CreateTestScene()
{
    //Floor
    //Area light
    //Figure in front of light

    auto matteWhite = std::make_shared<MatteMaterial>(Color3f(1,1,1), 0, nullptr, nullptr);
    auto matteRed = std::make_shared<MatteMaterial>(Color3f(1,0,0), 0, nullptr, nullptr);
    auto matteGreen = std::make_shared<MatteMaterial>(Color3f(0,1,0), 0, nullptr, nullptr);

    // Floor, which is a large white plane
    auto floor = std::make_shared<SquarePlane>();
    floor->transform = Transform(Vector3f(0,0,0), Vector3f(-90,0,0), Vector3f(10,10,1));
    auto floorPrim = std::make_shared<Primitive>(floor);
    floorPrim->material = matteWhite;
    floorPrim->name = QString("Floor");

    // Light source, which is a diffuse area light with a large plane as its shape
    auto lightSquare = std::make_shared<SquarePlane>();
    lightSquare->transform = Transform(Vector3f(0,2.5f,5), Vector3f(0,180,0), Vector3f(8, 5, 1));
    auto lightSource = std::make_shared<DiffuseAreaLight>(lightSquare->transform, Color3f(1,1,1) * 2.f, lightSquare);
    auto lightPrim = std::make_shared<Primitive>(lightSquare, nullptr, lightSource);
    lightPrim->name = QString("Light Source");
    lightSource->name = QString("Light Source 1");

    // Light source 2, which is a diffuse area light with a large plane as its shape
    auto lightSquare2 = std::make_shared<SquarePlane>();
    lightSquare2->transform = Transform(Vector3f(5,2.5f,0), Vector3f(0,90,0), Vector3f(8, 5, 1));
    auto lightSource2 = std::make_shared<DiffuseAreaLight>(lightSquare2->transform, Color3f(0.9,1,0.7) * 2.f, lightSquare2, true);
    auto lightPrim2 = std::make_shared<Primitive>(lightSquare2, nullptr, lightSource2);
    lightPrim2->name = QString("Light Source 2");
    lightSource2->name = QString("Light Source 2");

    // Shadow casting shape, which is a red sphere
    auto sphere = std::make_shared<Sphere>();
    sphere->transform = Transform(Vector3f(0,1,0), Vector3f(0,0,0), Vector3f(1,1,1));
    auto spherePrim = std::make_shared<Primitive>(sphere);
    spherePrim->material = matteRed;
    spherePrim->name = QString("Red Sphere");

    // Back wall, which is a green rectangle
    auto greenWall = std::make_shared<SquarePlane>();
    greenWall->transform = Transform(Vector3f(-5,2.5f,0), Vector3f(0,90,0), Vector3f(10, 5, 1));
    auto greenWallPrim = std::make_shared<Primitive>(greenWall);
    greenWallPrim->material = matteGreen;
    greenWallPrim->name = QString("Wall");


    primitives.append(floorPrim);
    primitives.append(lightPrim);
    primitives.append(lightPrim2);
    primitives.append(spherePrim);
    primitives.append(greenWallPrim);

    lights.append(lightSource);
    lights.append(lightSource2);

    for(std::shared_ptr<Primitive> p : primitives)
    {
        p->shape->create();
    }

    camera = Camera(400, 400, Point3f(5, 8, -5), Point3f(0,0,0), Vector3f(0,1,0));
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.create();
    film = Film(400, 400);
}

void Scene::Clear()
{
    // These lists contain shared_ptrs
    // so the pointers will be freed
    // if appropriate when we clear the lists.
    primitives.clear();
    lights.clear();
    materials.clear();
    camera = Camera();
    film = Film();
}


void Scene::CreateManyLightsScene() {
    //Floor
    //Area lights
    //Figure in front of light

    auto matteWhite = std::make_shared<MatteMaterial>(Color3f(1,1,1), 0, nullptr, nullptr);
    auto matteRed = std::make_shared<MatteMaterial>(Color3f(1,0,0), 0, nullptr, nullptr);
    auto matteGreen = std::make_shared<MatteMaterial>(Color3f(0,1,0), 0, nullptr, nullptr);

    // Floor, which is a large white plane
    auto floor = std::make_shared<SquarePlane>();
    floor->transform = Transform(Vector3f(0,0,0), Vector3f(-90,0,0), Vector3f(10,10,1));
    auto floorPrim = std::make_shared<Primitive>(floor);
    floorPrim->material = matteWhite;
    floorPrim->name = QString("Floor");

    // Light source, which is a diffuse area light with a large plane as its shape

    float ppp = 2;
    int numLights = 100;
    int numLights_sqrt = std::sqrt(numLights);
    Vector3f scale(.1, .1, .1);


    for(int j=0; j<2; j++) {

    for(int i=0; i<numLights; i++) {
        auto lightSphere = std::make_shared<Sphere>();
        float x = -4.0f + (float) (i/numLights_sqrt) / numLights_sqrt * 10;
        float z = -4.0f + (float) (i%numLights_sqrt) / numLights_sqrt * 10;
        lightSphere->transform = Transform(Vector3f(x, j+2, z), Vector3f(0,180,0), scale);
        auto lightSource = std::make_shared<DiffuseAreaLight>(lightSphere->transform, Color3f(1,1,1) * 2.f, lightSphere);
        auto lightPrim = std::make_shared<Primitive>(lightSphere, nullptr, lightSource);
        lightPrim->name = QString("Light Source");
        lightSource->name = QString("Light Source") + QString::number(i);

        primitives.append(lightPrim);
        lights.append(lightSource);
    }
}

    // Shadow casting shape, which is a red sphere
    auto sphere = std::make_shared<Sphere>();
    sphere->transform = Transform(Vector3f(0,1,0), Vector3f(0,0,0), Vector3f(1,1,1));
    auto spherePrim = std::make_shared<Primitive>(sphere);
    spherePrim->material = matteRed;
    spherePrim->name = QString("Red Sphere");

    // Back wall, which is a green rectangle
    auto greenWall = std::make_shared<SquarePlane>();
    greenWall->transform = Transform(Vector3f(-5,2.5f,0), Vector3f(0,90,0), Vector3f(10, 5, 1));
    auto greenWallPrim = std::make_shared<Primitive>(greenWall);
    greenWallPrim->material = matteGreen;
    greenWallPrim->name = QString("Wall");

    primitives.append(floorPrim);
    primitives.append(spherePrim);
    primitives.append(greenWallPrim);


    for(std::shared_ptr<Primitive> p : primitives)
    {
        p->shape->create();
    }

    camera = Camera(400, 400, Point3f(5, 8, -5), Point3f(0,0,0), Vector3f(0,1,0));
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.create();
    film = Film(400, 400);

    std::vector<cy::Point3f> lightPos ;
    std::vector<cy::Color> lightIntensities;
    for(auto light : lights) {
        Point3f p = light->transform.position();
        lightPos.push_back( cy::Point3f(p.x, p.y, p.z));
        lightIntensities.push_back( cy::Color(light->R().x, light->R().y, light->R().z ));
    }

    int minLevelLights = 1;
    const cy::Point3f *lightPos_ptr = lightPos.data();
    const cy::Color *lightCol_ptr = lightIntensities.data();


    LGH.Build(lightPos_ptr, lightCol_ptr, (int)lightPos.size(), minLevelLights);

    int nLevels = LGH.GetNumLevels();

    std::vector<std::vector<cy::Point3f>> all_points(nLevels);


    auto lightingFuc = [&](int level, int light_id, const cy::Point3f &light_position, const cy::Color &light_intensity){
        all_points[level].push_back(light_position);
    };


    cy::Point3f zeroPos(0.f, 0.f, 0.f);
    LGH.Light(zeroPos, 1.01f, 1, lightingFuc);

    std::cout << all_points.size() << std::endl;

}
