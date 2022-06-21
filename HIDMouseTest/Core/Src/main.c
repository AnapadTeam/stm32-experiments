/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "touchsensing.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_hid.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TSC_HandleTypeDef htsc;

/* USER CODE BEGIN PV */

extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct {
    uint8_t button;
    uint8_t x;
    uint8_t y;
    uint8_t wheel;
    uint8_t unused_2;
    uint8_t unused_3;
    uint8_t unused_4;
    uint8_t unused_5;
} hid_mouse_report;

hid_mouse_report i_hid_mouse_report = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TSC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint8_t mouse_mode = 3;

static void update_mouse_mode() {
    switch (mouse_mode) {
        case 0:
            mouse_mode = 1;
            HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD5_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD6_Pin, GPIO_PIN_SET);
            break;
        case 1:
            mouse_mode = 2;
            HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD5_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, LD6_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            mouse_mode = 3;
            HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD5_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD6_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            mouse_mode = 0;
            HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, LD5_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LD6_Pin, GPIO_PIN_RESET);
            break;
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_TSC_Init();
    MX_TOUCHSENSING_Init();
    /* USER CODE BEGIN 2 */

    update_mouse_mode();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        extern const TSL_LinRot_T MyLinRots[];

        uint8_t changed = 0;
        static uint8_t user_button_pressed = 0;
        static uint8_t previousTSLStatus = 0;

        tsl_user_status_t status = TSL_USER_STATUS_BUSY;
        status = tsl_user_Exec();
        if (status == TSL_USER_STATUS_BUSY) {
            // TSC is acquiring sensor data
            HAL_Delay(1);
        } else {
            if (MyLinRots[0].p_Data->StateId == TSL_STATEID_DETECT) {
                uint8_t position = MyLinRots[0].p_Data->Position;
                int8_t direction = position > 64 ? 1 : -1;
                if (mouse_mode == 0) {
                    i_hid_mouse_report.x = direction * 10;
                    changed = 1;
                } else if (mouse_mode == 1) {
                    i_hid_mouse_report.y = direction * 10;
                    changed = 1;
                } else if (mouse_mode == 2) {
                    if (direction == -1) {
                        changed = i_hid_mouse_report.button != 0x01;
                        i_hid_mouse_report.button = 0x01;
                    } else {
                        changed = i_hid_mouse_report.button != 0x02;
                        i_hid_mouse_report.button = 0x02;
                    }
                } else if (mouse_mode == 3) {
                    i_hid_mouse_report.wheel = direction;
                    changed = 1;
                }
                previousTSLStatus = 0;
            } else if (previousTSLStatus != 1) {
                i_hid_mouse_report.button = 0;
                i_hid_mouse_report.x = 0;
                i_hid_mouse_report.y = 0;
                i_hid_mouse_report.wheel = 0;
                changed = 1;
                previousTSLStatus = 1;
            }
        }

        if (HAL_GPIO_ReadPin(GPIOA, B1_Pin) == GPIO_PIN_SET) {
            user_button_pressed = 1;
        } else {
            if (user_button_pressed) {
                update_mouse_mode();
            }
            user_button_pressed = 0;
        }

        if (changed) {
            USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&i_hid_mouse_report, sizeof(i_hid_mouse_report));
            changed = 0;
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief TSC Initialization Function
 * @param None
 * @retval None
 */
static void MX_TSC_Init(void) {

    /* USER CODE BEGIN TSC_Init 0 */

    /* USER CODE END TSC_Init 0 */

    /* USER CODE BEGIN TSC_Init 1 */

    /* USER CODE END TSC_Init 1 */

    /** Configure the TSC peripheral
     */
    htsc.Instance = TSC;
    htsc.Init.CTPulseHighLength = TSC_CTPH_2CYCLES;
    htsc.Init.CTPulseLowLength = TSC_CTPL_2CYCLES;
    htsc.Init.SpreadSpectrum = DISABLE;
    htsc.Init.SpreadSpectrumDeviation = 1;
    htsc.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
    htsc.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV4;
    htsc.Init.MaxCountValue = TSC_MCV_8191;
    htsc.Init.IODefaultMode = TSC_IODEF_OUT_PP_LOW;
    htsc.Init.SynchroPinPolarity = TSC_SYNC_POLARITY_FALLING;
    htsc.Init.AcquisitionMode = TSC_ACQ_MODE_NORMAL;
    htsc.Init.MaxCountInterrupt = DISABLE;
    htsc.Init.ChannelIOs = TSC_GROUP1_IO3 | TSC_GROUP2_IO3 | TSC_GROUP3_IO2;
    htsc.Init.ShieldIOs = 0;
    htsc.Init.SamplingIOs = TSC_GROUP1_IO4 | TSC_GROUP2_IO4 | TSC_GROUP3_IO3;
    if (HAL_TSC_Init(&htsc) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TSC_Init 2 */

    /* USER CODE END TSC_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin | EXT_RESET_Pin | LD3_Pin | LD6_Pin | LD4_Pin | LD5_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : NCS_MEMS_SPI_Pin EXT_RESET_Pin LD3_Pin LD6_Pin
                             LD4_Pin LD5_Pin */
    GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin | EXT_RESET_Pin | LD3_Pin | LD6_Pin | LD4_Pin | LD5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : MEMS_INT1_Pin MEMS_INT2_Pin */
    GPIO_InitStruct.Pin = MEMS_INT1_Pin | MEMS_INT2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : B1_Pin */
    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : I2C2_SCL_Pin I2C2_SDA_Pin */
    GPIO_InitStruct.Pin = I2C2_SCL_Pin | I2C2_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : SPI2_SCK_Pin SPI2_MISO_Pin SPI2_MOSI_Pin */
    GPIO_InitStruct.Pin = SPI2_SCK_Pin | SPI2_MISO_Pin | SPI2_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
