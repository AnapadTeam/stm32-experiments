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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "get_set_bits.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define HAL_timeout_5s 5000
#define I2C_GT9110_address (uint16_t)0x5D

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

USART_HandleTypeDef husart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum usart_line_ending { NONE, LF, CR_LF };

void usart_transmit_string(char *string, enum usart_line_ending line_ending) {
    const char *line_ending_string;
    switch (line_ending) {
        case NONE:
            line_ending_string = "";
            break;
        case LF:
            line_ending_string = "\n";
            break;
        case CR_LF:
            line_ending_string = "\r\n";
            break;
    }

    char formatted_string[strlen((const char *)string) + 3];
    snprintf(formatted_string, sizeof(formatted_string), "%s%s", string, line_ending_string);
    if (HAL_USART_Transmit(&husart1, (uint8_t *)formatted_string, strlen((const char *)formatted_string),
                           HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
}

void i2c_write_register_byte(uint16_t slave_address, uint16_t register_address, uint8_t register_data) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    uint8_t register_byte_write_data[3] = {(register_address >> 8) & 0xFF, register_address & 0xFF, register_data};
    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, register_byte_write_data, sizeof(register_byte_write_data),
                                HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
}

void i2c_write_register_bytes(uint16_t slave_address, uint16_t register_address, uint8_t *register_data,
                              uint16_t register_data_length) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    uint8_t register_address_data[2] = {(register_address >> 8) & 0xFF, register_address & 0xFF};

    uint8_t register_write_data[sizeof(register_address_data) + register_data_length];
    memcpy(register_write_data, register_address_data, sizeof(register_address_data));
    memcpy(register_write_data + sizeof(register_address_data), register_data, register_data_length);

    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, register_write_data, sizeof(register_write_data),
                                HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
}

uint8_t i2c_read_register_byte(uint16_t slave_address, uint16_t register_address) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    uint8_t register_byte_write_data[2] = {(register_address >> 8) & 0xFF, register_address & 0xFF};
    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, register_byte_write_data, sizeof(register_byte_write_data),
                                HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }

    uint16_t slave_read_address = (slave_address << 1) | 0x01;
    uint8_t received_data_byte;
    if (HAL_I2C_Master_Receive(&hi2c1, slave_read_address, &received_data_byte, 1, HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
    return received_data_byte;
}

void i2c_read_register_bytes(uint16_t slave_address, uint16_t register_address, uint8_t *register_data,
                             uint16_t register_data_length) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    uint8_t register_byte_write_data[2] = {(register_address >> 8) & 0xFF, register_address & 0xFF};
    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, register_byte_write_data, sizeof(register_byte_write_data),
                                HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }

    uint16_t slave_read_address = (slave_address << 1) | 0x01;
    if (HAL_I2C_Master_Receive(&hi2c1, slave_read_address, register_data, register_data_length, HAL_timeout_5s) !=
        HAL_OK) {
        Error_Handler();
    }
}

void serialize_touchscreen_coordinate_data(uint8_t number_of_touches, uint8_t *touchscreen_coordinate_data) {
    // Loop through touches and send serialized touch data
    for (uint8_t touch_index = 0; touch_index < 10; touch_index++) {
        uint8_t *touch_coordinate_data = touchscreen_coordinate_data + (touch_index * 8);
        uint8_t id = touch_coordinate_data[0];
        uint16_t x = (touch_coordinate_data[2] << 8) | touch_coordinate_data[1];
        uint16_t y = (touch_coordinate_data[4] << 8) | touch_coordinate_data[3];
        uint16_t size = (touch_coordinate_data[6] << 8) | touch_coordinate_data[5];

        // Zero out touch data when the touch index is greater than the number of touches
        if (touch_index + 1 > number_of_touches) {
            x = 0;
            y = 0;
            size = 0;
        }

        char formatted_touch_data[75];
        snprintf(formatted_touch_data, sizeof(formatted_touch_data), "t,%d,%d,%d,%d,%d", touch_index, id, x, y, size);
        usart_transmit_string(formatted_touch_data, CR_LF);
    }
}

void poll_and_serialize_touchscreen_data() {
    // Send touchscreen configuration (e.g. the X/Y resolution)
    uint8_t touchscreen_resolution_data[4];
    i2c_read_register_bytes(I2C_GT9110_address, 0x8146, touchscreen_resolution_data,
                            sizeof(touchscreen_resolution_data));
    uint16_t x_resolution = (touchscreen_resolution_data[1] << 8) | touchscreen_resolution_data[0];
    uint16_t y_resolution = (touchscreen_resolution_data[3] << 8) | touchscreen_resolution_data[2];
    char formatted_ts_config_data[50];
    snprintf(formatted_ts_config_data, sizeof(formatted_ts_config_data), "c,%d,%d", x_resolution, y_resolution);
    usart_transmit_string(formatted_ts_config_data, CR_LF);

    // Continuously send touchscreen touch data
    uint8_t touchscreen_coordinate_data[8 * 10] = {0}; // 10 8-byte touch data
    uint32_t zero_touches_sample_threshold = 300;
    while (1) {
        // Wait until touchscreen data is ready to be read
        uint8_t buffer_ready = 0;
        uint8_t number_of_touches = 0;
        uint8_t coordinate_status_register = 0;
        uint32_t zero_touches_samples = 0;
        uint8_t zero_touches_sent = 0;
        do {
            coordinate_status_register = i2c_read_register_byte(I2C_GT9110_address, 0x814E);
            buffer_ready = get_bit((uint32_t *)&coordinate_status_register, 7);
            number_of_touches = get_bits((uint32_t *)&coordinate_status_register, 3, 0);

            // Sent zero touches if confident enough
            if (!zero_touches_sent && number_of_touches == 0 &&
                ++zero_touches_samples >= zero_touches_sample_threshold) {
                zero_touches_sent = 1;
                serialize_touchscreen_coordinate_data(number_of_touches, touchscreen_coordinate_data);
            }
        } while (!buffer_ready);

        // Read and send all coordinate data
        i2c_read_register_bytes(I2C_GT9110_address, 0x814F, touchscreen_coordinate_data,
                                sizeof(touchscreen_coordinate_data));
        serialize_touchscreen_coordinate_data(number_of_touches, touchscreen_coordinate_data);

        // Reset buffer status to trigger another touchscreen sample
        i2c_write_register_byte(I2C_GT9110_address, 0x814E, 0);
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
    MX_USART1_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */

    // Print some new lines
    for (int i = 0; i < 3; i++) {
        usart_transmit_string("", CR_LF);
    }

    poll_and_serialize_touchscreen_data();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {

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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x0000020B;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_Init(void) {

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    husart1.Instance = USART1;
    husart1.Init.BaudRate = 921600;
    husart1.Init.WordLength = USART_WORDLENGTH_8B;
    husart1.Init.StopBits = USART_STOPBITS_1;
    husart1.Init.Parity = USART_PARITY_NONE;
    husart1.Init.Mode = USART_MODE_TX_RX;
    husart1.Init.CLKPolarity = USART_POLARITY_LOW;
    husart1.Init.CLKPhase = USART_PHASE_1EDGE;
    husart1.Init.CLKLastBit = USART_LASTBIT_DISABLE;
    if (HAL_USART_Init(&husart1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */
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

    /*Configure GPIO pins : PA2 PA3 PA6 PA7 */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : PB0 PB1 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
    //__disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, LD3_Pin);
        HAL_Delay(250);
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
