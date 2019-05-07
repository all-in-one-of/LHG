#pragma once

#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <random>
#include <math.h>
#include <algorithm>



#define PI 3.141592653589793238462643383279502884197169
#define NDEBUG
#define assert(expression) ((void)0)

class lightGrid
{
public:
	struct Level {
		Level() {}
		~Level() { /*delete[] colors; delete[] pDev;*/ }

		//************Chianti: store all info into vector and get rid of PC*****************//
		//cy::PointCloud<Point3f, float, 3, int> pc;
		//pointCloud pc;
		std::vector<UT_Vector3F> pc;

		//Color   *colors;
		std::vector<UT_Vector3F> colors;
		//Point3f   *positions;
		std::vector<UT_Vector3F> positions;
		//Point3f *pDev; // position deviation for random shadow sampling
		std::vector<UT_Vector3F> pDev;
	};

	lightGrid();
	~lightGrid();
	void Clear() { if (levels) delete[] levels; levels = nullptr; numLevels = 0; }	//!< Deletes all data.
	Level *GetLevels() const { return levels; }
	int   GetNumLevels() const { return numLevels; }	//!< Returns the number of levels in the hierarchy.
	float GetCellSize() const { return cellSize; }		//!< Returns the size of a cell in the lowest (finest) level of the hierarchy.
	const UT_Vector3F& GetLightPos(int level, int i) const;
	const int      GetLightIndex(int level, int i) const;					//!< Returns the i^th light index at the given level.
	const UT_Vector3F&   GetLightIntens(int level, int ix) const;							//!< Returns the intensity of the light with index ix at the given level.
	const UT_Vector3F& GetLightPosDev(int level, int ix) const;  	//!< Returns the position variation of the light with index ix at the given level.

		//! Builds the Lighting Grid Hierarchy for the given point light positions and intensities using the given parameters.
	//! This method builds the hierarchy using the given cellSize as the size of the lowest (finest) level grid cells.
	bool Build(const UT_Vector3F *lightPos,	
		const UT_Vector3F   *lightIntensities, int numLights, int minLevelLights, float cellSize, int highestLevel)
	{
		//****************TODO: update dobuild***********************//
		return DoBuild(lightPos, lightIntensities, numLights, 0, minLevelLights, cellSize, highestLevel);
	}

	bool BuildAuto(const UT_Vector3F *lightPos,
		const UT_Vector3F   *lightIntensities, int numLights, int minLevelLights, float cellSize, int highestLevel)
	{
		//****************TODO: update dobuild***********************//
		return DoBuild(lightPos, lightIntensities, numLights, 1, minLevelLights, cellSize, highestLevel);
	}


private:
	Level *levels;
	int    numLevels;
	float  cellSize;
	float RandomX();
	UT_Vector3F RandomPos();
	bool DoBuild(const UT_Vector3F *lightPos, const UT_Vector3F *lightColor, int numLights, float autoFitScale, int minLevelLights, float cellSize = 0, int highestLevel = 10);



};

