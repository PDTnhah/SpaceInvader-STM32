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
#define AUDIO_GAIN_SHIFT    1U

static osSemaphoreId_t dmaSemaphore;

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
    /* Init Debug LED (PG13 - Green LED on STM32F429I-DISCO) */
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);

    audioQueueHandle = osMessageQueueNew(AUDIO_QUEUE_LENGTH, sizeof(AudioRequest), NULL);
    if (audioQueueHandle == NULL)
    {
        return;
    }

    dmaSemaphore = osSemaphoreNew(1, 0, NULL);

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

static int16_t txBuffer[3000];

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        osSemaphoreRelease(dmaSemaphore);
    }
}

static void playClip(const int16_t *samples, uint32_t sampleCount)
{
    /* Dừng nếu đang phát dở để tránh xung đột */
    HAL_I2S_DMAStop(&hi2s2);

    /* Xóa sạch cờ Semaphore nếu còn sót lại từ lần trước */
    while(osSemaphoreAcquire(dmaSemaphore, 0) == osOK);

    uint32_t txIndex = 0;

    /* Gửi 64 khung im lặng (128 samples) để đánh thức chip MAX98357A (Wake-up time) */
    for (uint32_t i = 0; i < 64; i++) {
        txBuffer[txIndex++] = 0;
        txBuffer[txIndex++] = 0;
    }

    /* Giới hạn độ dài để không tràn txBuffer (tối đa chứa 1500 frames) */
    if (sampleCount > 1400) {
        sampleCount = 1400;
    }

    /* Mono → Stereo: chuyển toàn bộ file âm thanh vào txBuffer trong 1 lần */
    for (uint32_t i = 0U; i < sampleCount; i++)
    {
        int32_t sample = (int32_t)samples[i] >> AUDIO_GAIN_SHIFT;
        txBuffer[txIndex++] = (int16_t)sample; /* Left  */
        txBuffer[txIndex++] = (int16_t)sample; /* Right */
    }

    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET); // Bật LED báo hiệu đang truyền I2S

    /* Gửi toàn bộ txBuffer bằng DMA trong 1 lệnh duy nhất */
    if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t *)txBuffer, (uint16_t)txIndex) == HAL_OK)
    {
        /* Ngủ (Block thread) chờ DMA báo truyền xong qua Callback */
        osSemaphoreAcquire(dmaSemaphore, osWaitForever);
    }

    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); // Tắt LED
}
