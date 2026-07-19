# Space Invaders - STM32F429I-DISCO

An embedded implementation of the classic Space Invaders game on the STM32F429I-DISCO board, utilizing TouchGFX for the UI and graphics rendering.

## Hardware Bill of Materials (BOM)

| Component | Quantity | Role | Key Specifications |
| --- | --- | --- | --- |
| *STM32F429I-DISCO* | 1 | Main Development Board | Core processing, display, and memory. |
| *STM32F429ZIT6 Microcontroller* | 1 (On-board) | Main MCU | ARM Cortex-M4, 180 MHz, 2MB Flash, 256KB RAM. |
| *2.4" QVGA TFT LCD* | 1 (On-board) | Display Screen | 320x240 pixels, 16bpp, with ILI9341 controller and resistive touch. |
| *64-Mbit SDRAM* | 1 (On-board) | Framebuffer/Asset Storage | External memory for TouchGFX framebuffers. |
| *L3GD20 ST MEMS* | 1 (On-board) | Motion Sensor | 3-axis digital output gyroscope. |
| *User Push Button* | 1 (On-board) | Input Control | Used for game interaction (e.g., shooting/starting). |
| *LEDs (LD3, LD4)* | 2 (On-board) | Status Indication | Visual feedback for game states or debugging. |
| *USB Mini-B Cable* | 1 | Power & Programming | Used to connect ST-LINK/V2 to PC for flashing/debugging. |

## Development Environment & Flashing

The default IDE is set to STM32CubeIDE. To change the IDE, open the STM32F429I_DISCO_REV_D01.ioc with STM32CubeMX and select from the supported IDEs (EWARM from version 8.50.9, MDK-ARM, and STM32CubeIDE).

Supports flashing of the STM32F429I_DISCO_DEV_D01 board directly from TouchGFX Designer using GCC and STM32CubeProgrammer. Flashing the board requires STM32CubeProgrammer which can be downloaded from the ST webpage. 

This TBS is configured for 320 x 240 pixels 16bpp screen resolution.

## Performance Testing
Performance testing can be done using the GPIO pins designated with the following signals: 
- VSYNC_FREQ  - Pin PE2
- RENDER_TIME - Pin PE3
- FRAME_RATE  - Pin PE4
- MCU_ACTIVE  - Pin PE5
Soạn
Viết cho Phạm Thanh
