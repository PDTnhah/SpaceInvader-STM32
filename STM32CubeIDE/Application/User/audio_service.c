#include "audio_service.h"

/* Audio samples - same folder as this file */
#include "laserShoot.h"
#include "hitHurt.h"

#include "main.h"
#include "cmsis_os2.h"

/* Dùng handle hi2s2 do CubeMX khởi tạo sẵn trong main.c (MX_I2S2_Init) */
extern I2S_HandleTypeDef hi2s2;

typedef enum
{
    AUDIO_REQUEST_LASER = 0,
    AUDIO_REQUEST_HIT
} AudioRequest;

static osMessageQueueId_t audioQueueHandle;
static osThreadId_t audioThreadHandle;

#define AUDIO_QUEUE_LENGTH  8U
#define AUDIO_BLOCK_FRAMES  128U
#define AUDIO_GAIN_SHIFT    1U

static void AudioTask(void *argument);
static void playClip(const int16_t *samples, uint32_t sampleCount);
static void enqueueRequest(AudioRequest request);

static const osThreadAttr_t audioThreadAttributes = {
    .name       = "AudioTask",
    .priority   = (osPriority_t)osPriorityLow,
    .stack_size = 1024U
};

/* ------------------------------------------------------------
 * AudioService_Init
 * Không cần init I2S nữa — CubeMX đã gọi MX_I2S2_Init() trong main.c
 * Hàm này giữ lại để main.c không cần sửa.
 * ------------------------------------------------------------ */
void AudioService_Init(void)
{
    /* Nothing to do — hi2s2 already initialized by MX_I2S2_Init() */
}

/* ------------------------------------------------------------
 * AudioService_Start
 * Gọi sau khi RTOS scheduler đã chạy (osKernelStart).
 * Tạo queue + thread audio.
 * ------------------------------------------------------------ */
void AudioService_Start(void)
{
    audioQueueHandle = osMessageQueueNew(AUDIO_QUEUE_LENGTH, sizeof(AudioRequest), NULL);
    if (audioQueueHandle == NULL)
    {
        return;
    }

    audioThreadHandle = osThreadNew(AudioTask, NULL, &audioThreadAttributes);
}

void AudioService_PlayLaser(void)
{
    enqueueRequest(AUDIO_REQUEST_LASER);
}

void AudioService_PlayHit(void)
{
    enqueueRequest(AUDIO_REQUEST_HIT);
}

/* ============================================================
 * Internal helpers
 * ============================================================ */
static void enqueueRequest(AudioRequest request)
{
    if (audioQueueHandle == NULL)
    {
        return;
    }
    /* Không block nếu queue đầy — bỏ qua âm thanh mới */
    (void)osMessageQueuePut(audioQueueHandle, &request, 0U, 0U);
}

static void AudioTask(void *argument)
{
    (void)argument;
    AudioRequest request;

    for (;;)
    {
        if (osMessageQueueGet(audioQueueHandle, &request, NULL, osWaitForever) != osOK)
        {
            continue;
        }

        switch (request)
        {
            case AUDIO_REQUEST_LASER:
                playClip(laserShoot, laserShoot_len);
                break;

            case AUDIO_REQUEST_HIT:
                playClip(hitHurt, hitHurt_len);
                break;

            default:
                break;
        }
    }
}

static int16_t txBuffer[AUDIO_BLOCK_FRAMES * 2U];

static void playClip(const int16_t *samples, uint32_t sampleCount)
{
    uint32_t index = 0U;

    /* Dừng nếu đang phát dở để tránh xung đột */
    HAL_I2S_DMAStop(&hi2s2);

    /* Gửi 2 khung im lặng để đánh thức chip MAX98357A (Wake-up time) */
    for (uint32_t i = 0; i < AUDIO_BLOCK_FRAMES * 2U; i++) {
        txBuffer[i] = 0;
    }
    HAL_I2S_Transmit(&hi2s2, (uint16_t *)txBuffer, (uint16_t)(AUDIO_BLOCK_FRAMES * 2U), HAL_MAX_DELAY);
    HAL_I2S_Transmit(&hi2s2, (uint16_t *)txBuffer, (uint16_t)(AUDIO_BLOCK_FRAMES * 2U), HAL_MAX_DELAY);

    while (index < sampleCount)
    {
        uint32_t frameCount = sampleCount - index;
        if (frameCount > AUDIO_BLOCK_FRAMES)
        {
            frameCount = AUDIO_BLOCK_FRAMES;
        }

        /* Mono → Stereo: nhân đôi mỗi sample sang L+R */
        for (uint32_t i = 0U; i < frameCount; i++)
        {
            int32_t sample = (int32_t)samples[index + i] >> AUDIO_GAIN_SHIFT;
            txBuffer[(i * 2U)]      = (int16_t)sample; /* Left  */
            txBuffer[(i * 2U) + 1U] = (int16_t)sample; /* Right */
        }

        /* Blocking transmit — chạy trong thread riêng nên không chặn UI */
        if (HAL_I2S_Transmit(&hi2s2, (uint16_t *)txBuffer,
                              (uint16_t)(frameCount * 2U), HAL_MAX_DELAY) != HAL_OK)
        {
            break;
        }

        index += frameCount;
    }
}
