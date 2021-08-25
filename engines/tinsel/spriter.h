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

namespace Tinsel {

struct Vertex3i {
    int32 x,y,z;
};

struct Vertex3f {
    float x,y,z;
};

struct Vertex2c {
    int x,y;
    int flags;
};

void InitSpriter(int width, int height);
void SetCamera(short rotX,short rotY,short rotZ,int posX,int posY,int posZ,int cameraAp);


void TransformXYZ(int x, int y, int z, Vertex2c& v);


void LoadModel(const char* modelName, const char* textureName);

} // End of namespace Tinsel

#endif	// TINSEL_SPRITER_H
