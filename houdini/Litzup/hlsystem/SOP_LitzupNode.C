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


#include "SOP_LitzupNode.h"

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


void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        "litzup",
        "LitzUp",
        SOP_LitzupNode::myConstructor,
        SOP_LitzupNode::myTemplateList,
        (unsigned int)0,      // Min required sources
        2,      // Maximum sources
        0));
}

// The names here have to match the inline evaluation functions
static PRM_Name names[] = {
    PRM_Name("alpha", "alpha"),
    PRM_Name("cell_size", "Cell Size")
};

static PRM_Default	cell_size(1.0);

static int buildLGH_static(void *data, int index, float time, const PRM_Template *tplate) {
	OP_Context myContext(time);
	SOP_LitzupNode *particleSystem_ptr = (SOP_LitzupNode *)data;

	particleSystem_ptr->createCopyAndLightNode(myContext, 1, 10.f);
	return 0;
}

PRM_Template
SOP_LitzupNode::myTemplateList[] = {
	PRM_Template(PRM_INT,	1, &names[0], PRMoneDefaults),
	PRM_Template(PRM_TYPE_FLOAT,	1, &names[1], &cell_size),
	//PRM_Template(PRM_CALLBACK, 1, &names[2], 0, 0, 0, buildLGH_static),
    PRM_Template(),
};

int *SOP_LitzupNode::myOffsets = 0;

OP_Node *SOP_LitzupNode::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_LitzupNode(net, name, op);
}


SOP_LitzupNode::SOP_LitzupNode(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op)
    , mySystem(NULL), maxLevel(0)
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

SOP_LitzupNode::~SOP_LitzupNode() {}

void
SOP_LitzupNode::birthParticle(UT_Vector3 pos, UT_Vector3 col, int level)
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

void SOP_LitzupNode::birthParticles1(std::vector<UT_Vector3> &pos, std::vector<UT_Vector3> &col, int numLights, int level)
{
	for (int i = 0; i < numLights; i++) {
		birthParticle(pos[i], col[i], level);
	}

}

/////////////////////////Chianti: updating buildLGH//////////////////////////


void SOP_LitzupNode::buildLGH(const GU_Detail *mySource, fpreal currframe, OP_Context &context)
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
	fpreal now = context.getTime();
	lightGrid LGH;

	int minLevelLights = 1;
	float cellSize = CELL_SIZE(now);
	int highestLevel = 8;

	//const cy::Point3f *lightPos_ptr = positions.data();
	//const cy::Color *lightCol_ptr = colors.data();

	const UT_Vector3 *lightPos_ptr = positions.data();
	const UT_Vector3 *lightCol_ptr = colors.data();

	LGH.Build(lightPos_ptr, lightCol_ptr, (int)positions.size(), minLevelLights, cellSize, highestLevel);


	//////////////////////// //////////// Birth Particles //////////// ////////////////

	lightGrid::Level *levels = LGH.GetLevels();

	int numLevels = LGH.GetNumLevels();
	if (numLevels > maxLevel)
		maxLevel = numLevels;
	for (int i = 0; i < numLevels; i++) {
		/*int numLights = levels[i].pc.GetPointCount() + 1;
		birthParticles(levels[i].positions, levels[i].colors, numLights, i);*/
		int numLights = levels[i].positions.size();
		birthParticles1(levels[i].positions, levels[i].colors, numLights, i);
	}


	//////////////////////// //////////// Connect nodes //////////// ////////////////
	OP_NetworkBoxItemList connected_nodes;
	getConnectedItems(connected_nodes, false, false, false);
	
	if (missingNode(numLevels)) {
		copy_nodes.clear();
	}

	fpreal alpha = ALPHA();


	while (copy_nodes.size() < numLevels) {
		int level = copy_nodes.size();
		int scale = 1 << level;
		float radius = cellSize * alpha * scale * 0.5;
		OP_Node* node = createCopyAndLightNode(context, level, radius);
		copy_nodes.push_back(node);
	}

	// Update parameter if necessay
	if (m_alpha != alpha || m_cellsize != cellSize) {
		m_cellsize = cellSize;
		m_alpha = alpha;
		updateNodeParameter(context, cellSize, alpha);
	}

}

void
SOP_LitzupNode::birthParticle()
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

}


void
SOP_LitzupNode::timeStep(fpreal now)
{
}

void
SOP_LitzupNode::initSystem()
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

	myLevel = GA_RWHandleF(gdp->addIntTuple(GA_ATTRIB_POINT, "level", 1));

}

OP_ERROR
SOP_LitzupNode::cookMySop(OP_Context &context)
{

    // We must lock our inputs before we try to access their geometry.
    // OP_AutoLockInputs will automatically unlock our inputs when we return.
    // NOTE: Don't call unlockInputs yourself when using this!
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    // Now, indicate that we are time dependent (i.e. have to cook every
    // time the current frame changes).
    OP_Node::flags().setTimeDep(1);

    // Channel manager has time info for us
    CH_Manager *chman = OPgetDirector()->getChannelManager();

    // This is the frame that we're cooking at...
    fpreal currframe = chman->getSample(context.getTime());

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
		buildLGH(mySource, currframe, context);
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

bool SOP_LitzupNode::missingNode(int numLevels)
{
	OP_Network* root = (OP_Network *)OPgetDirector()->findNode("/obj");

	for (int i = 0; i < maxLevel; i++) {
		std::string copy_name = "Litzup_cp_level" + std::to_string(i);
		OP_Node * copyNode = findNode(copy_name.c_str());
		if (!copyNode) {
			return true;
		}

		std::string light_name = "Litzup_light_level" + std::to_string(i);
		OP_Node * lightNode = root->findNode(light_name.c_str());
		
		if (!lightNode) {
			return true;
		}		
	}

	return false;;
}

const char *
SOP_LitzupNode::inputLabel(unsigned inum) const
{
    switch (inum)
    {
	case 0: return "Particle Source Geometry";
	case 1: return "Primitives to Copy";
    }
    return "Unknown source";
}

OP_Node * SOP_LitzupNode::createCopyAndLightNode(OP_Context & context, int level, float radius)
{
	float t = context.getTime();
	OP_Node * copyNode = nullptr;
	OP_Node* lightNode = nullptr;

	// Copy node, create node
	OP_Network *parent = getParent();
	std::string copy_name = "Litzup_cp_level" + std::to_string(level);

	copyNode = parent->findNode(copy_name.c_str());

	if (!copyNode) {
		copyNode = parent->createNode("copytopoints", copy_name.c_str());

		if (!copyNode)
			return copyNode;
		// run creation script
		if (!copyNode->runCreateScript())
			return copyNode;

		// set parameters
		std::string val = "@level==" + std::to_string(level);
		bool succuss = copyNode->setParameterOrProperty("targetgroup", 0, t, val.c_str(), CH_StringMeaning::CH_STRING_LITERAL);
		if (!succuss) {
			std::cout << "Failed to set copy node parameter" << std::endl;
		}
		// set input
		copyNode->setInput(1, this);
		OP_Node* primitive = getInput(1);
		if (primitive)
			copyNode->setInput(0, primitive);
		copyNode->moveToGoodPosition();
	}


	// Light node, create node
	std::string light_name = "Litzup_light_level" + std::to_string(level);
	OP_Network* root = (OP_Network *)OPgetDirector()->findNode("/obj");
	if (!root) {
		std::cout << "No obj node" << std::endl;
	}
	
	lightNode = root->findNode(light_name.c_str());

	if (!lightNode) {
		lightNode = root->createNode("hlight", light_name.c_str());

		if (!lightNode || !lightNode->runCreateScript())
			return copyNode;

		UT_String copynode_path;
		parent->getFullPath(copynode_path);
		copynode_path.append('/');
		copynode_path.append(copy_name.c_str());

		bool succuss = lightNode->setParameterOrProperty("light_type", 0, t, 6);
		succuss &= lightNode->setParameterOrProperty("areageometry", 0, t, copynode_path.c_str(), CH_StringMeaning::CH_STRING_LITERAL);
		succuss &= lightNode->setParameterOrProperty("activeradiusenable", 0, t, 1);
		succuss &= lightNode->setParameterOrProperty("activeradius", 0, t, radius);

		succuss &= lightNode->setParameterOrProperty("normalizearea", 0, t, 0, false);


		if (!succuss) {
			std::cout << "Failed to set ligth parameter" << std::endl;
		}
		lightNode->moveToGoodPosition();

	}

	return copyNode;
}

void SOP_LitzupNode::updateNodeParameter(OP_Context & context, fpreal cellSize, fpreal alpha)
{
	float t = context.getTime();
	OP_Network* root = (OP_Network *)OPgetDirector()->findNode("/obj");

	for (int i = 0; i < maxLevel; i++) {
		int scale = 1 << i;
		std::string light_name = "Litzup_light_level" + std::to_string(i);

		OP_Node * lightNode = root->findNode(light_name.c_str());
		lightNode->setParameterOrProperty("activeradius", 0, t, cellSize * alpha * scale * 0.5);
	}
}
