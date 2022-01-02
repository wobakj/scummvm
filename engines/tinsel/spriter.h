/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Spriter - 3D Renderer
 */

#ifndef	TINSEL_SPRITER_H
#define	TINSEL_SPRITER_H

#include "tinsel/dw.h"
#include "tinsel/events.h"

#include "common/stack.h"

namespace Tinsel {

struct Vertex3i {
    int32 x,y,z;
};

struct Vertex3f {
    float x,y,z;
};

struct Vertex2f {
    float x,y;
};

struct Vertex2c {
    int x,y;
    int flags;
};

struct Matrix {
    float m[3][3];
    float t[3];
};

typedef Common::FixedStack<Matrix, 30> MatrixStack;
typedef Common::Array<Vertex3f> Vertices3f;


enum RenderProgramOp : uint16_t {
    MATRIX_DUPLICATE    = 1,
    MATRIX_REMOVE       = 2,
    U3                  = 3,
    TRANSFORM           = 4,
    TRANSLATE_X         = 5,
    TRANSLATE_Y         = 6,
    TRANSLATE_Z         = 7,
    TRANSLATE_XYZ       = 8,
    ROTATE_X            = 9,
    ROTATE_Y            = 10,
    ROTATE_Z            = 11,
    ROTATE_XYZ          = 17,
    STOP                = 16,
};

struct AnimationInfo {
    Common::String name;

    uint meshNum;

    uint translateTablesHunk;
    uint translateTables;
    uint translateNum;

    uint rotateTablesHunk;
    uint rotateTables;
    uint rotateNum;

    uint scaleTablesHunk;
    uint scaleTables;
    uint scaleNum;
};

struct MeshInfo {
    uint meshTablesHunk;
    uint meshTables;

    uint renderProgramHunk;
    uint renderProgram;
};

struct RBHEntry {
    Common::Array<uint8_t> data;
    Common::Array<uint> mappingIdx;
    uint size;
    uint flags;
};

typedef Common::Array<RBHEntry> RBH;

struct Primitive {
    uint indices[8];
    uint color;
    Vertex2f uv[4];
    uint texture;
};

struct MeshPart {
    uint type;
    uint cull;
    uint numVertices;
    Common::Array<Primitive> primitives;
};

struct Mesh {
    Common::Array<Vertex3f> vertices;
    Common::Array<Vertex3f> normals;
    Common::Array<MeshPart> parts;
    Common::Array<MeshPart> parts2;
};

struct Meshes {
    uint vertexCount;
    uint normalCount;

    Common::Array<Mesh> meshes;
};

struct VecFTables {
    Common::Array<Common::Array<Vertex3f>> frame;
};

struct VecITables {
    Common::Array<Common::Array<Vertex3i>> frame;
};

struct ModelTables {
    Common::Array<Vertex3f> translationTable;
    Common::Array<Vertex3i> rotationTable;
    Common::Array<Vertex3f> scaleTable;
    Meshes                  meshes;
};

struct Model {
    RBH rbh;
    RBH rbhU;
    // void * vmc;
    uint animationCount;
    uint field_0xe;
    uint field_0xf;
    uint8_t* renderProgram;

    // animation tables
    VecITables startRotateTables;
    VecFTables startScaleTables;
    VecFTables startTranslateTables;
    VecITables endRotateTables;
    VecFTables endScaleTables;
    VecFTables endTranslateTables;
    uint startFrame;
    uint endFrame;

    uint flags;
    uint field_0x32;
    uint field_0x33;

    ModelTables tables;

    uint interpolant;
};

struct Viewport {
    int ap;
    float width;
    float height;

    Common::Rect rect;
};

struct View {
    int centerX;
    int centerY;

    Common::Rect viewRect;
    Common::Rect screenRect;

    Viewport viewport;

    float cameraPosX;
    float cameraPosY;
    float cameraPosZ;
    int cameraRotX;
    int cameraRotY;
    int cameraRotZ;
};

class Spriter {
private:
    MatrixStack _modelMatrix;
    MatrixStack* _currentMatrix;

    Common::Array<AnimationInfo> _animMain;
    AnimationInfo _animShadow;

    MeshInfo _meshMain;
    Common::Array<MeshInfo> _meshShadow;

    Model _modelMain;
    Model _modelShadow;

    View _view;

public:
    void Init(int width, int height);

    void SetCamera(short rotX,short rotY,short rotZ,int posX,int posY,int posZ,int cameraAp);

    void TransformSceneXYZ(int x, int y, int z, Vertex2c& v);

    void LoadModel(const char* modelName, const char* textureName);

private:
    // Matrix operations
    const Matrix& MatrixCurrent() const;
    void MatrixReset();
    void MatrixIdentity();
    void MatrixRemove();
    void MatrixDuplicate();
    void MatrixMulVertex(Matrix &m, Vertex3f& v);
    void MatrixTranslate(float x, float y, float z);
    void MatrixRotateX(int angle);
    void MatrixRotateY(int angle);
    void MatrixRotateZ(int angle);

    void SetViewport(int ap);

    void TransformVertices(Vertex3f* vOut, Vertex3f* vIn, int count);

    // Loading of model
    void LoadH(const char* modelName);
    void LoadGBL(const char* modelName);
    void LoadRBH(const char* modelName, RBH& rbh);

    Meshes LoadMeshes(RBH rbh, uint table1, uint index1, uint frame);
    VecFTables LoadTableVector3f(RBH rbh, uint table, uint offset);
    VecITables LoadTableVector3i(RBH rbh, uint table, uint offset);
    void InitModel(Model& model, MeshInfo &meshInfo, Common::Array<AnimationInfo> &animInfo);

    // Rendering
    void RunRenderProgram(Model &model, bool initial);

    void FindSimilarVertices(Mesh& mesh, Vertices3f &verticesTransformed, Common::Array<uint16>& sameVertices) const;
    void MergeVertices(Mesh &mesh, Common::Array<uint16>& sameVertices);

    void TransformAndRenderMesh(Mesh& mesh, Vertices3f& verticesTransformed);
    void TransformMesh(Mesh& mesh, Vertices3f& verticesTransformed);
    void RenderMeshParts(Mesh& mesh, Vertices3f& verticesTransformed);
    void RenderMeshPartColor(MeshPart& part, Vertices3f& verticesTransformed);
    void RenderMeshPartTexture(MeshPart& part, Vertices3f& verticesTransformed);
};

} // End of namespace Tinsel

#endif	// TINSEL_SPRITER_H
