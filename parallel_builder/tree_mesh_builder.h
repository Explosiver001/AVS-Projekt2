/**
 * @file    tree_mesh_builder.h
 *
 * @author  Michal Novák <xnovak3g@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    01.12.2024
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"


#define GRID_SIZE_CUT_OFF 1

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

private:
    bool isCubeEmpty(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const unsigned cubeGridSize);
    unsigned buildTree(const Vec3_t<float> &cubeOffset, const ParametricScalarField &field, const unsigned gridSize);
    float sqrt3;

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }
    std::vector<Triangle_t> mTriangles; 
};

#endif // TREE_MESH_BUILDER_H
