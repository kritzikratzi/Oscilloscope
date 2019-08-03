#include "samplerate.h"
#include "OsciResampler.h"

class OsciResamplerData {
public: 

	OsciResamplerData() {
		int err = 0; 
		state = src_new(SRC_SINC_FASTEST, 4, &err); 
	}

	~OsciResamplerData() {
		src_delete(state); 
	}

	SRC_STATE * state; 
	SRC_DATA data; 
};

OsciResampler::OsciResampler() {
	data = make_unique<OsciResamplerData>(); 
}

OsciResampler::~OsciResampler() {

}