/**
 * WavFile.h
 *
 * Marek Bereza 2011
 * 
 * Originally stolen from somewhere.
 * 
 * Version 5
 */
#include <stdio.h>

#include <math.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef round
#define round(x) (x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f))
#endif

#define WAVFILE_READ	1
#define WAVFILE_WRITE	2

class WavFile {
private:
	
	fstream *file;
public:
	int length;	
public:
	
	int getLength() {
		return length/channels;
	}
	float *data;
	int channels;
	int samplerate;
	int bitsPerSample;
	short format;
	int ioMode;
	
	void setFormat(int _channels, int _samplerate, int _bitsPerSample) {
		channels = _channels;
		samplerate = _samplerate;
		bitsPerSample = _bitsPerSample;
	}
	
	WavFile() {
		channels = 1;
		samplerate = 44100;
		bitsPerSample = 16;
		data = NULL;
		file = NULL;
		length = 0;
		ioMode = WAVFILE_READ;
	}
	
	
	~WavFile() {
		close();
		unload();
	}
	
	
	bool open(string path, int RW) {
		close();
		ioMode = RW;
		if(RW==WAVFILE_WRITE) {
			
			
			close();
			file = new fstream(path.c_str(), ios::out | ios::binary);
			
			writeFormat();
			return true;
		} else {
			close();
			file = new fstream(path.c_str(), ios::in | ios::binary);
			readFormat();
			file->seekg(44, ios::beg);
			return true;
			//printf("WavFile: WAVFILE_READ not implemented yet!\n");
			//return false;
		}
	}
	
	
	/**
	 * This interleaves samples for putting into wav files.
	 * Give it 2-dim float array (input) and a 1-dim float array (output)
	 * and a number of samples in each channel and it will fill up the output
	 */
	static void interleaveSamples(float **in, float *out, int length) {
		for(int i = 0; i < length; i++) {
			out[i*2 + 0] = in[0][i];
			out[i*2 + 1] = in[1][i];
		}
		
	}
	
	void writeFormat() {
		cout<<"writeFormat()"<<endl;
		format = 1;
		
		int mySubChunk1Size = 16;
		int myByteRate = samplerate * channels * bitsPerSample/8;
		short myBlockAlign = channels * bitsPerSample/8;
		
		int myChunkSize = 36 + length*bitsPerSample/8;
		int myDataSize = length*bitsPerSample/8;
		// write the wav file per the wav file format
		file->seekp (0, ios::beg); 
		file->write ("RIFF", 4);
		file->write ((char*) &myChunkSize, 4);
		file->write ("WAVE", 4);
		file->write ("fmt ", 4);
		file->write ((char*) &mySubChunk1Size, 4);
		file->write ((char*) &format, 2); // should be 1 for PCM
		file->write ((char*) &channels, 2); // # channels (1 or 2)
		file->write ((char*) &samplerate, 4); // 44100
		file->write ((char*) &myByteRate, 4); //
		file->write ((char*) &myBlockAlign, 2);
		file->write ((char*) &bitsPerSample, 2); //16
		file->write ("data", 4);
		file->write ((char*) &myDataSize, 4);
	}
	
	
	/**
	 * Buffer length is nnumber of samples.
	 */
	bool write(float *samples, const int buffLength) {
		if(file==NULL) return false;
		short * buff = new short[buffLength];
		for(int i = 0; i < buffLength; i++) {
			//hansi: was 32760.f
			buff[i] = round(samples[i]*16380.f);
		}
		file->write ((char*)buff, buffLength*sizeof(short));
		length += buffLength;
		delete buff; 
		return true;
	}
	
	bool write(short *samples, int buffLength) {
		if(file==NULL) return false;
		file->write ((char*)samples, buffLength*sizeof(short));
		length += buffLength;
		return true;
	}
	
	int read(float *samples, const int buffLength) {
		if(file==NULL) {
			return 0;
		}
		
		short * buff = new short[buffLength];
		int itemsRead = file->read((char*)buff, buffLength*sizeof(short)).gcount();
		itemsRead /= sizeof(short);
		for(int i = 0; i < itemsRead; i++) {
			samples[i] = ((float)buff[i])/32767.f;
		}
		
		delete buff; 

		return itemsRead;
	}
	
	
	
	void close() {
		if(file!=NULL) {
			if(ioMode==WAVFILE_WRITE) {
				writeFormat();
			}
			file->close();
			delete file;
			file = NULL;
		}
	}
	
	
#define WRITE_BUFF_SIZE 256
	// write out the wav file
	bool save(string path) {
		
		if(data==NULL) return false;
		
		open(path.c_str(), WAVFILE_WRITE);
		
		short buff[WRITE_BUFF_SIZE];
		int pos = 0;
		while(pos<length) {
			int len = MIN(WRITE_BUFF_SIZE, length-pos);
			for(int i = 0; i < len; i++) {
				buff[i] = round(data[pos]*32760.f);
				pos++;
			}
			file->write ((char*)buff, len*bitsPerSample/8);
		}
		
		close();
		return true;
	}
	
	// this gives you data that you have to free yourself
	float *getData() {
		float *d = data;
		data = NULL;
		return d;
	}
	
	// length is the length in samples, not frames 
	// - must give malloced memory
	void setData(float *_data, int _length) {
		data = _data;
		length = _length;
	}
	
	void readFormat() {
		file->seekp (0, ios::beg); 
		short myFormat,myBlockAlign;		
		int mySubChunk1Size,myByteRate;
		int myChunkSize,myDataSize;
		format = 1;
		file->seekg(4, ios::beg);
		file->read( (char*) &myChunkSize, 4 ); // read the ChunkSize
		file->seekg(16, ios::beg);
		file->read( (char*) &mySubChunk1Size, 4 ); // read the SubChunk1Size
		file->read( (char*) &format, sizeof(short) ); // read the file format.  This should be 1 for PCM
		file->read( (char*) &channels, sizeof(short) ); // read the # of channels (1 or 2)
		file->read( (char*) &samplerate, sizeof(int) ); // read the samplerate
		file->read( (char*) &myByteRate, sizeof(int) ); // read the byterate
		file->read( (char*) &myBlockAlign, sizeof(short) ); // read the blockalign
		file->read( (char*) &bitsPerSample, sizeof(short) ); // read the bitspersample
		
		file->seekg(40, ios::beg);
		file->read( (char*) &length, sizeof(int) ); // read the size of the data
		
		printf("Reading format: l = %d\n", samplerate);
		length /= sizeof(short);
		
		
	}
	/**
	 * Reads an entire file into the data member of this class. It dynamically
	 * allocates the memory, and never bothers to delete it (unless you call this
	 * method again), so take care.
	 */
	bool load(string path) {
		
		printf("Path: %s\n", path.c_str());
		if(file!=NULL) close();
		file = new fstream(path.c_str(), ios::in | ios::binary);
		readFormat();
		if(bitsPerSample!=16 || length<=0 || format!=1) {
			return false;
		}
		
		// read the data chunk
		short *rawData = new short[length];
		file->seekg(44, ios::beg);
		file->read((char*)rawData, length*sizeof(short));
		file->close(); // close the input file
		
		unload();
		// bytes to shorts
		data = new float[length];
		printf("Allocated data\n");
		for(int i = 0; i < length; i++) {
			data[i] = ((float)rawData[i])/32767.f;
		}
		delete rawData;
		
		close();
		
		return true; // this should probably be something more descriptive
	}
	void unload() {
		if(data!=NULL) free(data);
		data = NULL;
	}
};