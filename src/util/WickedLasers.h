#pragma once

#include "ofxMightyUI.h"
#include "util/Audio.h"
#include "samplerate.h"
#include "OsciMesh.h"

class CircleBuffer;
class LaserdockDevice;
struct LaserdockSample;


class WickedLasers {
public:
	WickedLasers();
	~WickedLasers();

	void addBuffer(float* buffer, int bufferSize, int nChannels, int sampleRate);

	int bufferSize;
	MonoSample left;
	MonoSample right;

	float lx, ly;
	float s;

	void connect();
	void disconnect();
	bool isConnected();
	void resetBuffers();

private:

	void processIncomming();
	void laser_fill_samples(LaserdockSample* samples, const uint32_t count, float* left, float* right);
	void of_update(ofEventArgs& args);
	void of_exit(ofEventArgs& args);
	ofVec3f last;

	float intensity = 1.0;
	bool invertX = false;
	bool invertY = false;
	bool flipXY = false;

	shared_ptr<LaserdockDevice> laserDevice;
	LaserdockSample* laserSamples; // we leak this
	mutex laserDeviceMutex;

	int visualSampleRate = 60000;
	vector<float> targetBuffer;

	thread resampleThread;
	MonoSample incomming;
	static const int chunkSize = 1024;
	int sampleRate = 44100;

	SRC_STATE* resampleState;
	SRC_DATA resampleData;

	bool connected = false;
	bool shouldResetBuffers = false;
	bool exiting = false; 
};
