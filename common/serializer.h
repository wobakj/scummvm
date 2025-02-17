/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_SERIALIZER_H
#define COMMON_SERIALIZER_H

#include "common/stream.h"
#include "common/str.h"

namespace Common {

/**
 * @defgroup common_serializer Serializer
 * @ingroup common
 *
 * @brief API for serializing data.
 *
 * @{
 */

#define VER(x) Common::Serializer::Version(x)

#define SYNC_AS(SUFFIX,TYPE,SIZE) \
	template<typename T> \
	void syncAs ## SUFFIX(T &val, Version minVersion = 0, Version maxVersion = kLastVersion) { \
		if (_version < minVersion || _version > maxVersion) \
			return; \
		if (_loadStream) \
			val = static_cast<T>(_loadStream->read ## SUFFIX()); \
		else { \
			TYPE tmp = val; \
			_saveStream->write ## SUFFIX(tmp); \
		} \
		_bytesSynced += SIZE; \
	}

#define SYNC_PRIMITIVE(suffix) \
	template <typename T> \
	static inline void suffix(Serializer &s, T &value) { \
		s.syncAs##suffix(value); \
	}

/**
 * This class allows syncing / serializing data (primarily game savestates)
 * between memory and Read/WriteStreams.
 * It optionally supports versioning the serialized data (client code must
 * use the syncVersion() method for this). This makes it possible to support
 * multiple versions of a savegame format with a single codepath
 *
 * This class was heavily inspired by the save/load code in the SCUMM engine.
 *
 * @todo Maybe rename this to Synchronizer?
 *
 * @todo One feature the SCUMM code has but that is missing here: Support for
 *       syncing arrays of a given type and *fixed* size; and also support
 *       for when the array size changed between versions. Also, support for
 *       2D-arrays.
 *
 * @todo Proper error handling!
 */
class Serializer {
public:
	typedef uint32 Version;
	static const Version kLastVersion = 0xFFFFFFFF;

	SYNC_PRIMITIVE(Uint32LE)
	SYNC_PRIMITIVE(Uint32BE)
	SYNC_PRIMITIVE(Sint32LE)
	SYNC_PRIMITIVE(Sint32BE)
	SYNC_PRIMITIVE(FloatLE)
	SYNC_PRIMITIVE(FloatBE)
	SYNC_PRIMITIVE(DoubleLE)
	SYNC_PRIMITIVE(DoubleBE)
	SYNC_PRIMITIVE(Uint16LE)
	SYNC_PRIMITIVE(Uint16BE)
	SYNC_PRIMITIVE(Sint16LE)
	SYNC_PRIMITIVE(Sint16BE)
	SYNC_PRIMITIVE(Byte)
	SYNC_PRIMITIVE(SByte)

protected:
	SeekableReadStream *_loadStream;
	WriteStream *_saveStream;

	uint _bytesSynced;

	Version _version;

public:
	Serializer(SeekableReadStream *in, WriteStream *out)
		: _loadStream(in), _saveStream(out), _bytesSynced(0), _version(0) {
		assert(in || out);
	}
	virtual ~Serializer() {}

	inline bool isSaving() { return (_saveStream != 0); }
	inline bool isLoading() { return (_loadStream != 0); }

	// WORKAROUND for bugs #4698 "BeOS: tinsel does not compile" and
	// #4697 "BeOS: Cruise does not compile". gcc 2.95.3, which is used
	// for BeOS fails due to an internal compiler error, when we place the
	// following function definitions in another place. Before this work-
	// around the following SYNC_AS definitions were placed at the end
	// of the class declaration. This caused an internal compiler error
	// in the line "syncAsUint32LE(_version);" of
	// "bool syncVersion(Version currentVersion)".
	SYNC_AS(Byte, byte, 1)
	SYNC_AS(SByte, int8, 1)

	SYNC_AS(Uint16LE, uint16, 2)
	SYNC_AS(Uint16BE, uint16, 2)
	SYNC_AS(Sint16LE, int16, 2)
	SYNC_AS(Sint16BE, int16, 2)

	SYNC_AS(Uint32LE, uint32, 4)
	SYNC_AS(Uint32BE, uint32, 4)
	SYNC_AS(Sint32LE, int32, 4)
	SYNC_AS(Sint32BE, int32, 4)
	SYNC_AS(FloatLE, float, 4)
	SYNC_AS(FloatBE, float, 4)
	SYNC_AS(DoubleLE, double, 4)
	SYNC_AS(DoubleBE, double, 4)

	/**
	 * Returns true if an I/O failure occurred.
	 * This flag is never cleared automatically. In order to clear it,
	 * client code has to call clearErr() explicitly.
	 */
	bool err() const {
		if (_saveStream)
			return _saveStream->err();
		else
			return _loadStream->err();
	}

	/**
	 * Reset the I/O error status as returned by err().
	 */
	void clearErr() {
		if (_saveStream)
			_saveStream->clearErr();
		else
			_loadStream->clearErr();
	}

	/**
	 * Sync the "version" of the savegame we are loading/creating.
	 * @param currentVersion	current format version, used when writing a new file
	 * @return true if the version of the savestate is not too new.
	 */
	bool syncVersion(Version currentVersion) {
		_version = currentVersion;
		syncAsUint32LE(_version);
		return _version <= currentVersion;
	}

	/**
	 * Return the version of the savestate being serialized. Useful if the engine
	 * needs to perform additional adjustments when loading old savestates.
	 */
	Version getVersion() const { return _version; }

	/**
	 * Manually set the version
	 */
	void setVersion(Version version) { _version = version; }

	/**
	 * Return the total number of bytes synced so far.
	 */
	uint bytesSynced() const { return _bytesSynced; }

	/**
	 * Skip a number of bytes in the data stream.
	 * This is useful to skip obsolete fields in old savestates.
	 */
	void skip(uint32 size, Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return; // Ignore anything which is not supposed to be present in this save game version

		_bytesSynced += size;
		if (isLoading())
			_loadStream->skip(size);
		else {
			while (size--)
				_saveStream->writeByte(0);
		}
	}

	/**
	 * Sync a block of arbitrary fixed-length data.
	 */
	void syncBytes(byte *buf, uint32 size, Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return; // Ignore anything which is not supposed to be present in this save game version

		if (isLoading())
			_loadStream->read(buf, size);
		else
			_saveStream->write(buf, size);
		_bytesSynced += size;
	}

	/**
	 * Sync a 'magic id' of up to 256 bytes, and return whether it matched.
	 * When saving, this will simply write out the magic id and return true.
	 * When loading, this will read the specified number of bytes, compare it
	 * to the given magic id and return true on a match, false otherwise.
	 *
	 * A typical magic id is a FOURCC like 'MAGI'.
	 *
	 * @param magic		magic id as a byte sequence
	 * @param size		length of the magic id in bytes
	 * @return true if the magic id matched, false otherwise
	 */
	bool matchBytes(const char *magic, byte size, Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return true; // Ignore anything which is not supposed to be present in this save game version

		bool match;
		if (isSaving()) {
			_saveStream->write(magic, size);
			match = true;
		} else {
			char buf[256];
			_loadStream->read(buf, size);
			match = (0 == memcmp(buf, magic, size));
		}
		_bytesSynced += size;
		return match;
	}

	/**
	 * Sync a C-string, by treating it as a zero-terminated byte sequence.
	 * @todo Replace this method with a special Syncer class for Common::String
	 */
	void syncString(String &str, Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return; // Ignore anything which is not supposed to be present in this save game version

		if (isLoading()) {
			char c;
			str.clear();
			while ((c = _loadStream->readByte())) {
				str += c;
				_bytesSynced++;
			}
			_bytesSynced++;
		} else {
			_saveStream->writeString(str);
			_saveStream->writeByte(0);
			_bytesSynced += str.size() + 1;
		}
	}

	/**
	 * Sync a U32-string
	 */
	void syncString32(U32String &str, Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return; // Ignore anything which is not supposed to be present in this save game version

		uint32 len = str.size();

		syncAsUint32LE(len);

		if (isLoading()) {
			U32String::value_type *sl = new U32String::value_type[len];
			for (uint i = 0; i < len; i++)
				syncAsUint32LE(sl[i]);
			str = U32String(sl, len);
		} else {
			for (uint i = 0; i < len; i++)
				_saveStream->writeUint32LE(str[i]);
			_bytesSynced += 4 * len;
		}
	}

	template <typename T>
	void syncArray(T *arr, size_t entries, void (*serializer)(Serializer &, T &), Version minVersion = 0, Version maxVersion = kLastVersion) {
		if (_version < minVersion || _version > maxVersion)
			return;

		for (size_t i = 0; i < entries; ++i) {
			serializer(*this, arr[i]);
		}
	}
};

#undef SYNC_PRIMITIVE
#undef SYNC_AS


// Mixin class / interface
// TODO: Maybe rename this to Syncable ?
class Serializable {
public:
	virtual ~Serializable() {}

	// Maybe rename this method to "syncWithSerializer" or "syncUsingSerializer" ?
	virtual void saveLoadWithSerializer(Serializer &ser) = 0;
};

/** @} */

} // End of namespace Common

#endif
