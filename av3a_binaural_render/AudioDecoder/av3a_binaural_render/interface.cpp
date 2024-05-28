
#include "interface.h"
#include "stream_renderer.h"
#include <VMF_DTypes.h>
#include <stdio.h>
#include "version.h"

void* CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize)
{
	if (!metadata)
		return 0;

	StreamRenderer *render = new StreamRenderer;
#if defined(__arm__) || defined(__aarch64__)
// 	time_t curTime = time(NULL);
// 	struct tm tm1;
// 	localtime_r(&curTime, &tm1);
// 	char fn[2048] = { 0 };
// 	sprintf(fn, "/data/data/com.arcvide.player.file/cache/av3arender_%04d_%02d_%02d_%02d_%02d_%02d.log", (1900 + tm1.tm_year), (1 + tm1.tm_mon), tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
// 	//	sprintf(fn, "av3adec_%04d_%02d_%02d_%02d_%02d_%02d.log", (1900 + tm1.tm_year), (1 + tm1.tm_mon), tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
// 	render->pLog = freopen(fn, "w+", stdout);
// 	setvbuf(stdout, NULL, _IONBF, 0);
	//	ASLOG_MsgA(3, "avs3_create_decoder %p\n", hAvs3Dec);
#endif
	if (render->CreateRenderer(metadata, sampleRate, blockSize) != 0) {
		delete render;
		return 0;
	}
	return render;
}

//int PutPlanarAudioBuffer(void* render, const float **buffer, int frameNum, int channelNum);

int PutInterleavedAudioBuffer(void* render, const float *buffer, int frameNum, int channelNum)
{
	if (!render)
		return 0;
	return ((StreamRenderer*)render)->PutInterleavedAudioBuffer(buffer, frameNum, channelNum);
}

//int GetBinauralPlanarAudioBuffer(void* render, float **buffer, int frameNum);

int GetBinauralInterleavedAudioBuffer(void* render, float *buffer, int frameNum)
{
	if (!render)
		return 0;
	return ((StreamRenderer*)render)->GetBinauralInterleavedAudioBuffer(buffer, frameNum);
}

int UpdateMetadata(void* render, Avs3MetaData *metadata)
{
	if (!render)
		return 0;
	return ((StreamRenderer*)render)->UpdateMetadata(metadata);
}

int SetListenerPosition(void* render, float *position, float *front, float *up)
{
	if (!render)
		return 0;
	return ((StreamRenderer*)render)->SetListenerPosition(position, front, up);
}

int DestroyRenderer(void* render)
{
	if (!render)
		return 0;
//	printf("DestroyRenderer in %p\n", ((StreamRenderer*)render)->audioContext);
	((StreamRenderer*)render)->DestroyRenderer();
//	printf("\nDestroyRenderer out\n");
	if (((StreamRenderer*)render)->pLog)
		fclose(((StreamRenderer*)render)->pLog);
	delete (StreamRenderer*)render;
//	printf("DestroyRenderer out\n");

	return 0;
}

