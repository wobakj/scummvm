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
 */

#include "backends/platform/android/audio.h"

#include "audio/mixer_intern.h"

AndroidAudio::AndroidAudio()
	: _stream(nullptr)
{
	init();
}

AndroidAudio::~AndroidAudio(){
	_stream->stop();
	_stream->close();
}


void AndroidAudio::init() {
	oboe::AudioStreamBuilder _streamBuilder;

	_streamBuilder.setDirection(oboe::Direction::Output);
	_streamBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
	_streamBuilder.setUsage(oboe::Usage::Game);

	_streamBuilder.setFormat(oboe::AudioFormat::I16);
	_streamBuilder.setChannelCount(2);

	_streamBuilder.setCallback(this);

	oboe::Result result = _streamBuilder.openStream(&_stream);
	if (result != oboe::Result::OK) {
		LOGE("Error opening audio stream %s", convertToText(result));
		return;
	}

	_mixer = new Audio::MixerImpl(_stream->getSampleRate());
	_mixer->setReady(true);

	result = _stream->start();
	if (result != oboe::Result::OK) {
		LOGE("Error starting audio stream %s", convertToText(result));
		return;
	}
}

void AndroidAudio::suspendAudio()
{
	_mixer->pauseAll(true);
}

int AndroidAudio::resumeAudio()
{
	_mixer->pauseAll(false);
}

oboe::DataCallbackResult AndroidAudio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
	_mixer->mixCallback((byte*)audioData, numFrames * _stream->getBytesPerFrame());
	return oboe::DataCallbackResult::Continue;
}
