#pragma once

#include "Audio.h"
#include <memory>
#include <thread>

class OsciResamplerData;
class OsciResampler {
	OsciResampler(); 
	virtual ~OsciResampler(); 
	int put(float * data, int numChannels);
	int take(MonoSample * destL, MonoSample * destR, MonoSample * destZ);

private:
	std::unique_ptr<OsciResamplerData> data; 
	std::thread resampleThread;
};
