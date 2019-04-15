/*
 * Copyright (c) 2019
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * Simple Particle demonstration code.  Includes sample collision detection...
 */

#include "SOP_SParticle.h"
#include "vec.h"


#include <GU/GU_Detail.h>
#include <GU/GU_RayIntersect.h>

#include <GEO/GEO_PrimPart.h>

#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>

#include <PRM/PRM_Include.h>

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>

using namespace HDK_Sample;

void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        "hdk_sparticle",
        "LitzUp",
        SOP_SParticle::myConstructor,
        SOP_SParticle::myTemplateList,
        1,      // Min required sources
        2,      // Maximum sources
        0));
}

// The names here have to match the inline evaluation functions
static PRM_Name names[] = {
    PRM_Name("alpha", "alpha"),
    PRM_Name("cell_size", "Cell Size"),
    PRM_Name("force", "Force"),
	PRM_Name("build_lgh", "Build LGH")
};

static PRM_Default	cell_size(1.0);

static int buildLGH_static(void *data, int index, float time, const PRM_Template *tplate) {
	OP_Context myContext(time);
	SOP_SParticle *particleSystem_ptr = (SOP_SParticle *)data;

	particleSystem_ptr->buildLGH_callback(myContext);

	return 0;
}

PRM_Template
SOP_SParticle::myTemplateList[] = {
	PRM_Template(PRM_INT,	1, &names[0], PRMoneDefaults),
	PRM_Template(PRM_TYPE_FLOAT,	1, &names[1], &cell_size),
	PRM_Template(PRM_XYZ_J,	3, &names[2]),
	PRM_Template(PRM_CALLBACK, 1, &names[3], 0, 0, 0, buildLGH_static),
    PRM_Template(),
};

int *SOP_SParticle::myOffsets = 0;

OP_Node *SOP_SParticle::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_SParticle(net, name, op);
}


void SOP_SParticle::buildLGH_callback(OP_Context &myContext) {
	cookMySop(myContext);
}


SOP_SParticle::SOP_SParticle(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op)
    , mySystem(NULL)
{
    // Make sure that our offsets are allocated.  Here we allow up to 32
    // parameters, no harm in over allocating.  The definition for this
    // function is in OP/OP_Parameters.h
    if (!myOffsets)
		myOffsets = allocIndirect(32);

    // Now, flag that nothing has been built yet...
    myVelocity.clear();
	myColor.clear();
	myLevel.clear();
}

SOP_SParticle::~SOP_SParticle() {}

void
SOP_SParticle::birthParticle(UT_Vector3 pos, UT_Vector3 col, int level)
{
	// Strictly speaking, we should be using mySource->getPointMap() for the
	// initial invalid point, but mySource may be NULL.
	GA_Offset srcptoff = GA_INVALID_OFFSET;
	GA_Offset vtxoff = mySystem->giveBirth();
	if (mySource)
	{
		if (mySourceNum >= mySource->getPointMap().indexSize())
			mySourceNum = 0;
		if (mySource->getPointMap().indexSize() > 0) // No points in the input
			srcptoff = mySource->pointOffset(mySourceNum);
		mySourceNum++; // Move on to the next source point...
	}

	// GU_Detail	*gdp = myGdpHandle.gdp();

	GA_Offset ptoff = gdp->vertexPoint(vtxoff);


	if (GAisValid(srcptoff))
	{

		//if (ptoff == 31 || ptoff == 35) {
		//	std::cout << "stop" << std::endl;
		//}

		gdp->setPos3(ptoff, pos);
		
		myColor.set(ptoff, col);
		myLevel.set(ptoff, level);
	}
	else
	{
		gdp->setPos3(ptoff, SYSdrand48() - .5, SYSdrand48() - .5, SYSdrand48() - .5);
		myColor.set(ptoff, UT_Vector3(1,1,1));
		myLevel.set(ptoff, 1);
	}

}

void HDK_Sample::SOP_SParticle::birthParticles(cy::Point3f *pos, cy::Color *col, int numLights, int level)
{

	for (int i = 0; i < numLights; i++) {
		cy::Point3f position = *pos++;
		cy::Color color = *col++;

		UT_Vector3 pos_vec3(position.x, position.y, position.z);
		UT_Vector3 col_vec3(color.r, color.g, color.b);
		birthParticle(pos_vec3, col_vec3, level);
	}


}

void HDK_Sample::SOP_SParticle::birthParticles1(std::vector<UT_Vector3> &pos, std::vector<UT_Vector3> &col, int numLights, int level)
{
	for (int i = 0; i < numLights; i++) {
		birthParticle(pos[i], col[i], level);
	}

}

//void HDK_Sample::SOP_SParticle::buildLGH(const GU_Detail *mySource, fpreal currframe)
//{
//	GA_Index numPaticles = mySource->getPointMap().indexSize();
//	const GA_Attribute* p_ref_position = mySource->findPointAttribute("P");
//	const GA_Attribute* p_ref_color = mySource->findPointAttribute("Cd");
//
//	GA_ROHandleV3  pos(p_ref_position);
//	GA_ROHandleV3  col(p_ref_color);
//
//	if (pos.isInvalid() || col.isInvalid()) {
//		std::cout << "Input type error" << std::endl;
//		return;
//	}
//
//
//	//////////////////////// //////////// Prepare Data //////////// ////////////////////////
//
//	std::vector<cy::Point3f> positions;
//	std::vector<cy::Color> colors;
//
//	for (GA_Size i = 0; i < numPaticles; i++) {
//		UT_Vector3F point_pos = pos.get(i);
//		cy::Point3f cy_pos(point_pos.x(), point_pos.y(), point_pos.z());
//		positions.push_back(cy_pos);
//
//		UT_Vector3F point_col = col.get(i);
//		cy::Color cy_col(point_col.x(), point_col.y(), point_col.z());
//		colors.push_back(cy_col);
//
//		//if (i == 31 || i == 35) {
//		//	std::cout << point_pos << std::endl;
//		//}
//	}
//
//	//////////////////////// //////////// Build LGH //////////// ////////////////////////
//	cy::LightingGridHierarchy LGH;
//
//	int minLevelLights = 1;
//	float cellSize = CELL_SIZE();
//	int highestLevel = 4;
//
//	const cy::Point3f *lightPos_ptr = positions.data();
//	const cy::Color *lightCol_ptr = colors.data();
//
//	LGH.Build(lightPos_ptr, lightCol_ptr, (int)positions.size(), minLevelLights, cellSize, highestLevel);
//
//
//	//////////////////////// //////////// Birth Particles //////////// ////////////////
//
//
//	cy::LightingGridHierarchy::Level *levels = LGH.GetLevels();
//	int numLevels = LGH.GetNumLevels();
//
//	for (int i = 0; i < numLevels; i++) {
//		int numLights = levels[i].pc.GetPointCount() + 1;
//		birthParticles(levels[i].positions, levels[i].colors, numLights, i);
//	}
//}
//
//

/////////////////////////Chianti: updating buildLGH//////////////////////////


void HDK_Sample::SOP_SParticle::buildLGH1(const GU_Detail *mySource, fpreal currframe)
{
	GA_Index numPaticles = mySource->getPointMap().indexSize();
	const GA_Attribute* p_ref_position = mySource->findPointAttribute("P");
	const GA_Attribute* p_ref_color = mySource->findPointAttribute("Cd");

	GA_ROHandleV3  pos(p_ref_position);
	GA_ROHandleV3  col(p_ref_color);

	if (pos.isInvalid() || col.isInvalid()) {
		std::cout << "Input type error" << std::endl;
		return;
	}


	//////////////////////// //////////// Prepare Data //////////// ////////////////////////

	/*
	std::vector<cy::Point3f> positions;
	std::vector<cy::Color> colors;
	*/

	//*****Chianti: using houdini points class ******//
	std::vector<UT_Vector3F> positions;
	std::vector<UT_Vector3F> colors;

	for (GA_Size i = 0; i < numPaticles; i++) {
		UT_Vector3F point_pos = pos.get(i);
		//cy::Point3f cy_pos(point_pos.x(), point_pos.y(), point_pos.z());
		positions.push_back(point_pos);

		UT_Vector3F point_col = col.get(i);
		//cy::Color cy_col(point_col.x(), point_col.y(), point_col.z());
		colors.push_back(point_col);

	}

	//////////////////////// //////////// Build LGH //////////// ////////////////////////

	//******TODO: remember to get rid of the namespace *******//
	//cy::LightingGridHierarchy LGH;

	lightGrid LGH;

	int minLevelLights = 1;
	float cellSize = CELL_SIZE();
	int highestLevel = 4;

	//const cy::Point3f *lightPos_ptr = positions.data();
	//const cy::Color *lightCol_ptr = colors.data();

	const UT_Vector3 *lightPos_ptr = positions.data();
	const UT_Vector3 *lightCol_ptr = colors.data();

	LGH.Build(lightPos_ptr, lightCol_ptr, (int)positions.size(), minLevelLights, cellSize, highestLevel);


	//////////////////////// //////////// Birth Particles //////////// ////////////////


	//cy::LightingGridHierarchy::Level *levels = LGH.GetLevels();
	lightGrid::Level *levels = LGH.GetLevels();

	int numLevels = LGH.GetNumLevels();

	for (int i = 0; i < numLevels; i++) {
		/*int numLights = levels[i].pc.GetPointCount() + 1;
		birthParticles(levels[i].positions, levels[i].colors, numLights, i);*/
		int numLights = levels[i].positions.size();
		birthParticles1(levels[i].positions, levels[i].colors, numLights, i);

	}
}

void
SOP_SParticle::birthParticle()
{
    // Strictly speaking, we should be using mySource->getPointMap() for the
    // initial invalid point, but mySource may be NULL.
    GA_Offset srcptoff = GA_INVALID_OFFSET;
    GA_Offset vtxoff = mySystem->giveBirth();
    if (mySource)
    {
		if (mySourceNum >= mySource->getPointMap().indexSize())
		    mySourceNum = 0;
		if (mySource->getPointMap().indexSize() > 0) // No points in the input
		    srcptoff = mySource->pointOffset(mySourceNum);
		mySourceNum++; // Move on to the next source point...
    }
    GA_Offset ptoff = gdp->vertexPoint(vtxoff);
    if (GAisValid(srcptoff))
    {
		gdp->setPos3(ptoff, mySource->getPos3(srcptoff));
		if (mySourceVel.isValid())
				myVelocity.set(ptoff, mySourceVel.get(srcptoff));
		else
				myVelocity.set(ptoff, UT_Vector3(0, 0, 0));
    }
	else
    {
        gdp->setPos3(ptoff, SYSdrand48()-.5, SYSdrand48()-.5, SYSdrand48()-.5);
		myVelocity.set(ptoff, UT_Vector3(0, 0, 0));
    }
    // First index of the life variable represents how long the particle has
    // been alive (set to 0).
    myLife.set(ptoff, 0, 1);
    // The second index of the life variable represents how long the particle
    // will live (in frames)
    myLife.set(ptoff, 1, 30+30*SYSdrand48());
}

int
SOP_SParticle::moveParticle(GA_Offset ptoff, const UT_Vector3 &force)
{
    float life = myLife.get(ptoff, 0);
    float death = myLife.get(ptoff, 1);
    life += 1;
    myLife.set(ptoff, life, 0); // Store back in point
    if (life >= death)
        return 0;               // The particle should die!

    float tinc = 1./30.;        // Hardwire 1/30 of a second time inc...

    // Adjust the velocity (based on the force) - of course, the multiplies
    // can be pulled out of the loop...
    UT_Vector3 vel = myVelocity.get(ptoff);
    vel += tinc*force;
    myVelocity.set(ptoff, vel);

    // Now adjust the point positions

    if (myCollision)
    {
	UT_Vector3 dir = vel * tinc;

	// here, we only allow hits within the length of the velocity vector
	GU_RayInfo info(dir.normalize());

	UT_Vector3 start = gdp->getPos3(ptoff);
	if (myCollision->sendRay(start, dir, info) > 0)
	    return 0;	// We hit something, so kill the particle
    }

    UT_Vector3 pos = gdp->getPos3(ptoff);
    pos += tinc*vel;
    gdp->setPos3(ptoff, pos);

    return 1;
}

void
SOP_SParticle::timeStep(fpreal now)
{
    UT_Vector3 force(FX(now), FY(now), FZ(now));
	int nbirth = 10;

    if (error() >= UT_ERROR_ABORT)
	return;

  //  for (int i = 0; i < nbirth; ++i)
		//birthParticle();

    for (GA_Size i = 0; i < mySystem->getNumParticles(); i++)
    {
		GA_Offset offset = mySystem->vertexPoint(i);

		if (!moveParticle(offset, force))
			mySystem->deadParticle(i);
    }
    mySystem->deleteDead();
}

void
SOP_SParticle::initSystem()
{
    if (!gdp) gdp = new GU_Detail;

    // Check to see if we really need to reset everything
    if (gdp->getPointMap().indexSize() > 0 || myVelocity.isInvalid())
    {
	mySourceNum = 0;
	gdp->clearAndDestroy();
	mySystem = (GEO_PrimParticle *)gdp->appendPrimitive(GEO_PRIMPART);
	mySystem->clearAndDestroy();

	// A vector attribute will be transformed correctly by following
	//	SOPs.  Use float types for stuff like color...
	myVelocity = GA_RWHandleV3(gdp->addFloatTuple(GA_ATTRIB_POINT, "v", 3));
	if (myVelocity.isValid())
	    myVelocity.getAttribute()->setTypeInfo(GA_TYPE_VECTOR);

	myColor = GA_RWHandleV3(gdp->addFloatTuple(GA_ATTRIB_POINT, "Cd", 3));
	if (myColor.isValid())
		myColor.getAttribute()->setTypeInfo(GA_TYPE_VECTOR);
	}

	myLife = GA_RWHandleF(gdp->addFloatTuple(GA_ATTRIB_POINT, "life", 2));

	myLevel = GA_RWHandleF(gdp->addIntTuple(GA_ATTRIB_POINT, "level", 1));


}

OP_ERROR
SOP_SParticle::cookMySop(OP_Context &context)
{
    // We must lock our inputs before we try to access their geometry.
    // OP_AutoLockInputs will automatically unlock our inputs when we return.
    // NOTE: Don't call unlockInputs yourself when using this!
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    // Now, indicate that we are time dependent (i.e. have to cook every
    // time the current frame changes).
    OP_Node::flags().timeDep = 1;

    // Channel manager has time info for us
    CH_Manager *chman = OPgetDirector()->getChannelManager();

    // This is the frame that we're cooking at...
    fpreal currframe = chman->getSample(context.getTime());
	fpreal alpha = ALPHA(); // Find our reset frame...

	//if (!mySystem)
	//{
	//	myLastCookTime = alpha;
	//	initSystem();
	//}

	initSystem();

		// Set up our source information...
		mySource = inputGeo(0, context);
		if (mySource)
		{
			//**************************chianti: testing revised point type ***********************//
			buildLGH1(mySource, currframe);
		}

		// This is where we notify our handles (if any) if the inputs have
		// changed.  This is normally done in cookInputGroups, but since there
		// is no input group, NULL is passed as the fourth argument.
		notifyGroupParmListeners(0, -1, mySource, NULL);


		// Now cook the geometry up to our current time
		// Here, we could actually re-cook the source input to get a moving
		// source...  But this is just an example ;-)
		currframe += 0.05;	// Add a bit to avoid floating point error
		while (myLastCookTime < currframe)
		{
		    // Here we have to convert our frame number to the actual time.
			// TONG TODO

		    //timeStep(chman->getTime(myLastCookTime));

		    myLastCookTime += 1;
		}

		// Set the node selection for the generated particles. This will 
		// highlight all the points generated by this node, but only if the 
		// highlight flag is on and the node is selected.
		select(GA_GROUP_POINT);
    

    return error();
}

const char *
SOP_SParticle::inputLabel(unsigned inum) const
{
    switch (inum)
    {
	case 0: return "Particle Source Geometry";
	case 1:	return "Collision Object";
    }
    return "Unknown source";
}
