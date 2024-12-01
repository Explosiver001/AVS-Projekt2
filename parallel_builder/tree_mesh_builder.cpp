/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Michal Novák <xnovak3g@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    01.12.2024
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{
    sqrt3 = sqrt(3);
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.
    unsigned totalTriangles = 0;

    // start parallel region
    #pragma omp parallel default(none) shared(totalTriangles, field)
    {
        #pragma omp single nowait
        {
            totalTriangles = buildTree(Vec3_t<float>(), field, mGridSize);
        }
    }
        
    return totalTriangles;
}


unsigned TreeMeshBuilder::buildTree(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const unsigned gridSize){
    
    // check if cube is empty
    if(isCubeEmpty(cubeOffset, field, gridSize)){
        // if cube is empty, there is no need to calculate triangles inside
        return 0;
    }
    
    // once cube is only 1 voxel, call buildCube
    if(gridSize <= GRID_SIZE_CUT_OFF)
    {
        return buildCube(cubeOffset, field);
    }

    // compute inner grid size
    unsigned innerGridSize = gridSize / 2;

    unsigned totalTriangles = 0;

    // divide computation to 8 threads, each working on 1 inner cube
    size_t innerCubes = sc_vertexNormPos.size();
    for (int i = 0; i < innerCubes; i++){
        #pragma omp task firstprivate(i) shared(field, totalTriangles, cubeOffset, innerGridSize)
        {
            Vec3_t<float> vertexNormPos = sc_vertexNormPos.at(i);

            // calculate new cube coordinates
            Vec3_t<float> innerCubeOffset(
                cubeOffset.x + vertexNormPos.x * innerGridSize,
                cubeOffset.y + vertexNormPos.y * innerGridSize,
                cubeOffset.z + vertexNormPos.z * innerGridSize
            );

            // recursively call buildTree
            unsigned innerTriangles = buildTree(innerCubeOffset, field, innerGridSize);

            // store calculated results
            #pragma omp atomic update
            totalTriangles += innerTriangles;
        }
    }
    
    #pragma omp taskwait
    {
        return totalTriangles;
    }
}

bool TreeMeshBuilder::isCubeEmpty(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const unsigned cubeGridSize){
	float halfEdgeLength = cubeGridSize * mGridResolution / 2.0;
    
    // calculate cube center
	const Vec3_t<float> cubeCenter(
		cubeOffset.x * mGridResolution + halfEdgeLength,
		cubeOffset.y * mGridResolution + halfEdgeLength,
		cubeOffset.z * mGridResolution + halfEdgeLength
	);

	return evaluateFieldAt(cubeCenter, field) > (mIsoLevel + sqrt3 * halfEdgeLength);
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
    #pragma omp critical
    {
        mTriangles.push_back(triangle);
    }
}
