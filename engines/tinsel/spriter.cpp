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


#include "tinsel/spriter.h"

#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/file.h"

#include "math/quat.h"

namespace Tinsel {

const Math::Matrix4& Spriter::MatrixCurrent() const {
	return _currentMatrix->top();
}

void Spriter::MatrixReset() {
	_currentMatrix->clear();

	Math::Matrix4 m;
	_currentMatrix->push(m);
}

void Spriter::MatrixPop() {
	_currentMatrix->pop();
}

void Spriter::MatrixPush() {
	_currentMatrix->push(_currentMatrix->top());
}

void Spriter::MatrixTranslate(float x, float y, float z) {
	Math::Vector3d v {x,y,z};
	_currentMatrix->top().translate(v);
}

void Spriter::MatrixRotateX(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::xAxis(angle).toMatrix();
}

void Spriter::MatrixRotateY(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::yAxis(angle).toMatrix();
}

void Spriter::MatrixRotateZ(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::zAxis(angle).toMatrix();
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
	_currentMatrix = &_modelMatrix;
	_meshShadow.resize(50);


	_view.screenRect.left   = 0;
	_view.screenRect.top	= 0;
	_view.screenRect.right  = width;
	_view.screenRect.bottom = height;

	_view.centerX = width / 2;
	_view.centerY = height / 2;

	_view.viewRect.left   = -_view.centerX;
	_view.viewRect.top	= -_view.centerY;
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

void Spriter::TransformSceneXYZ(int x, int y, int z, int& xOut, int& yOut) {
	MatrixReset();
	MatrixRotateX(_view.cameraRotX);
	MatrixRotateY(_view.cameraRotY);
	MatrixRotateZ(_view.cameraRotZ);
	MatrixTranslate(-_view.cameraPosX, -_view.cameraPosY, -_view.cameraPosZ);
	MatrixTranslate(x / 100.0f, y / 100.0f, z / 100.0f);

	Math::Vector3d v(0,0,0);

	MatrixCurrent().transform(&v, true);

	// apply viewport
	xOut = _view.centerX + v.x();
	yOut = _view.centerY + v.y();
}

void Spriter::LoadH(const Common::String& modelName) {
	Common::String filename = modelName + ".h";

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

void Spriter::LoadGBL(const Common::String& modelName) {
	Common::String filename = modelName + ".gbl";

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

void Spriter::LoadRBH(const Common::String& modelName, RBH& rbh) {
	Common::String filename = modelName + ".rbh";

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
			//	 uint offset = f.readUint32LE();
			//	 --entries;
			// }
			f.skip(size - 8);
			rbh[srcIdx].mappingIdx.push_back(dstIdx);
		} else {
			assert(false);
		}
	}
}

void Spriter::LoadVMC(const Common::String& textureName) {
	Common::String filename = textureName + ".vmc";

	Common::File f;
	f.open(filename);

	int16 buffer[4];
	buffer[0] = f.readSint16LE();
	buffer[1] = f.readSint16LE();
	buffer[2] = f.readSint16LE();
	buffer[3] = f.readSint16LE();

	uint texCount = 4;

	Common::Array<uint8> vmc(texCount * 65536);

	while (true) {
		if ((buffer[3] | buffer[0] | buffer[2] | buffer[1]) == 0) break;

		int a = buffer[1];
		int size1 = buffer[2];
		int b = buffer[0];
		int size2 = buffer[3];

		int size = size2 * size1 * 2;

		uint texId = ((a >> 8) & 0xfffU) * 16 + ((b >> 7) & 0xffffU) & 0xffff;
		if (texId > 3) {
			return;
		}
		uint aAdj = a & 0xff;
		if (a < 0) {
			aAdj = -(-a & 0xffU);
		}
		uint bAdj = b & 0x7f;
		if (b < 0) {
			bAdj = -(-b & 0x7fU);
		}

		uint pos = (((aAdj & 0x1ff) * 128 + (bAdj & 0xffff) & 0xffff) * 2) + (texId * 65536);

		f.read(vmc.data() + pos, size);

		buffer[0] = f.readSint16LE();
		buffer[1] = f.readSint16LE();
		buffer[2] = f.readSint16LE();
		buffer[3] = f.readSint16LE();
	}


	glGenTextures(4, _texture);
	Common::Array<uint8> tex(256*256*3);
	for (uint i = 0; i < texCount; ++i)	{
		for (uint j = 0; j < 256 * 256; ++j)
		{
			tex[(j * 3) + 0] = vmc[(i * 65536) + j];
			tex[(j * 3) + 1] = vmc[(i * 65536) + j];
			tex[(j * 3) + 2] = vmc[(i * 65536) + j];
		}
		glBindTexture(GL_TEXTURE_2D, _texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data());
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, &vmc[(i * 65536)]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
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

	for (auto& mesh : result.meshes) {
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
			v.readFromStream(&s4);
		}

		uint index5 = s3.readUint32LE();
		uint table5 = rbh[table3].mappingIdx[1];

		mesh.normals.resize(s3.readUint32LE());

		Common::MemoryReadStream s5(rbh[table5].data.data(), rbh[table5].data.size());
		s5.skip(index5);

		for (auto& n : mesh.normals) {
			n.readFromStream(&s5);
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
						prim.uv[i].readFromStream(&s6);
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

VecFTables Spriter::LoadTableVector3f(RBH rbh, uint table, uint offset) {
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

		for (auto& v : frameEntries) {
			v.readFromStream(&s2);
		}
	}

	return result;
}

VecFTables Spriter::LoadTableVector3i(RBH rbh, uint table, uint offset) {
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
			uint32 x = s2.readUint32LE();
			uint32 y = s2.readUint32LE();
			uint32 z = s2.readUint32LE();

#define convert(v) (((v & 0xfff) / 4095.0f) * 360.0f)
			it.set(convert(x), convert(y), convert(z));
#undef convert
		}
	}

	return result;
}

void Spriter::InitModel(Model &model, MeshInfo &meshInfo, Common::Array<AnimationInfo> &animInfos) {
	model.renderProgram           = model.rbh[meshInfo.renderProgramHunk].data.data() + meshInfo.renderProgram;

	AnimationInfo &animInfo = animInfos[0];

	model.tables.meshes           = LoadMeshes(model.rbh, meshInfo.meshTablesHunk, meshInfo.meshTables, 0);
	model.tables.translationTable = LoadTableVector3f(model.rbh, animInfo.translateTablesHunk, animInfo.translateTables).frame[0];
	model.tables.rotationTable    = LoadTableVector3i(model.rbh, animInfo.rotateTablesHunk, animInfo.rotateTables).frame[0];
	model.tables.scaleTable       = LoadTableVector3f(model.rbh, animInfo.scaleTablesHunk, animInfo.scaleTables).frame[0];

	_currentMatrix = &_modelMatrix;

	MatrixReset();

	RunRenderProgram(model, true);
}

void Spriter::RunRenderProgram(Model &model, bool initial) {
	uint8* program = model.renderProgram;
	uint ip = 0;

	Vertices3f vertices;
	vertices.reserve(model.tables.meshes.vertexCount);

	// Vertices3f normals;
	// normals.reserve(model.tables.meshes.normalCount);

	Common::Array<uint16> sameVertices;
	sameVertices.resize(model.tables.meshes.vertexCount);

	bool stop = false;
	do {
		RenderProgramOp opCode = (RenderProgramOp)READ_LE_UINT16(&program[ip]);
		ip += 2;

		switch(opCode) {
			case MATRIX_DUPLICATE: {
				MatrixPush();
				break;
			}
			case MATRIX_REMOVE: {
				MatrixPop();
				break;
			}
			case U3: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;
				// if (print)
				// {
				//	 printf("U3 ");
				//	 printf("mesh entry id: %i\n", entry);
				// }
				// local_14 = spriter_;
				// spriter::renderProgram_FUN_0040ea10(spriter_,(&pSVar5->meshTable->entries)[entry],&spriter_->u2);
				break;
			}
			case TRANSFORM: {
				uint16 entry = READ_LE_UINT16(&program[ip]);

				Mesh& mesh = model.tables.meshes.meshes[entry];
				ip += 2;

				if (initial) {
					FindSimilarVertices(mesh, vertices, sameVertices);
					MergeVertices(mesh, sameVertices);
				} else {
					TransformAndRenderMesh(mesh, vertices);
				}

				break;
			}
			case TRANSLATE_X: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translationTable[entry];
				MatrixTranslate(v.x(), 0, 0);

				break;
			}
			case TRANSLATE_Y: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translationTable[entry];
				MatrixTranslate(0, v.y(), 0);

				break;
			}
			case TRANSLATE_Z: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translationTable[entry];
				MatrixTranslate(0, 0, v.z());

				break;
			}
			case TRANSLATE_XYZ: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translationTable[entry];
				MatrixTranslate(v.x(), v.y(), v.z());

				break;
			}
			case ROTATE_X: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotationTable[entry];
				MatrixRotateX(v.x());

				break;
			}
			case ROTATE_Y: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotationTable[entry];
				MatrixRotateY(v.y());

				break;
			}
			case ROTATE_Z: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotationTable[entry];
				MatrixRotateZ(v.z());

				break;
			}
			case STOP: {
				stop = true;
				break;
			}
			case ROTATE_XYZ: {
				uint entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotationTable[entry];
				MatrixRotateX(v.x());
				MatrixRotateY(v.y());
				MatrixRotateZ(v.z());

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
	const Math::Matrix4 &m = MatrixCurrent();

	uint i_start = verticesTransformed.size();
	for (uint i = 0; i < mesh.vertices.size(); ++i) {
		Math::Vector3d& vIn = mesh.vertices[i];

		Math::Vector3d vOut = vIn;
		m.transform(&vOut, true);
		verticesTransformed.push_back(vOut);

		for (uint j = 0; j < verticesTransformed.size() - 1; ++j) {
			float d = vOut.getDistanceTo(verticesTransformed[j]);
			// if (d < 0.01f) {
			if (d < .1f) { // this is too big maybe?
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
	// transformed vertices from previous meshes might be in the current mesh, hence they need to be transformed manually
	const Math::Matrix4 &m = MatrixCurrent();

	for (auto& vIn : mesh.vertices) {
		Math::Vector3d vOut = vIn;
		m.transform(&vOut, true);
		verticesTransformed.push_back(vOut);
	}
}

void Spriter::RenderMeshParts(Mesh& mesh, Vertices3f& verticesTransformed) {
	for(auto& part : mesh.parts) {
		switch(part.type) {
		case 0:
		case 1:
			RenderMeshPartColor(part, verticesTransformed);
			break;
		// case 0x02:
		//	 RenderMeshPart2(spriter,transformedVertices,(PRIMITIVE_23 *)submesh->data, submesh->primitiveCount, viewport);
		//	 break;
		// case 0x03:
		//	 RenderMeshPart3(spriter,transformedVertices,(PRIMITIVE_23 *)submesh->data, submesh->primitiveCount, viewport);
		//	 break;
		case 4:
		case 5:
			RenderMeshPartTexture(part, verticesTransformed);
			break;
		}
	}
	return;
}

void Spriter::RenderMeshPartColor(MeshPart& part, Vertices3f& verticesTransformed) {
	if(!part.cull) glEnable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_FAN);
	for (auto& prim : part.primitives) {
		GLubyte r = (prim.color >> 0) & 0xff;
		GLubyte g = (prim.color >> 8) & 0xff;
		GLubyte b = (prim.color >> 16) & 0xff;

		glColor3ub(r,g,b);

		for (uint32 i = 0; i < part.numVertices; ++i)
		{
			uint32 index = prim.indices[i];
			Math::Vector3d& v = verticesTransformed[index];
			glVertex3f(v.x(), v.y(), v.z());
		}
	}
	glEnd();
	glDisable(GL_CULL_FACE);
}

void Spriter::RenderMeshPartTexture(MeshPart& part, Vertices3f& verticesTransformed) {
	if (!part.cull) glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	for (auto& prim : part.primitives) {
		glBindTexture(GL_TEXTURE_2D, _texture[prim.texture]);

		glBegin(GL_TRIANGLE_FAN);
		for (uint i = 0; i < part.numVertices; ++i) {
			uint index = prim.indices[i];
			Math::Vector3d& v = verticesTransformed[index];
			glTexCoord2f(prim.uv[i].getX() / 255.0f, prim.uv[i].getY() / 255.0f);
			glVertex3f(v.x(), v.y(), v.z());
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
}

void Spriter::LoadModel(const Common::String &modelName, const Common::String &textureName) {
	LoadH(modelName);
	LoadGBL(modelName);
	LoadRBH(modelName, _modelMain.rbh);
	LoadVMC(modelName);

	InitModel(_modelMain, _meshMain, _animMain);
}

void Spriter::RenderModel(Model &model) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, 1.0, -1.0, 0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glTranslatef(0, 0, -0.5f);


	// glTranslatef(0.5f, 0.75f, -0.05f);

	// glTranslatef(0, 0, -100.0f);

	glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);

	static float angle2  = 0;
	glRotatef(angle2, 0.0f, 1.0f, 0.0f);
	angle2 += 1.f;

	float f = 1.0f / 100.0f;
	glScalef(f, f, f);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	RunRenderProgram(model, false);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


}

} // End of namespace Tinsel
