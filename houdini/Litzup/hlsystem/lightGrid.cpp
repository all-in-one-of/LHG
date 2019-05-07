#include "lightGrid.h"
#include <iostream>


lightGrid::lightGrid()
	:levels(nullptr), numLevels(0)
{
}


lightGrid::~lightGrid()
{
	Clear();
}

const UT_Vector3F& lightGrid::GetLightPos(int level, int i) const 
{ 
	//return levels[level].pc.GetPoint(i); 
	return levels[level].positions[i];
}


const int lightGrid::GetLightIndex(int level, int i) const
{
	//return levels[level].pc.GetPointIndex(i);  //!< Returns the i^th light index at the given level.

	//**************Chianti: didn't use this function********************//
	return 0;
}

const UT_Vector3F&  lightGrid::GetLightIntens(int level, int ix) const
{
	return levels[level].colors[ix]; 
}								//!< Returns the intensity of the light with index ix at the given level.

const UT_Vector3F& lightGrid::GetLightPosDev(int level, int ix) const
{ 
	return level > 0 ? levels[level].pDev[ix] : UT_Vector3F(0, 0, 0); 
}	//!< Returns the position variation of the light with index ix at the given level.


float lightGrid::RandomX()
{
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<float> distribution;
	float x = distribution(generator);
	float y = distribution(generator);
	if (y > (cosf(x * PI + 1)*0.5f))
		x -= 1;
	return x;
}

UT_Vector3F lightGrid::RandomPos()
{
	UT_Vector3F p;
	p.x() = RandomX();
	p.y() = RandomX();
	p.z() = RandomX();
	return p;
}

bool lightGrid::DoBuild(const UT_Vector3F *lightPos, const UT_Vector3F *lightColor, int numLights, float autoFitScale, int minLevelLights, float cellSize, int highestLevel)
{
	Clear();
	if (numLights <= 0 || highestLevel <= 0) return false;

	// Compute the bounding box for the lighPoss
	UT_Vector3F boundMin = UT_Vector3F(lightPos[0]);
	UT_Vector3F boundMax = UT_Vector3F(lightPos[0]);
	for (int i = 1; i < numLights; i++) {
		for (int d = 0; d < 3; d++) {
			if (boundMin[d] > lightPos[i][d]) boundMin[d] = lightPos[i][d];
			if (boundMax[d] < lightPos[i][d]) boundMax[d] = lightPos[i][d];
		}
	}
	UT_Vector3F boundDif = boundMax - boundMin;

	//**************chianti: update min, max function***************//
	//float boundDifMin = boundDif.Min();
	float boundDifMin = std::fmin(boundDif.x(), std::fmin(boundDif.y(), boundDif.z()));
	float boundDifMax = std::fmax(boundDif.x(), std::fmax(boundDif.y(), boundDif.z()));

	// Determine the actual highest level
	float highestCellSize;

	//**************chianti: update IPoint3i***************//
	//cy::IPoint3i highestGridRes;
	UT_Vector3i highestGridRes;
	if (autoFitScale > 0) {


		//highestCellSize = boundDif.Max() * autoFitScale;
		highestCellSize = boundDifMax * autoFitScale;
		int s = int(1.0f / autoFitScale) + 2;
		if (s < 2) s = 2;

		//**************chianti: update set function***************//
		//highestGridRes.Set(s, s, s);
		highestGridRes = UT_Vector3i(s, s, s);
	}
	else {
		int highestLevelMult = 1 << (highestLevel - 1);
		highestCellSize = cellSize * highestLevelMult;
		
		while (highestLevel > 1 && highestCellSize > boundDifMin * 2) {
			highestLevel--;
			highestLevelMult = 1 << (highestLevel - 1);
			highestCellSize = cellSize * highestLevelMult;
		}
		//**************MAY HAVE ISSUE ******** chianti: update assigning highestGridRes***************//
		//highestGridRes = cy::IPoint3i(boundDif / highestCellSize) + 2;
		highestGridRes = UT_Vector3i(boundDif / highestCellSize) + UT_Vector3i(2, 2, 2);
	}


	//////////////////////////////////////done initializing highestGridRes//////////////////////////////////////////////////

	struct Node {
		Node() 
			: position(0, 0, 0), color(0, 0, 0), weight(0), firstChild(-1) 
		{}

		//Point3f position;
		//Color color;

		UT_Vector3F position;
		UT_Vector3F color;

#ifdef CY_LIGHTING_GRID_ORIG_POS
		Point3f origPos;
#endif // CY_LIGHTING_GRID_ORIG_POS

		//Point3f stdev;
		UT_Vector3F stdev;

		float weight;
		int firstChild;
		/*void AddLight(float w, const Point3f &p, const Color &c)*/
		void AddLight(float w, const UT_Vector3F &p, const UT_Vector3F &c)
		{
			weight += w;
			position += w * p;
			color += w * c;
			stdev += w * (p*p);
		}
		void Normalize()
		{
			if (weight > 0) {
				position /= weight;
				stdev = stdev / weight - position * position;
			}
		}
	};

	// Allocate the temporary nodes
	numLevels = highestLevel + 1;
	std::vector< std::vector<Node> > nodes(numLevels);

	/*auto gridIndex = [](cy::IPoint3i &index, const Point3f &pos, float cellSize)*/
	auto gridIndex = [](UT_Vector3i &index, const UT_Vector3F &pos, float cellSize)
	{
		//Point3f normP = pos / cellSize;
		//index = cy::IPoint3i(normP);
		//return normP - Point3f(index);

		UT_Vector3F normP = pos / cellSize;
		/*index = UT_Vector3i((int32_t)normP.x, (int32_t)normP.y, (int32_t)normP.z);*/
		index = UT_Vector3i(normP.x(), normP.y(), normP.z());

		if (index.z() < 0) {
			std::cout << "index.z < 0" << std::endl;
		}

		if (index.x() < 0) {
			std::cout << "index.x < 0" << std::endl;
		}

		if (index.y() < 0) {
			std::cout << "index.y < 0" << std::endl;
		}

		return normP - UT_Vector3F(index.x(), index.y(), index.z());

	};

	



	//auto addLightToNodes = [](std::vector<Node> &nds, const int nodeIDs[8], 
	//	const Point3f &interp, const Point3f &light_pos, const Color &light_color)
	auto addLightToNodes = [](std::vector<Node> &nds, const int nodeIDs[8],
		const UT_Vector3F &interp, const UT_Vector3F &light_pos, const UT_Vector3F &light_color)
	{
		for (int j = 0; j < 8; j++) {
			float w = ((j & 1) ? interp.x() : (1 - interp.x())) * ((j & 2) ? interp.y() : (1 - interp.y())) * ((j & 4) ? interp.z() : (1 - interp.z()));
			nds[nodeIDs[j]].AddLight(w, light_pos, light_color);
		}
	};

	// Generate the grid for the highest level

	/*Point3f highestGridSize = Point3f(highestGridRes - 1) * highestCellSize;
	Point3f center = (boundMax + boundMin) / 2;
	Point3f corner = center - highestGridSize / 2; */
	UT_Vector3 highestGridSize = (UT_Vector3F(-1.0) + UT_Vector3F(highestGridRes.x(), highestGridRes.y(), highestGridRes.z())) * highestCellSize;
	UT_Vector3F center = (boundMax + boundMin) / 2;
	UT_Vector3F corner = center - UT_Vector3F(highestGridSize.x() / 2.f, highestGridSize.y() / 2.f, highestGridSize.z() / 2.f);

	nodes[highestLevel].resize(highestGridRes.x() * highestGridRes.y() * highestGridRes.z());


#ifdef CY_LIGHTING_GRID_ORIG_POS
	for (int z = 0, j = 0; z < highestGridRes.z(); z++) {
		for (int y = 0; y < highestGridRes.y(); y++) {
			for (int x = 0; x < highestGridRes.x(); x++, j++) {
				nodes[highestLevel][j].origPos = corner + Point3f(x, y, z)*highestCellSize;
			}
		}
	}
#endif // CY_LIGHTING_GRID_ORIG_POS


	for (int i = 0; i < numLights; i++) {

		/*cy::IPoint3i index;
		Point3f interp = gridIndex(index, Point3f(lightPos[i]) - corner, highestCellSize);*/
		UT_Vector3i index;
		UT_Vector3F interp = gridIndex(index, UT_Vector3F(lightPos[i]) - corner, highestCellSize);


		int is = index.z()*highestGridRes.y()*highestGridRes.x() + index.y()*highestGridRes.x() + index.x();
		int nodeIDs[8] = {
			is,
			is + 1,
			is + highestGridRes.x(),
			is + highestGridRes.x() + 1,
			is + highestGridRes.x()*highestGridRes.y(),
			is + highestGridRes.x()*highestGridRes.y() + 1,
			is + highestGridRes.x()*highestGridRes.y() + highestGridRes.x(),
			is + highestGridRes.x()*highestGridRes.y() + highestGridRes.x() + 1,
		};
		for (int j = 0; j < 8; j++) assert(nodeIDs[j] >= 0 && nodeIDs[j] < (int)nodes[highestLevel].size());
		addLightToNodes(nodes[highestLevel], nodeIDs, interp, lightPos[i], lightColor[i]);
	}
	for (int i = 0; i < (int)nodes[highestLevel].size(); i++) nodes[highestLevel][i].Normalize();

	// Generate the lower levels
	float nodeCellSize = highestCellSize;
	/*cy::IPoint3i gridRes = highestGridRes;*/
	UT_Vector3i gridRes = highestGridRes;
	int levelSkip = 0;
	for (int level = highestLevel - 1; level > 0; level--) {
		// Find the number of nodes for this level
		int nodeCount = 0;
		for (int i = 0; i < (int)nodes[level + 1].size(); i++) {
			if (nodes[level + 1][i].weight > 0) {
				nodes[level + 1][i].firstChild = nodeCount;
				nodeCount += 8;
			}
		}

		if (nodeCount > numLights / 4) {
			levelSkip = level;
			break;
		}

		nodes[level].resize(nodeCount);
		// Add the lights to the nodes
		nodeCellSize /= 2;
		gridRes *= 2;

#ifdef CY_LIGHTING_GRID_ORIG_POS
		for (int i = 0; i < (int)nodes[level + 1].size(); i++) {
			int fc = nodes[level + 1][i].firstChild;
			if (fc < 0) continue;
			for (int z = 0, j = 0; z < 2; z++) {
				for (int y = 0; y < 2; y++) {
					for (int x = 0; x < 2; x++, j++) {
						nodes[level][fc + j].origPos = nodes[level + 1][i].origPos + Point3f(x, y, z)*nodeCellSize;
					}
				}
			}
		}
#endif // CY_LIGHTING_GRID_ORIG_POS


		for (int i = 0; i < numLights; i++) {
			//cy::IPoint3i index;
			//Point3f interp = gridIndex(index, Point3f(lightPos[i]) - corner, nodeCellSize);
			UT_Vector3i index;
			UT_Vector3F interp = gridIndex(index, UT_Vector3F(lightPos[i]) - corner, nodeCellSize);

			// find the node IDs
			int nodeIDs[8];

			//index <<= level + 2;
			index.x() <<= level + 2;
			index.y() <<= level + 2;
			index.z() <<= level + 2;


			for (int z = 0, j = 0; z < 2; z++) {
				int iz = index.z() + z;
				for (int y = 0; y < 2; y++) {
					int iy = index.y() + y;
					for (int x = 0; x < 2; x++, j++) {
						int ix = index.x() + x;
						int hix = ix >> (highestLevel + 2);
						int hiy = iy >> (highestLevel + 2);
						int hiz = iz >> (highestLevel + 2);
						int nid = hiz * highestGridRes.y()*highestGridRes.x() + hiy * highestGridRes.x() + hix;
						for (int l = highestLevel - 1; l >= level; l--) {
							int ii = ((index.z() >> l) & 4) | ((index.y() >> (l + 1)) & 2) | ((index.x() >> (l + 2)) & 1);
							assert(nodes[l + 1][nid].firstChild >= 0);
							nid = nodes[l + 1][nid].firstChild + ii;
							// assert(nid >= 0 && nid < (int)nodes[l].size());

							if (!(nid >= 0 && nid < (int)nodes[l].size())) {
								std::cout << nid << std::endl;
							}
						}
						nodeIDs[j] = nid;
					}
				}
			}
			addLightToNodes(nodes[level], nodeIDs, interp, lightPos[i], lightColor[i]);
		}
		for (int i = 0; i < (int)nodes[level].size(); i++) nodes[level][i].Normalize();
	}

	// Copy light data
	numLevels = highestLevel + 1 - levelSkip;
	int levelBaseSkip = 0;
	// Skip levels that have two few lights (based on minLevelLights).
	for (int level = 1; level < numLevels; level++) {
		std::vector<Node> &levelNodes = nodes[level + levelSkip];
		int count = 0;
		for (int i = 0; i < (int)levelNodes.size(); i++) {
			if (levelNodes[i].weight > 0) {
				count++;
			}
		}
		if (count < minLevelLights) {
			numLevels = level;
			break;
		}
	}

	levels = new Level[numLevels];

	//************************QUESTION: what is this function for?*****************************//
	for (int level = 1; level < numLevels; level++) {
		std::vector<Node> &levelNodes = nodes[level + levelSkip];
		Level &thisLevel = levels[level];

		//*******************Chianti: ignore kd tree ************************//
		//std::vector<Point3f> pos(levelNodes.size());
		int lightCount = 0;
		for (int i = 0; i < (int)levelNodes.size(); i++) {
			if (levelNodes[i].weight > 0) {

				
				//pos[lightCount++] = levelNodes[i].position;
				lightCount++;
			}
		}
		//thisLevel.pc.Build(lightCount, pos.data());
		//// TONG
		//thisLevel.positions = new Point3f[lightCount];

		//thisLevel.colors = new Color[lightCount];
		//thisLevel.pDev = new Point3f[lightCount];


		//***************TODO:ignore pc, push to level variables**********************//
		for (int i = 0, j = 0; i < (int)levelNodes.size(); i++) {
			if (levelNodes[i].weight > 0) {
				assert(j < lightCount);
				// TONG
				//thisLevel.positions[j] = levelNodes[i].position;

				//thisLevel.colors[j] = levelNodes[i].color;
				//thisLevel.pDev[j].x = sqrtf(levelNodes[i].stdev.x) * cyPi<float>();
				//thisLevel.pDev[j].y = sqrtf(levelNodes[i].stdev.y) * cyPi<float>();
				//thisLevel.pDev[j].z = sqrtf(levelNodes[i].stdev.z) * cyPi<float>();


				thisLevel.positions.push_back(levelNodes[i].position);
				thisLevel.colors.push_back(levelNodes[i].color);
				UT_Vector3F pDev = UT_Vector3F(sqrtf(levelNodes[i].stdev.x()) * PI, sqrtf(levelNodes[i].stdev.y()) * PI, sqrtf(levelNodes[i].stdev.z()) * PI);

				thisLevel.pDev.push_back(pDev);
				j++;
			}
		}
		levelNodes.resize(0);
		levelNodes.shrink_to_fit();
	}
	
	
	//std::vector<Point3f> pos(numLights);
	//// TONG
	//levels[0].positions = new Point3f[numLights];
	//levels[0].colors = new Color[numLights];


	for (int i = 0; i < numLights; i++) {
		//pos[i] = lightPos[i];
		//levels[0].colors[i] = lightColor[i];

		// TONG
		//levels[0].positions[i] = lightPos[i];


		levels->colors.push_back(lightColor[i]);
		levels->positions.push_back(lightPos[i]);
	}
	//levels[0].pc.Build(numLights, pos.data());
	this->cellSize = nodeCellSize;

	return true;
}

