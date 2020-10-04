#include "WickedLasers.h"
#include "globals.h"
#include "ofxNative.h"

#include "LaserdockDeviceManager.h"
#include "LaserdockDevice.h"

typedef bool (LaserdockDevice::* ReadMethodPtr)(uint32_t*);

const uint32_t laser_samples_per_packet = 64;


#define ANI_SPEED 5
#define EPS 1E-6

#pragma mark WickedLasersSettingsView

#pragma mark Osci
WickedLasers::WickedLasers() :bufferSize(512) {
	laserSamples = (LaserdockSample*)calloc(sizeof(LaserdockSample), laser_samples_per_packet);
	connected = false;

	left.loop = false;
	right.loop = false;
	left.play();
	right.play();

	int err = 0;
	resampleState = src_new(SRC_SINC_FASTEST, 2, &err);

	resampleThread = thread([&]() {ofxNative::setThreadName("resampling"); processIncomming(); });
	resampleThread.detach();

	ofAddListener(ofEvents().update, this, &WickedLasers::of_update, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().exit, this, &WickedLasers::of_exit, OF_EVENT_ORDER_BEFORE_APP);
}

WickedLasers::~WickedLasers() {
	disconnect();
	src_delete(resampleState);
	free(laserSamples);
}

//--------------------------------------------------------------
void WickedLasers::addBuffer(float* buffer, int bufferSize, int nChannels, int sampleRate) {
	this->sampleRate = sampleRate;
	int pos = 0;
	const int chunkSize = 256;
	float temp[2 * chunkSize]; // stereo
	while (pos < bufferSize) {
		int len = min(bufferSize - pos, chunkSize);
		AudioAlgo::copy(temp + 0, 2, buffer + nChannels * pos + 0, nChannels, len);
		AudioAlgo::copy(temp + 1, 2, buffer + nChannels * pos + 1, nChannels, len);
		incomming.append(temp, len * 2);

		pos += len;
	}
}

void WickedLasers::die() {
	exiting = true;
	if(resampleThread.joinable()) resampleThread.join();
}

void WickedLasers::processIncomming() {
	while (!exiting) {

		bool didNothing = true;

		if (shouldResetBuffers) {
			shouldResetBuffers = false;
			auto sz = min(left.totalLength, right.totalLength );
			left.peel(sz);
			right.peel(sz);

			sz = incomming.totalLength / 2;
			incomming.peel(2 * sz);
		}

		if (!connected && incomming.totalLength > 0) {
			incomming.clear();
			ofSleepMillis(1);
			continue;
		}

		int n;
		float* buffer = incomming.peekHead(n);

		if (n > 0) {
			didNothing = false;

			visualSampleRate = 60000;
			double ratio = visualSampleRate / (double)sampleRate;
			if (targetBuffer.size() < visualSampleRate) targetBuffer.resize(visualSampleRate, 0);

			auto run_resample = [ratio](float* buffer, int num_frames,
				float* buffer_out, int num_out_frames,
				SRC_STATE* resampleState, SRC_DATA& resampleData) {
				src_set_ratio(resampleState, ratio);
				resampleData.data_in = buffer;
				resampleData.input_frames = num_frames;
				resampleData.data_out = buffer_out;
				resampleData.output_frames = num_out_frames;
				resampleData.src_ratio = ratio;
				resampleData.end_of_input = 0;
				int res = src_process(resampleState, &resampleData);
			};


			run_resample(buffer, n / 2, &targetBuffer.front(), visualSampleRate / 2, resampleState, resampleData);
			left.append(&targetBuffer.front(), resampleData.output_frames_gen, 2);
			right.append(&targetBuffer.front() + 1, resampleData.output_frames_gen, 2);

			incomming.removeHead();
		}

		// copy to laser:
//			if(scale_arg) buff.scale = args::get(scale_arg);
//			if(brightness_arg) buff.brightness = args::get(brightness_arg);
		while (left.totalLength > laser_samples_per_packet) {
			didNothing = false;

			float leftBuff[laser_samples_per_packet];
			float rightBuff[laser_samples_per_packet];

			left.velocity = globals.laserSize;
			right.velocity = globals.laserSize;
			left.playbackIndex = 0;
			right.playbackIndex = 0;
			left.play();
			right.play();

			left.copyTo(leftBuff, 1, laser_samples_per_packet);
			right.copyTo(rightBuff, 1, laser_samples_per_packet);
			left.peel(laser_samples_per_packet);
			right.peel(laser_samples_per_packet);

			lock_guard<mutex> guard(laserDeviceMutex);
			if (connected) {
				laser_fill_samples(laserSamples, laser_samples_per_packet, leftBuff, rightBuff);
				laserDevice->send_samples(laserSamples, laser_samples_per_packet);
			}
		}

		if (didNothing) {
			ofSleepMillis(2);
		}
	}

	disconnect();
}

//--------------------------------------------------------------
void WickedLasers::of_update(ofEventArgs& args) {
	// ui hidden?
	if (!connected) {
		while (left.totalLength >= bufferSize && right.totalLength >= bufferSize) {
			left.peel(bufferSize);
			right.peel(bufferSize);
		}
		return;
	}
}

void WickedLasers::of_exit(ofEventArgs& args) {
	disconnect();
	exiting = true; 
}

void WickedLasers::laser_fill_samples(LaserdockSample* samples, const uint32_t count, float* left, float* right) {

	if (count == 0) return;
	//        rg = 0x4F4F;
 //       b = 0xFF4F;

	for (uint32_t i = 0; i < count; i++) {

		const float limit = 1.0;

		float fx = ofClamp(left[i], -limit, limit);
		float fy = ofClamp(right[i], -limit, limit);

		float sx = ofMap(fy, -1, 1,
			1 - globals.laserKeystoneY,
			1 + globals.laserKeystoneY, true);
		float sy = ofMap(fx, -1, 1,
			1 - globals.laserKeystoneX,
			1 + globals.laserKeystoneX, true);
		fx *= sx;
		fx *= sy;

		fx += globals.laserOffsetX;
		fy += globals.laserOffsetY;


		uint16_t x = float_to_laserdock_xy(ofClamp(fx,-1,1));
		uint16_t y = float_to_laserdock_xy(ofClamp(fy,-1,1));
		float h = globals.hue;
		ofFloatColor color = h == 360 ?
			ofFloatColor(1, 1, 1) : ofFloatColor::fromHsb(h / 360.0, 1, 1);

		uint16_t cr = color.r * globals.laserIntensity * 255;
		uint16_t cg = color.g * globals.laserIntensity * 255;
		uint16_t cb = color.b * globals.laserIntensity * 255;

		uint16_t rg, b;
		rg = (cg << 8) | cr;
		b = 0xFF00 | cb;

		samples[i].x = x;
		samples[i].y = y;
		samples[i].rg = rg;
		samples[i].b = b;
	}
}


void WickedLasers::connect() {
	disconnect();
	lock_guard<mutex> guard(laserDeviceMutex);
	cout << ("Connecting ...") << std::endl;

	laserDevice = shared_ptr<LaserdockDevice>(LaserdockDeviceManager::getInstance().get_next_available_device());

	if (laserDevice) {
		laserDevice->set_dac_rate(60000);
		cout << ("connected with 60kbps") << std::endl;
		shouldResetBuffers = true;
		connected = true;
		globals.laserConnected = true; 
	}
	else {
		ofSystemAlertDialog("no laser device found. on windows you must install wickedlasers LaserOS first");
		connected = false;
		globals.laserConnected = false; 
	}
}

void WickedLasers::disconnect() {
	lock_guard<mutex> guard(laserDeviceMutex);
	if (connected) {
		cout << ("Disconnecting ...") << std::endl;
		connected = false;
		laserDevice = nullptr;
		globals.laserConnected = false; 
	}
	globals.laserConnected = false;
}

bool WickedLasers::isConnected() {
	return connected;
}

void WickedLasers::resetBuffers() {
	shouldResetBuffers = true;
}
