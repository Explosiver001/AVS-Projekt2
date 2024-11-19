/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  FULL NAME <xlogin00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    DATE
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.
    
    return buildTree(Vec3_t<float>(), field, mGridSize);
}


unsigned TreeMeshBuilder::buildTree(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const unsigned gridSize){
    
    if(isCubeEmpty(cubeOffset, field, float(gridSize))){
        return 0;
    }
    
    if(gridSize <= GRID_SIZE_CUT_OFF)
    {
        return buildCube(cubeOffset, field);
    }

    unsigned innerGridSize = gridSize / 2;
    float innerGridEdge = float(innerGridSize);
    unsigned totalTriangles = 0;

    size_t innerCubes = sc_vertexNormPos.size();

    for (int i = 0; i < innerCubes; i++){
        Vec3_t<float> vertexNormPos = sc_vertexNormPos.at(i);
        Vec3_t<float> innerCubeOffset(
				cubeOffset.x + vertexNormPos.x * innerGridEdge,
				cubeOffset.y + vertexNormPos.y * innerGridEdge,
				cubeOffset.z + vertexNormPos.z * innerGridEdge
			);
        totalTriangles += buildTree(innerCubeOffset, field, innerGridSize);
    }
    
    return totalTriangles;
}

bool TreeMeshBuilder::isCubeEmpty(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const float cubeEdgeLength){

    float fullEdgeLength = cubeEdgeLength * mGridResolution;
	float halfEdgeLength = fullEdgeLength / 2.F;
    
	const Vec3_t<float> cubeCenter(
		cubeOffset.x * mGridResolution + halfEdgeLength,
		cubeOffset.y * mGridResolution + halfEdgeLength,
		cubeOffset.z * mGridResolution + halfEdgeLength
	);

	static const float expr = sqrtf(3.F) / 2.F;

	return evaluateFieldAt(cubeCenter, field) > (mIsoLevel + expr * fullEdgeLength);
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    // NOTE: This method is called from "buildCube(...)"!

    // 1. Store pointer to and number of 3D points in the field
    //    (to avoid "data()" and "size()" call in the loop).
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());
    
    float value = std::numeric_limits<float>::max();


    // 2. Find minimum square distance from points "pos" to any point in the
    //    field.
    #pragma omp simd reduction(min:value)
    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        // Comparing squares instead of real distance to avoid unnecessary
        // "sqrt"s in the loop.
        value = std::min(value, distanceSquared);
    }

    // 3. Finally take square root of the minimal square distance to get the real distance
    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    mTriangles.push_back(triangle);
}
