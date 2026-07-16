#include "audio_service.h"
#include "main.h"
#include "cmsis_os2.h"

typedef enum
{
    AUDIO_REQUEST_LASER = 0,
    AUDIO_REQUEST_HIT
} AudioRequest;

static osMessageQueueId_t audioQueueHandle;
static osThreadId_t audioThreadHandle;

#define AUDIO_QUEUE_LENGTH  8U

static void AudioTask(void *argument);
static void enqueueRequest(AudioRequest request);

static const osThreadAttr_t audioThreadAttributes = {
    .name       = "AudioTask",
    .priority   = (osPriority_t)osPriorityLow,
    .stack_size = 1024U
};

void AudioService_Init(void)
{
}

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

static void enqueueRequest(AudioRequest request)
{
    if (audioQueueHandle == NULL)
    {
        return;
    }
    (void)osMessageQueuePut(audioQueueHandle, &request, 0U, 0U);
}

static void AudioTask(void *argument)
{
    (void)argument;
    AudioRequest request;

    /* Cấu hình sử dụng chân PC2 */
#define BUZZER_PORT GPIOC
#define BUZZER_PIN  GPIO_PIN_2

    /* 
     * Nếu còi của bạn là loại "Active Low" (tức là cắm điện mức thấp mới kêu, mức cao lại tắt), 
     * hãy đổi BUZZER_ACTIVE_STATE thành GPIO_PIN_RESET và BUZZER_IDLE_STATE thành GPIO_PIN_SET
     */
#define BUZZER_ACTIVE_STATE GPIO_PIN_SET 
#define BUZZER_IDLE_STATE   GPIO_PIN_RESET

    for (;;)
    {
        if (osMessageQueueGet(audioQueueHandle, &request, NULL, osWaitForever) != osOK)
        {
            continue;
        }

        switch (request)
        {
            case AUDIO_REQUEST_LASER:
                for (int i = 0; i < 2; i++)
                {
                    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_ACTIVE_STATE);
                    osDelay(30);
                    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_IDLE_STATE);
                    osDelay(30);
                }
                break;

            case AUDIO_REQUEST_HIT:
                HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_ACTIVE_STATE);
                osDelay(150);
                HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_IDLE_STATE);
                osDelay(50);
                break;

            default:
                break;
        }
    }
}
