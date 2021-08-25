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
 * Starts up new scenes.
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
#include "common/stack.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace Tinsel {

struct Matrix {
	float m[3][3];
    float t[3];
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

//----------------- EXTERNAL FUNCTIONS ---------------------

// // in EFFECT.C
// extern void EffectPolyProcess(CORO_PARAM, const void *);

// // in PDISPLAY.C
// #ifdef DEBUG
// extern void CursorPositionProcess(CORO_PARAM, const void *);
// #endif
// extern void TagProcess(CORO_PARAM, const void *);
// extern void PointProcess(CORO_PARAM, const void *);
// extern void EnableTags();


//----------------- LOCAL DEFINES --------------------

// #include "common/pack-start.h"	// START STRUCT PACKING

// #include "common/pack-end.h"	// END STRUCT PACKING


//----------------- LOCAL GLOBAL DATA --------------------

// #ifdef DEBUG
// static bool g_ShowPosition = false;	// Set when showpos() has been called
// #endif

// int g_sceneCtr = 0;
// static int g_initialMyEscape = 0;

// static SCNHANDLE g_SceneHandle = 0;	// Current scene handle - stored in case of Save_Scene()

typedef Common::FixedStack<Matrix, 30> MatrixStack;
static MatrixStack g_modelMatrix;
static MatrixStack* g_currentMatrix = &g_modelMatrix;
static View g_view;

void MatrixIdentity();

void MatrixReset() {
    g_currentMatrix->clear();
    MatrixIdentity();
}

void MatrixPop() {
    g_currentMatrix->pop();
}

void MatrixMulVertex(Matrix &m, Vertex3f& v) {
    float x = v.x;
    float y = v.y;
    float z = v.z;

    v.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z;
    v.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z;
    v.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z;
}

void MatrixIdentity() {
    Matrix identity {};
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;

    g_currentMatrix->push(identity);
}

void MatrixTranslate(float x, float y, float z) {
    Vertex3f v {x,y,z};
    Matrix &m = g_currentMatrix->top();
    MatrixMulVertex(m, v);
    m.t[0] += v.x;
    m.t[1] += v.y;
    m.t[2] += v.z;
}

void MatrixRotateX(int angle) {
    Matrix &m = g_currentMatrix->top();

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

void MatrixRotateY(int angle) {
    Matrix &m = g_currentMatrix->top();

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

void MatrixRotateZ(int angle) {
    Matrix &m = g_currentMatrix->top();

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

void SetViewport(int ap) {
    g_view.viewport.ap = ap;

    int ratio = ((g_view.screenRect.right - g_view.screenRect.left) << 12) / ap;
    g_view.viewport.width = ratio;
    g_view.viewport.height = ratio;

    g_view.viewport.ap = (ap + 1800) / 3600;

    g_view.viewport.rect = g_view.viewRect;
}


void InitSpriter(int width, int height) {
    g_view.screenRect.left   = 0;
    g_view.screenRect.top    = 0;
    g_view.screenRect.right  = width;
    g_view.screenRect.bottom = height;

    g_view.centerX = width / 2;
    g_view.centerY = height / 2;

    g_view.viewRect.left   = -g_view.centerX;
    g_view.viewRect.top    = -g_view.centerY;
    g_view.viewRect.right  =  g_view.screenRect.right  - g_view.screenRect.left - g_view.centerX - 1;
    g_view.viewRect.bottom =  g_view.screenRect.bottom - g_view.screenRect.top  - g_view.centerY;

    SetViewport(306030);
}


void SetCamera(short rotX, short rotY, short rotZ, int posX, int posY, int posZ, int cameraAp) {
    g_view.cameraPosX = posX * 0.01f;
    g_view.cameraPosY = posY * 0.01f;
    g_view.cameraPosZ = posZ * 0.01f;
    g_view.cameraRotX = rotX;
    g_view.cameraRotY = rotY;
    g_view.cameraRotZ = rotZ;

    SetViewport(cameraAp);
}

void TransformVertex(Vertex3f* vOut, Vertex3f* vIn, int count) {
    Matrix &m = g_currentMatrix->top();

    float viewportWidth = g_view.viewport.width;
    float viewportHeight = g_view.viewport.height;
    for (int i = 0; i < count; ++i, ++vIn, ++vOut) {
        vOut->x = m.m[0][0] * vIn->x + m.t[0] + m.m[0][1] * vIn->y + m.m[0][2] * vIn->z;
        vOut->y = m.m[1][0] * vIn->x + m.t[1] + m.m[1][1] * vIn->y + m.m[1][2] * vIn->z;
        vOut->z = m.m[2][0] * vIn->x + m.t[2] + m.m[2][1] * vIn->y + m.m[2][2] * vIn->z;

        if (vOut->z <= 1.0f) {
            //vOut->intensityFront = 1.0;
            //vOut->clipFlags = CLIP_FRONT;
            vOut->x *= viewportWidth;
            vOut->y *= viewportHeight;
        } else {
            float z = 1.0F / vOut->z;
            //vp->intensityFront = z;
            //vp->clipFlags = NO_CLIP;
            vOut->x *= z * viewportWidth;
            vOut->y *= z * viewportHeight;
        }

        // vOut->x = g_view.centerX + vOut->x;
        // vOut->y = g_view.centerY + vOut->y;

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

void TransformXYZ(int x, int y, int z, Vertex2c& v) {
    MatrixReset();
    MatrixRotateX(g_view.cameraRotX);
    MatrixRotateY(g_view.cameraRotY);
    MatrixRotateZ(g_view.cameraRotZ);
    MatrixTranslate(-g_view.cameraPosX, -g_view.cameraPosY, -g_view.cameraPosZ);
    MatrixTranslate(x / 100.0f, y / 100.0f, z / 100.0f);

    Vertex3f vIn {}, vOut{};
    TransformVertex(&vOut, &vIn, 1);

    v.x = g_view.centerX + vOut.x;
    v.y = g_view.centerY + vOut.y;

}

void LoadModel(const char* modelName, const char* textureName)
{

}

} // End of namespace Tinsel
