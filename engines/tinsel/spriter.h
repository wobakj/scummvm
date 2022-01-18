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
#include "common/str.h"

#include "math/vector3d.h"
#include "math/vector2d.h"
#include "math/matrix4.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
#include "graphics/opengl/system_headers.h"
#endif


namespace Tinsel {

typedef Common::FixedStack<Math::Matrix4, 30> MatrixStack;
typedef Common::Array<Math::Vector3d> Vertices3f;


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
	Math::Vector2d uv[4];
	uint texture;
};

struct MeshPart {
	uint type;
	uint cull;
	uint numVertices;
	Common::Array<Primitive> primitives;
};

struct Mesh {
	Common::Array<Math::Vector3d> vertices;
	Common::Array<Math::Vector3d> normals;
	Common::Array<MeshPart> parts;
	Common::Array<MeshPart> parts2;
};

struct Meshes {
	uint vertexCount;
	uint normalCount;

	Common::Array<Mesh> meshes;
};

struct VecFTables {
	Common::Array<Common::Array<Math::Vector3d>> frame;
};


struct ModelTables {
	Common::Array<Math::Vector3d> translationTable;
	Common::Array<Math::Vector3d> rotationTable;
	Common::Array<Math::Vector3d> scaleTable;
	Meshes                        meshes;
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
	VecFTables startRotateTables;
	VecFTables startScaleTables;
	VecFTables startTranslateTables;
	VecFTables endRotateTables;
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

	View _view;

	Common::Array<uint8> _palette;
	Common::Array<uint8> _textureData;

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	bool _textureGenerated;
	GLuint _texture[4];
#endif

public:
	Model _modelMain;
	Model _modelShadow;

public:
	Spriter();
	virtual ~Spriter();

	void Init(int width, int height);
	void SetCamera(short rotX,short rotY,short rotZ,int posX,int posY,int posZ,int cameraAp);
	void TransformSceneXYZ(int x, int y, int z, int& xOut, int& yOut);
	void LoadModel(const Common::String& modelName, const Common::String& textureName);
	void RenderModel(Model& model);

	void UpdatePalette(SCNHANDLE hPalette);

private:
	const Math::Matrix4& MatrixCurrent() const;

	void MatrixReset();

	void MatrixPop();
	void MatrixPush();
	void MatrixTranslate(float x, float y, float z);
	void MatrixRotateX(float angle);
	void MatrixRotateY(float angle);
	void MatrixRotateZ(float angle);

	void SetViewport(int ap);

	// Loading of model
	void LoadH(const Common::String& modelName);
	void LoadGBL(const Common::String& modelName);
	void LoadRBH(const Common::String& modelName, RBH& rbh);
	void LoadVMC(const Common::String& textureName);

	void UpdateTextures();

	Meshes LoadMeshes(RBH rbh, uint table1, uint index1, uint frame);
	VecFTables LoadTableVector3f(RBH rbh, uint table, uint offset);
	VecFTables LoadTableVector3i(RBH rbh, uint table, uint offset);
	void InitModel(Model& model, MeshInfo& meshInfo, Common::Array<AnimationInfo>& animInfo);

	// Rendering
	void RunRenderProgram(Model &model, bool initial);

	void FindSimilarVertices(Mesh& mesh, Vertices3f& verticesTransformed, Common::Array<uint16>& sameVertices) const;
	void MergeVertices(Mesh& mesh, Common::Array<uint16>& sameVertices);

	void TransformMesh(Mesh& mesh, Vertices3f& verticesTransformed);
	void RenderMesh(Mesh& mesh, Vertices3f& verticesTransformed);
	void RenderMeshPartColor(MeshPart& part, Vertices3f& verticesTransformed);
	void RenderMeshPartTexture(MeshPart& part, Vertices3f& verticesTransformed);
};

} // End of namespace Tinsel

#endif	// TINSEL_SPRITER_H
