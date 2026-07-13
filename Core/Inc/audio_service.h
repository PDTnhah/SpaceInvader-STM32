#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

void AudioService_Init(void);
void AudioService_Start(void);
void AudioService_PlayLaser(void);
void AudioService_PlayHit(void);

#ifdef __cplusplus
}
#endif

#endif
