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

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dialogs.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/mareels.h"
#include "tinsel/move.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"	// process IDs
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/movers.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"	// stopAllSamples()
#include "tinsel/spriter.h"
#include "tinsel/sysvar.h"
#include "tinsel/token.h"

#include "common/rect.h"

#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/file.h"

namespace Tinsel {

const Matrix& Spriter::MatrixCurrent() const {
    return _currentMatrix->top();
}

void Spriter::MatrixReset() {
    _currentMatrix->clear();
    MatrixIdentity();
}

void Spriter::MatrixRemove() {
    _currentMatrix->pop();
}

void Spriter::MatrixDuplicate() {
    _currentMatrix->push(_currentMatrix->top());
}

void Spriter::MatrixMulVertex(Matrix &m, Vertex3f& v) {
    float x = v.x;
    float y = v.y;
    float z = v.z;

    v.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z;
    v.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z;
    v.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z;
}

void Spriter::MatrixIdentity() {
    Matrix identity {};
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;

    _currentMatrix->push(identity);
}

void Spriter::MatrixTranslate(float x, float y, float z) {
    Vertex3f v {x,y,z};
    Matrix &m = _currentMatrix->top();
    MatrixMulVertex(m, v);
    m.t[0] += v.x;
    m.t[1] += v.y;
    m.t[2] += v.z;
}

void Spriter::MatrixRotateX(int angle) {
    Matrix &m = _currentMatrix->top();

    float a = ((angle & 0xfff) / 4095.0f) * 2.0f * M_PI;
    float s = sinf(a);
    float c = cosf(a);

    float m01 = m.m[0][1];
    float m02 = m.m[0][2];

    float m11 = m.m[1][1];
    float m12 = m.m[1][2];

    float m21 = m.m[2][1];
    float m22 = m.m[2][2];


    m.m[0][2] = m.m[0][1] * -s + m.m[0][2] * c;
    m.m[0][1] = m01       *  c + m02       * s;

    m.m[1][2] = m.m[1][1] * -s + m.m[1][2] * c;
    m.m[1][1] = m11       *  c + m12       * s;

    m.m[2][2] = m.m[2][1] * -s + m.m[2][2] * c;
    m.m[2][1] = m21       *  c + m22       * s;
}

void Spriter::MatrixRotateY(int angle) {
    Matrix &m = _currentMatrix->top();

    float a = ((angle & 0xfff) / 4095.0f) * 2.0f * M_PI;
    float s = sinf(a);
    float c = cosf(a);

    float m00 = m.m[0][0];
    float m02 = m.m[0][2];

    float m10 = m.m[1][0];
    float m12 = m.m[1][2];

    float m20 = m.m[2][0];
    float m22 = m.m[2][2];

    m.m[0][2] = m.m[0][0] * s + m.m[0][2] * c;
    m.m[0][0] = m00       * c - m02       * s;

    m.m[1][2] = m.m[1][0] * s + m.m[1][2] * c;
    m.m[1][0] = m10       * c - m12       * s;

    m.m[2][2] = m.m[2][0] * s + m.m[2][2] * c;
    m.m[2][0] = m20       * c - m22       * s;
}

void Spriter::MatrixRotateZ(int angle) {
    Matrix &m = _currentMatrix->top();

    float a = ((angle & 0xfff) / 4095.0f) * 2.0f * M_PI;
    float s = sinf(a);
    float c = cosf(a);

    float m00 = m.m[0][0];
    float m01 = m.m[0][1];

    float m10 = m.m[1][0];
    float m11 = m.m[1][1];

    float m20 = m.m[2][0];
    float m21 = m.m[2][1];

    m.m[0][1] = m.m[0][0] * -s + m.m[0][1] * c;
    m.m[0][0] = m00       *  c + m01       * s;

    m.m[1][1] = m.m[1][0] * -s + m.m[1][1] * c;
    m.m[1][0] = m10       *  c + m11       * s;

    m.m[2][1] = m.m[2][0] * -s + m.m[2][1] * c;
    m.m[2][0] = m20       *  c + m21       * s;
}

void Spriter::SetViewport(int ap) {
    _view.viewport.ap = ap;

    int ratio = ((_view.screenRect.right - _view.screenRect.left) << 12) / ap;
    _view.viewport.width = ratio;
    _view.viewport.height = ratio;

    _view.viewport.ap = (ap + 1800) / 3600;

    _view.viewport.rect = _view.viewRect;
}


void Spriter::Init(int width, int height) {

    bool is3D = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
    
    _currentMatrix = &_modelMatrix;
    _meshShadow.resize(50);


    _view.screenRect.left   = 0;
    _view.screenRect.top    = 0;
    _view.screenRect.right  = width;
    _view.screenRect.bottom = height;

    _view.centerX = width / 2;
    _view.centerY = height / 2;

    _view.viewRect.left   = -_view.centerX;
    _view.viewRect.top    = -_view.centerY;
    _view.viewRect.right  =  _view.screenRect.right  - _view.screenRect.left - _view.centerX - 1;
    _view.viewRect.bottom =  _view.screenRect.bottom - _view.screenRect.top  - _view.centerY;

    SetViewport(306030);
}


void Spriter::SetCamera(short rotX, short rotY, short rotZ, int posX, int posY, int posZ, int cameraAp) {
    _view.cameraPosX = posX * 0.01f;
    _view.cameraPosY = posY * 0.01f;
    _view.cameraPosZ = posZ * 0.01f;
    _view.cameraRotX = rotX;
    _view.cameraRotY = rotY;
    _view.cameraRotZ = rotZ;

    SetViewport(cameraAp);
}

void Spriter::TransformVertices(Vertex3f* vOut, Vertex3f* vIn, int count) {
    const Matrix &m = MatrixCurrent();

    float viewportWidth = _view.viewport.width;
    float viewportHeight = _view.viewport.height;
    for (int i = 0; i < count; ++i, ++vIn, ++vOut) {
        vOut->x = m.m[0][0] * vIn->x + m.t[0] + m.m[0][1] * vIn->y + m.m[0][2] * vIn->z;
        vOut->y = m.m[1][0] * vIn->x + m.t[1] + m.m[1][1] * vIn->y + m.m[1][2] * vIn->z;
        vOut->z = m.m[2][0] * vIn->x + m.t[2] + m.m[2][1] * vIn->y + m.m[2][2] * vIn->z;

        if (vOut->z <= 1.0f) {
            //vOut->clipFlags = CLIP_FRONT;
            vOut->x *= viewportWidth;
            vOut->y *= viewportHeight;
        } else {
            float z = 1.0F / vOut->z;
            //vp->clipFlags = NO_CLIP;
            vOut->x *= z * viewportWidth;
            vOut->y *= z * viewportHeight;
        }

        // vOut->x = _view.centerX + vOut->cx;
        // vOut->y = _view.centerY + vOut->y;

        // if ((ushort)((ushort)((float)ppiVar1->left < screenX) << 8 |
        //             (ushort)((float)ppiVar1->left == screenX) << 0xe) == 0) {
        //     ppiVar1->left = (int)ROUND(vp->x);
        // }
        // if ((float)ppiVar1->right < vp->x) {
        //     ppiVar1->right = (int)ROUND(vp->x);
        // }
        // screenY = vp->y;
        // screenY_ = vp->y;
        // if ((ushort)((ushort)((float)ppiVar1->top < screenY) << 8 |
        //             (ushort)((float)ppiVar1->top == screenY) << 0xe) == 0) {
        //     ppiVar1->top = (int)ROUND(screenY_);
        //     screenY_ = vp->y;
        //     ppiVar1 = _instancePtr->_zBufferRegions;
        // }
        // if ((float)ppiVar1->bottom < screenY_) {
        //     ppiVar1->bottom = (int)ROUND(screenY_);
        //     screenY_ = vp->y;
        //     screenX_ = vp->x;
        // }
        // else {
        //     screenX_ = vp->x;
        // }


    //   z = (float)(viewport->rect).left;
    //   if ((ushort)((ushort)(z < screenX_) << 8 | (ushort)(z == screenX_) << 0xe) == 0) {
    //     vp->clipFlags = vp->clipFlags | CLIP_LEFT;
    //   }
    //   else {
    //     if ((float)(viewport->rect).right <= screenX_) {
    //       vp->clipFlags = vp->clipFlags | CLIP_RIGHT;
    //     }
    //   }
    //   z = (float)(viewport->rect).top;
    //   if ((ushort)((ushort)(z < screenY_) << 8 | (ushort)(z == screenY_) << 0xe) == 0) {
    //     vp->clipFlags = vp->clipFlags | CLIP_TOP;
    //   }
    //   else {
    //     if ((float)(viewport->rect).bottom <= screenY_) {
    //       vp->clipFlags = vp->clipFlags | CLIP_BOTTOM;
    //     }
    //   }

    }
}

void Spriter::TransformSceneXYZ(int x, int y, int z, Vertex2c& v) {
    MatrixReset();
    MatrixRotateX(_view.cameraRotX);
    MatrixRotateY(_view.cameraRotY);
    MatrixRotateZ(_view.cameraRotZ);
    MatrixTranslate(-_view.cameraPosX, -_view.cameraPosY, -_view.cameraPosZ);
    MatrixTranslate(x / 100.0f, y / 100.0f, z / 100.0f);

    Vertex3f vIn {}, vOut{};
    TransformVertices(&vOut, &vIn, 1);

    v.x = _view.centerX + vOut.x;
    v.y = _view.centerY + vOut.y;
}

void Spriter::LoadH(const char* modelName) {
    Common::String filename = modelName;
    filename += ".h";

    Common::File f;
    f.open(filename);

    while(!f.eos()) {
        Common::String line = f.readLine();
        if (!line.hasPrefix("#define")) {
            continue;
        }
        line.erase(0, 8); // remove "#define "

        size_t underscorePos = line.findFirstOf('_');

        AnimationInfo *anim = nullptr;

        Common::String name = line.substr(0, underscorePos);

        line.erase(0, name.size() + 1); // remove the underscore too

        if (name.hasPrefix("SHADOW")) {
            anim = &_animShadow;
        } else {
            for (Common::Array<AnimationInfo>::iterator it = _animMain.begin(); it != _animMain.end(); ++it) {
                if (it->name.equals(name)) {
                    anim = it;
                    break;
                }
            }

            if (anim == nullptr) {
                AnimationInfo tempAnim {};
                tempAnim.name = name;
                _animMain.push_back(tempAnim);
                anim = &_animMain.back();
            }
        }

        size_t spacePos = line.findFirstOf(' ');
        Common::String sub = line.substr(0, spacePos);
        auto val = atoi(line.substr(spacePos).c_str());

        if (sub.equals("MESH_NUM")) {
            anim->meshNum = val;
        } else if (sub.equals("SCALE_NUM")) {
            anim->scaleNum = val;
        } else if (sub.equals("TRANSLATE_NUM")) {
            anim->translateNum = val;
        } else if (sub.equals("ROTATE_NUM")) {
            anim->rotateNum = val;
        }
    }
}

void Spriter::LoadGBL(const char* modelName) {
    Common::String filename = modelName;
    filename += ".gbl";

    Common::File f;
    f.open(filename);

    while(!f.eos()) {
        Common::String line = f.readLine();
        if (!line.hasPrefix("#define")) {
            continue;
        }
        line.erase(0, 8); // remove "#define "

        size_t underscorePos = line.findFirstOf('_');

        AnimationInfo *anim = nullptr;
        MeshInfo *mesh = nullptr;

        Common::String name = line.substr(0, underscorePos);

        line.erase(0, name.size() + 1); // remove the underscore too

        uint meshId = 0;
        if (name.hasPrefix("SHADOW")) {
            anim = &_animShadow;
            meshId = atoi(name.substr(6).c_str());
            assert(meshId < _meshShadow.size());
            mesh = &_meshShadow[meshId];
        } else {
            for (Common::Array<AnimationInfo>::iterator it = _animMain.begin(); it != _animMain.end(); ++it) {
                if (it->name.equals(name)) {
                    anim = it;
                    break;
                }
            }
            mesh = &_meshMain;
        }

        assert(anim != nullptr);
        assert(mesh != nullptr);

        size_t spacePos = line.findFirstOf(' ');
        Common::String sub = line.substr(0, spacePos);
        auto val = atoi(line.substr(spacePos).c_str());

        if (sub.equals("MESH_TABLES")) {
            mesh->meshTables = val;
        } else if (sub.equals("MESH_TABLES_hunk")) {
            mesh->meshTablesHunk = val;
        } else if (sub.equals("RENDER_PROGRAM")) {
            mesh->renderProgram = val;
        } else if (sub.equals("RENDER_PROGRAM_hunk")) {
            mesh->renderProgramHunk = val;
        } else if (sub.equals("TRANSLATE_TABLES")) {
            anim->translateTables = val;
        } else if (sub.equals("TRANSLATE_TABLES_hunk")) {
            anim->translateTablesHunk = val;
        } else if (sub.equals("ROTATE_TABLES")) {
            anim->rotateTables = val;
        } else if (sub.equals("ROTATE_TABLES_hunk")) {
            anim->rotateTablesHunk = val;
        } else if (sub.equals("SCALE_TABLES")) {
            anim->scaleTables = val;
        } else if (sub.equals("SCALE_TABLES_hunk")) {
            anim->scaleTablesHunk = val;
        }
    }
}

#define kPIFF 0x46464950
#define kRBHF 0x46484252
#define kRBHH 0x48484252
#define kBODY 0x59444f42
#define kRELC 0x434c4552

void Spriter::LoadRBH(const char* modelName, RBH& rbh) {
    Common::String filename = modelName;
    filename += ".rbh";

    Common::File f;
    f.open(filename);

    uint tag = f.readUint32LE();
    assert(tag == kPIFF);
    uint fileSize = f.readUint32LE();

    tag = f.readUint32LE();
    assert(tag == kRBHF);
    tag = f.readUint32LE();
    assert(tag == kRBHH);
    uint headerSize = f.readUint32LE();
    uint entriesCount = headerSize / 12;

    rbh.resize(entriesCount);
    for (RBH::iterator it = rbh.begin(); it != rbh.end(); ++it) {
        f.skip(4); // pointer to data
        it->size = f.readUint32LE();
        it->flags = f.readUint16LE();
        f.skip(2); // padding
        it->data.resize(it->size);
    }

    uint entryIdx = 0;
    while (f.pos() < fileSize) {
        tag = f.readUint32LE();
        uint size = f.readUint32LE();
        if (tag == kBODY) {
            f.read(rbh[entryIdx].data.data(), size);
            ++entryIdx;
        } else if (tag == kRELC) {
            uint srcIdx = f.readUint32LE();
            uint dstIdx = f.readUint32LE();
            // uint entries = (size - sizeof(uint32) * 2) / sizeof(uint32);
            // uint32* dstDataPtr = (uint32*)_rbh[dstIdx].data.data();
            // while (entries > 0)
            // {
            //     uint offset = f.readUint32LE();
            //     --entries;
            // }
            f.skip(size - 8);
            rbh[srcIdx].mappingIdx.push_back(dstIdx);
        } else {
            assert(false);
        }
    }
}

Meshes Spriter::LoadMeshes(RBH rbh, uint table1, uint index1, uint frame) {
    assert(table1 < rbh.size());

    Common::MemoryReadStream s1(rbh[table1].data.data(), rbh[table1].data.size());
    s1.skip(index1);

    uint numFrames = s1.readUint16LE();
    uint numEntries = s1.readUint16LE();

    assert(frame < numFrames);

    s1.skip(frame * 4);
    uint index2 = s1.readUint32LE();

    uint table2 = rbh[table1].mappingIdx[0];
    Common::MemoryReadStream s2(rbh[table2].data.data(), rbh[table2].data.size());
    s2.skip(index2);

    Meshes result;
    result.vertexCount = s2.readUint32LE();
    result.normalCount = s2.readUint32LE();
    result.meshes.resize(numEntries);

    for (auto& mesh : result.meshes)
    {
        uint index3 = s2.readUint32LE();
        uint table3 = rbh[table2].mappingIdx[0];

        Common::MemoryReadStream s3(rbh[table3].data.data(), rbh[table3].data.size());
        s3.skip(index3);


        uint index4 = s3.readUint32LE();
        uint table4 = rbh[table3].mappingIdx[1];

        mesh.vertices.resize(s3.readUint32LE());

        Common::MemoryReadStream s4(rbh[table4].data.data(), rbh[table4].data.size());
        s4.skip(index4);

        for (auto& v : mesh.vertices) {
            v.x = s4.readFloatLE();
            v.y = s4.readFloatLE();
            v.z = s4.readFloatLE();
        }

        uint index5 = s3.readUint32LE();
        uint table5 = rbh[table3].mappingIdx[1];

        mesh.normals.resize(s3.readUint32LE());

        Common::MemoryReadStream s5(rbh[table5].data.data(), rbh[table5].data.size());
        s5.skip(index5);

        for (auto& n : mesh.normals) {
            n.x = s5.readFloatLE();
            n.y = s5.readFloatLE();
            n.z = s5.readFloatLE();
        }

        uint index6 = s3.readUint32LE();
        uint table6 = rbh[table3].mappingIdx[0];

        Common::MemoryReadStream s6(rbh[table6].data.data(), rbh[table6].data.size());
        s6.skip(index6);

        while (true) {
            uint primitiveCount = s6.readUint16LE();
            uint primitiveType = s6.readUint16LE();
            uint dataSize = s6.readUint32LE();
            if (primitiveCount == 0) {
                break;
            }

            MeshPart part;
            part.numVertices = (primitiveType & 1) ? 4 : 3;
            part.type = primitiveType & 0x7f;
            part.cull = primitiveType & 0x80;
            part.primitives.resize(primitiveCount);

            for (auto& prim : part.primitives) {
                for (uint i = 0; i < 8; ++i) {
                    prim.indices[i] = s6.readUint16LE();
                }

                if (part.type == 0 || part.type == 1) {
                    prim.color = s6.readUint32LE();
                } else if (part.type == 2 || part.type == 3) {
                    assert(false); //not supported?
                } else if (part.type == 4 || part.type == 5) {
                    for (uint i = 0; i < part.numVertices; ++i) {
                        prim.uv[i].x = s6.readFloatLE();
                        prim.uv[i].y = s6.readFloatLE();
                    }
                    prim.texture = s6.readUint16LE();
                    s6.skip(2); //padding
                }
            }

            mesh.parts.push_back(part);
        }
    }

    return result;
}

VecFTables Spriter::LoadTableVector3f(RBH rbh, uint table, uint offset)
{
    assert(table < rbh.size());

    Common::MemoryReadStream s1(rbh[table].data.data(), rbh[table].data.size());
    s1.skip(offset);

    uint numFrames = s1.readUint16LE();
    uint numEntries = s1.readUint16LE();

    VecFTables result;
    result.frame.resize(numFrames);

    uint table2 = rbh[table].mappingIdx[0];

    for (uint frame = 0; frame < numFrames; frame++) {
        auto& frameEntries = result.frame[frame];
        uint index = s1.readUint32LE();

        Common::MemoryReadStream s2(rbh[table2].data.data(), rbh[table2].data.size());
        s2.skip(index);

        frameEntries.resize(numEntries);

        for (auto& it : frameEntries) {
            it.x = s2.readFloatLE();
            it.y = s2.readFloatLE();
            it.z = s2.readFloatLE();
        }
    }

    return result;
}

VecITables Spriter::LoadTableVector3i(RBH rbh, uint table, uint offset)
{
    assert(table < rbh.size());

    Common::MemoryReadStream s1(rbh[table].data.data(), rbh[table].data.size());
    s1.skip(offset);

    uint numFrames = s1.readUint16LE();
    uint numEntries = s1.readUint16LE();

    VecITables result;
    result.frame.resize(numFrames);

    uint table2 = rbh[table].mappingIdx[0];

    for (uint frame = 0; frame < numFrames; frame++) {
        auto& frameEntries = result.frame[frame];
        uint index = s1.readUint32LE();

        Common::MemoryReadStream s2(rbh[table2].data.data(), rbh[table2].data.size());
        s2.skip(index);

        frameEntries.resize(numEntries);

        for (auto& it : frameEntries) {
            it.x = s2.readUint32LE();
            it.y = s2.readUint32LE();
            it.z = s2.readUint32LE();
        }
    }

    return result;
}

void Spriter::InitModel(Model &model, MeshInfo &meshInfo, Common::Array<AnimationInfo> &animInfos) {
    model.renderProgram             = model.rbh[meshInfo.renderProgramHunk].data.data() + meshInfo.renderProgram;

    AnimationInfo &animInfo = animInfos[0];
    model.tables.meshes             = LoadMeshes(model.rbh, meshInfo.meshTablesHunk, meshInfo.meshTables, 0);
    model.tables.translationTable   = LoadTableVector3f(model.rbh, animInfo.translateTablesHunk, animInfo.translateTables).frame[0];
    model.tables.rotationTable      = LoadTableVector3i(model.rbh, animInfo.rotateTablesHunk, animInfo.rotateTables).frame[0];
    model.tables.scaleTable         = LoadTableVector3f(model.rbh, animInfo.scaleTablesHunk, animInfo.scaleTables).frame[0];

    _currentMatrix = &_modelMatrix;

    MatrixReset();
    MatrixIdentity();

    RunRenderProgram(model, true);

}

void Spriter::RunRenderProgram(Model &model, bool initial)
{
    uint8* program = model.renderProgram;
    uint ip = 0;

    Vertices3f vertices;
    vertices.reserve(model.tables.meshes.vertexCount);

    Vertices3f normals;
    normals.reserve(model.tables.meshes.normalCount);

    Common::Array<uint16> sameVertices;
    sameVertices.resize(model.tables.meshes.vertexCount);

    bool print = true;
    bool stop = false;
    do
    {
        // if (print ) printf("%4i %2i - ", ip, _matrices.count);

        RenderProgramOp opCode = (RenderProgramOp)READ_LE_UINT16(&program[ip]);
        ip += 2;

        switch(opCode)
        {
            case MATRIX_DUPLICATE:
            {
                // if (print) printf("MATRIX_DUPL\n");
                MatrixDuplicate();
                break;
            }
            case MATRIX_REMOVE:
            {
                // if (print) printf("MATRIX_REMOVE\n");
                MatrixRemove();
                break;
            }
            case U3:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;
                // if (print)
                // {
                //     printf("U3 ");
                //     printf("mesh entry id: %i\n", entry);
                // }
                // local_14 = spriter_;
                // spriter::renderProgram_FUN_0040ea10(spriter_,(&pSVar5->meshTable->entries)[entry],&spriter_->u2);
                break;
            }
            case TRANSFORM:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);

                Mesh& mesh = model.tables.meshes.meshes[entry];
                ip += 2;

                // if (print)
                // {
                //     printf("TRANSFORM ");
                //     printf("mesh entry id: %i\n", entry);
                //     printMatrix();
                // }

                if (initial)
                {
                    FindSimilarVertices(mesh, vertices, sameVertices);
                    MergeVertices(mesh, sameVertices);
                }
                else
                {
                    TransformAndRenderMesh(mesh, vertices);
                }

                break;
            }
            case TRANSLATE_X:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3f& v = model.tables.translationTable[entry];

                // if (print)
                // {
                //     printf("TRANSLATE_X ");
                //     printVertex(v);
                //     printf("\n");
                // }

                MatrixTranslate(v.x, 0, 0);

                // printMatrix();

                break;
            }
            case TRANSLATE_Y:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3f& v = model.tables.translationTable[entry];

                // if (print)
                // {
                //     printf("TRANSLATE_Y ");
                //     printVertex(v);
                //     printf("\n");
                // }

                MatrixTranslate(0, v.y, 0);

                // printMatrix();

                break;
            }
            case TRANSLATE_Z:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3f& v = model.tables.translationTable[entry];

                // if (print)
                // {
                //     printf("TRANSLATE_Z ");
                //     printVertex(v);
                //     printf("\n");
                // }

                MatrixTranslate(0, 0, v.z);

                // printMatrix();

                break;
            }
            case TRANSLATE_XYZ:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3f& v = model.tables.translationTable[entry];

                // if (print)
                // {
                //     printf("TRANSLATE_XYZ ");
                //     printVertex(v);
                //     printf("\n");
                // }

                MatrixTranslate(v.x, v.y, v.z);

                // printMatrix();

                break;
            }
            case ROTATE_X:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3i& v = model.tables.rotationTable[entry];

                // if (print)
                // {
                //     printf("ROTATE_X ");
                //     printVertexI(v);
                //     printf("\n");
                // }

                MatrixRotateX(v.x);

                break;
            }
            case ROTATE_Y:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3i& v = model.tables.rotationTable[entry];

                // if (print)
                // {
                //     printf("ROTATE_Y ");
                //     printVertexI(v);
                //     printf("\n");
                // }

                MatrixRotateY(v.y);

                break;
            }
            case ROTATE_Z:
            {
                uint16 entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3i& v = model.tables.rotationTable[entry];

                // if (print)
                // {
                //     printf("ROTATE_Z ");
                //     printVertexI(v);
                //     printf("\n");
                // }

                MatrixRotateZ(v.z);

                break;
            }
            case STOP:
            {
                // if (print) printf("STOP\n");
                stop = true;
                break;
            }
            case ROTATE_XYZ:
            {
                uint entry = READ_LE_UINT16(&program[ip]);
                ip += 2;

                Vertex3i& v = model.tables.rotationTable[entry];

                // if (print)
                // {
                //     printf("ROTATE_XYZ ");
                //     printVertexI(v);
                //     printf("\n");
                // }

                MatrixRotateX(v.x);
                MatrixRotateY(v.y);
                MatrixRotateZ(v.z);

                // printMatrix();

                break;
            }
            default:
            {
                error("UNKNOWN RENDER OP %i\n", opCode);
            }
        }

    } while (!stop);

}

void Spriter::FindSimilarVertices(Mesh& mesh, Vertices3f& verticesTransformed, Common::Array<uint16>& sameVertices) const {
    const Matrix &m = MatrixCurrent();

    uint i_start = verticesTransformed.size();
    for (uint i = 0; i < mesh.vertices.size(); ++i) {
        Vertex3f& vIn = mesh.vertices[i];

        Vertex3f vOut;
        vOut.x = m.m[0][0] * vIn.x + m.m[0][1] * vIn.y + m.m[0][2] * vIn.z + m.t[0];
        vOut.y = m.m[1][0] * vIn.x + m.m[1][1] * vIn.y + m.m[1][2] * vIn.z + m.t[1];
        vOut.z = m.m[2][0] * vIn.x + m.m[2][1] * vIn.y + m.m[2][2] * vIn.z + m.t[2];
        verticesTransformed.push_back(vOut);

        for (uint j = 0; j < verticesTransformed.size() - 1; ++j) {
            float dX = vOut.x - verticesTransformed[j].x;
            float dY = vOut.y - verticesTransformed[j].y;
            float dZ = vOut.z - verticesTransformed[j].z;
            if (dZ * dZ + dX * dX + dY * dY < 0.01f) { // this is too big maybe?
                sameVertices[i_start + i] = j + 1; // 0 is reserved for not found
                break;
            }
        }
    }
    return;
}

void Spriter::MergeVertices(Mesh &mesh, Common::Array<uint16>& sameVertices) {
    for (auto& part : mesh.parts) {
        for (auto& prim : part.primitives) {
            for (uint i = 0; i < part.numVertices; ++i) {
                if (sameVertices[prim.indices[i]] != 0) {
                    prim.indices[i] = sameVertices[prim.indices[i]] - 1;
                }
            }
        }
    }
}

void Spriter::TransformAndRenderMesh(Mesh& mesh, Vertices3f& verticesTransformed) {
    TransformMesh(mesh, verticesTransformed);
    RenderMeshParts(mesh, verticesTransformed);
}


void Spriter::TransformMesh(Mesh& mesh, Vertices3f& verticesTransformed) {
    const Matrix &m = MatrixCurrent();

    float viewportWidth = _view.viewport.width;
    float viewportHeight = _view.viewport.height;

    for(auto& vIn : mesh.vertices) {
        Vertex3f vOut;
        vOut.x = m.m[0][0] * vIn.x + m.m[0][1] * vIn.y + m.m[0][2] * vIn.z + m.t[0];
        vOut.y = m.m[1][0] * vIn.x + m.m[1][1] * vIn.y + m.m[1][2] * vIn.z + m.t[1];
        vOut.z = m.m[2][0] * vIn.x + m.m[2][1] * vIn.y + m.m[2][2] * vIn.z + m.t[2];

        if (vOut.z <= 1.0f) {
            //vOut->clipFlags = CLIP_FRONT;
            vOut.x *= viewportWidth;
            vOut.y *= viewportHeight;
        } else {
            float z = 1.0F / vOut.z;
            //vp->clipFlags = NO_CLIP;
            vOut.x *= z * viewportWidth;
            vOut.y *= z * viewportHeight;
        }

        verticesTransformed.push_back(vOut);

        // vp->z = vp->sZ;
        // vp->x = (float)spriter::_instancePtr->viewportCenterX + vp->x;
        // vp->y = (float)spriter::_instancePtr->viewportCenterY + vp->y;

        // ppiVar1 = spriter::_instancePtr->UBclipRect;
        // if ((ushort)((ushort)((float)ppiVar1->left < screenX) << 8 |
        //             (ushort)((float)ppiVar1->left == screenX) << 0xe) == 0) {
        //     ppiVar1->left = (int)ROUND(vp->x);
        //     ppiVar1 = spriter::_instancePtr->UBclipRect;
        // }
        // if ((float)ppiVar1->right < vp->x) {
        //     ppiVar1->right = (int)ROUND(vp->x);
        //     ppiVar1 = spriter::_instancePtr->UBclipRect;
        // }

        // screenY = vp->y;
        // screenY_ = vp->y;
        // if ((ushort)((ushort)((float)ppiVar1->top < screenY) << 8 | (ushort)((float)ppiVar1->top == screenY) << 0xe)
        //     == 0) {
        //     ppiVar1->top = (int)ROUND(screenY_);
        //     screenY_ = vp->y;
        //     ppiVar1 = spriter::_instancePtr->UBclipRect;
        // }
        // if ((float)ppiVar1->bottom < screenY_) {
        //     ppiVar1->bottom = (int)ROUND(screenY_);
        //     screenY_ = vp->y;
        //     screenX_ = vp->x;
        // }
        // else {
        //     screenX_ = vp->x;
        // }
        // z = (float)(viewport->rect).left;
        // if ((ushort)((ushort)(z < screenX_) << 8 | (ushort)(z == screenX_) << 0xe) == 0) {
        //     vp->clipFlags = vp->clipFlags | CLIP_LEFT;
        // }
        // else {
        //     if ((float)(viewport->rect).right <= screenX_) {
        //         vp->clipFlags = vp->clipFlags | CLIP_RIGHT;
        //     }
        // }
        // z = (float)(viewport->rect).top;
        // if ((ushort)((ushort)(z < screenY_) << 8 | (ushort)(z == screenY_) << 0xe) == 0) {
        //     vp->clipFlags = vp->clipFlags | CLIP_TOP;
        // }
        // else {
        //     if ((float)(viewport->rect).bottom <= screenY_) {
        //         vp->clipFlags = vp->clipFlags | CLIP_BOTTOM;
        //     }
        // }
    }
    // spriter::_verticesTransformed = spriter::_verticesTransformed + count;
    // return;
}

void Spriter::RenderMeshParts(Mesh& mesh, Vertices3f& verticesTransformed) {
    // glLoadIdentity();

    // glTranslatef(0, -0.5f, 0);

    // float f = 1.0f / 50.0f;
    // glScalef(f, -f, f);

    // glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);

    // static float angle2  = 0;
    // glRotatef(angle2, 0.0f, 1.0f, 0.0f);
    // angle2 += 0.1f;

    for(auto& part : mesh.parts) {
        switch(part.type) {
        case 0:
        case 1:
            RenderMeshPartColor(part, verticesTransformed);
            break;
        // case 0x02:
        //     RenderMeshPart2(spriter,transformedVertices,(PRIMITIVE_23 *)submesh->data, submesh->primitiveCount, viewport);
        //     break;
        // case 0x03:
        //     RenderMeshPart3(spriter,transformedVertices,(PRIMITIVE_23 *)submesh->data, submesh->primitiveCount, viewport);
        //     break;
        case 4:
        case 5:
            RenderMeshPartTexture(part, verticesTransformed);
            break;
        }
    }
    return;
}

void Spriter::RenderMeshPartColor(MeshPart& part, Vertices3f& verticesTransformed) {
    // if(part.cull) glEnable(GL_CULL_FACE);
    // glBegin(GL_TRIANGLE_FAN);
    // for (auto& prim : part.primitives)
    // {
    //     GLubyte r = (prim.color >> 0) & 0xff;
    //     GLubyte g = (prim.color >> 8) & 0xff;
    //     GLubyte b = (prim.color >> 16) & 0xff;

    //     glColor3ub(r,g,b);

    //     for (u32 i = 0; i < part.numVertices; ++i)
    //     {
    //         u32 index = prim.indices[i];
    //         Vertex& v = vertices[index];
    //         glVertex3f(v.x, v.y, v.z);
    //     }
    // }
    // glEnd();
    // glDisable(GL_CULL_FACE);
}

void Spriter::RenderMeshPartTexture(MeshPart& part, Vertices3f& verticesTransformed) {
    // if (!part.cull) glEnable(GL_CULL_FACE);
    // glEnable(GL_TEXTURE_2D);
    // glColor3f(1.0f, 1.0f, 1.0f);

    // for (auto& prim : part.primitives)
    // {
    //     glBindTexture(GL_TEXTURE_2D, _vmcTextures[prim.texture]);

    //     glBegin(GL_TRIANGLE_FAN);
    //     for (u32 i = 0; i < part.numVertices; ++i)
    //     {
    //         u32 index = prim.indices[i];
    //         Vertex& v = vertices[index];
    //         glTexCoord2f(prim.uv[i].x / 255.0f, prim.uv[i].y / 255.0f);
    //         glVertex3f(v.x, v.y, v.z);
    //     }
    //     glEnd();
    // }
    // glDisable(GL_TEXTURE_2D);
    // glDisable(GL_CULL_FACE);
}

void Spriter::LoadModel(const char* modelName, const char* textureName) {
    LoadH(modelName);
    LoadGBL(modelName);
    LoadRBH(modelName, _modelMain.rbh);

    InitModel(_modelMain, _meshMain, _animMain);
}

} // End of namespace Tinsel
