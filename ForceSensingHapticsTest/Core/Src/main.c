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
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define HAL_timeout_5s 5000
#define I2C_DRV2605_address (uint16_t)0x5A

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

USART_HandleTypeDef husart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum usart_line_ending { NONE, LF, CR_LF };

void usart_transmit_string(char string[], enum usart_line_ending line_ending) {
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

void i2c_write_register_byte(uint16_t slave_address, uint8_t register_address, uint8_t register_data) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    uint8_t register_byte_write_data[2] = {register_address, register_data};
    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, register_byte_write_data, sizeof(register_byte_write_data),
                                HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
}

uint8_t i2c_read_register_byte(uint16_t slave_address, uint8_t register_address) {
    uint16_t slave_write_address = (slave_address << 1) | 0x00;
    if (HAL_I2C_Master_Transmit(&hi2c1, slave_write_address, &register_address, 1, HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }

    uint16_t slave_read_address = (slave_address << 1) | 0x01;
    uint8_t received_data_byte;
    if (HAL_I2C_Master_Receive(&hi2c1, slave_read_address, &received_data_byte, 1, HAL_timeout_5s) != HAL_OK) {
        Error_Handler();
    }
    return received_data_byte;
}

void drv2605_print_status() {
    usart_transmit_string("Reading DRV2605 status register...", CR_LF);
    uint8_t status_register = i2c_read_register_byte(I2C_DRV2605_address, 0x00);

    char formatted_status_register[50];
    sprintf(formatted_status_register, "Status register: %d (0x%.2x)", status_register, status_register);
    usart_transmit_string(formatted_status_register, CR_LF);

    uint8_t device_id = get_bits((uint32_t *)&status_register, 7, 5);
    char formatted_device_id[50];
    sprintf(formatted_device_id, "Device ID: %d (0x%.2x)", device_id, device_id);
    usart_transmit_string(formatted_device_id, CR_LF);
}

void drv2605_calibrate() {
    usart_transmit_string("Calibrating DRV2605...", CR_LF);
    i2c_write_register_byte(I2C_DRV2605_address, 0x01, 0x07); // Put device into calibration mode

    uint8_t feedback_control_register = i2c_read_register_byte(I2C_DRV2605_address, 0x1A);
    set_bit((uint32_t *)&feedback_control_register, 0, 7); // ERM mode
    // set_bits((uint32_t*) &feedback_control_register, 2, 6, 4); // Brake factor
    // set_bits((uint32_t*) &feedback_control_register, 2, 3, 2); // Loop gain
    i2c_write_register_byte(I2C_DRV2605_address, 0x1A, feedback_control_register);

    /*i2c_write_register_byte(I2C_DRV2605_address, 0x16, 211); // Rated voltage of 4.5V (4.5 / 21.33e-3)
    i2c_write_register_byte(I2C_DRV2605_address, 0x17, 211); // Overdrive voltage-clamp of ??? TODO

    uint8_t control_4_register = i2c_read_register_byte(I2C_DRV2605_address, 0x1E);
    set_bits((uint32_t*) &control_4_register, 3, 5, 4); // Auto calibration time
    i2c_write_register_byte(I2C_DRV2605_address, 0x1E, control_4_register);

    uint8_t control_1_register = i2c_read_register_byte(I2C_DRV2605_address, 0x1B);
    set_bits((uint32_t*) &control_1_register, 0x13, 4, 0); // Drive time
    i2c_write_register_byte(I2C_DRV2605_address, 0x1B, control_1_register);*/

    i2c_write_register_byte(I2C_DRV2605_address, 0x0C, 0x01); // GO bit

    // Wait until calibration is done.
    while (i2c_read_register_byte(I2C_DRV2605_address, 0x0C)) {
    }

    uint8_t status_register = i2c_read_register_byte(I2C_DRV2605_address, 0x00);
    int8_t diag_result = get_bit((uint32_t *)&status_register, 3);
    if (diag_result == 0) {
        usart_transmit_string("DRV2605 calibration succeeded!", CR_LF);
    } else {
        usart_transmit_string("DRV2605 calibration failed!", CR_LF);
        Error_Handler();
    }
}

char *map_effect_name(uint8_t effect_number) {
    switch (effect_number) {
        case 1:
            return "1 - Strong Click - 100%";
        case 2:
            return "2 - Strong Click - 60%";
        case 3:
            return "3 - Strong Click - 30%";
        case 4:
            return "4 - Sharp Click - 100%";
        case 5:
            return "5 - Sharp Click - 60%";
        case 6:
            return "6 - Sharp Click - 30%";
        case 7:
            return "7 - Soft Bump - 100%";
        case 8:
            return "8 - Soft Bump - 60%";
        case 9:
            return "9 - Soft Bump - 30%";
        case 10:
            return "10 - Double Click - 100%";
        case 11:
            return "11 - Double Click - 60%";
        case 12:
            return "12 - Triple Click - 100%";
        case 13:
            return "13 - Soft Fuzz - 60%";
        case 14:
            return "14 - Strong Buzz - 100%";
        case 15:
            return "15 - 750 ms Alert 100%";
        case 16:
            return "16 - 1000 ms Alert 100%";
        case 17:
            return "17 - Strong Click 1 - 100%";
        case 18:
            return "18 - Strong Click 2 - 80%";
        case 19:
            return "19 - Strong Click 3 - 60%";
        case 20:
            return "20 - Strong Click 4 - 30%";
        case 21:
            return "21 - Medium Click 1 - 100%";
        case 22:
            return "22 - Medium Click 2 - 80%";
        case 23:
            return "23 - Medium Click 3 - 60%";
        case 24:
            return "24 - Sharp Tick 1 - 100%";
        case 25:
            return "25 - Sharp Tick 2 - 80%";
        case 26:
            return "26 - Sharp Tick 3 - 60%";
        case 27:
            return "27 - Short Double Click Strong 1 - 100%";
        case 28:
            return "28 - Short Double Click Strong 2 - 80%";
        case 29:
            return "29 - Short Double Click Strong 3 - 60%";
        case 30:
            return "30 - Short Double Click Strong 4 - 30%";
        case 31:
            return "31 - Short Double Click Medium 1 - 100%";
        case 32:
            return "32 - Short Double Click Medium 2 - 80%";
        case 33:
            return "33 - Short Double Click Medium 3 - 60%";
        case 34:
            return "34 - Short Double Sharp Tick 1 - 100%";
        case 35:
            return "35 - Short Double Sharp Tick 2 - 80%";
        case 36:
            return "36 - Short Double Sharp Tick 3 - 60%";
        case 37:
            return "37 - Long Double Sharp Click Strong 1 - 100%";
        case 38:
            return "38 - Long Double Sharp Click Strong 2 - 80%";
        case 39:
            return "39 - Long Double Sharp Click Strong 3 - 60%";
        case 40:
            return "40 - Long Double Sharp Click Strong 4 - 30%";
        case 41:
            return "41 - Long Double Sharp Click Medium 1 - 100%";
        case 42:
            return "42 - Long Double Sharp Click Medium 2 - 80%";
        case 43:
            return "43 - Long Double Sharp Click Medium 3 - 60%";
        case 44:
            return "44 - Long Double Sharp Tick 1 - 100%";
        case 45:
            return "45 - Long Double Sharp Tick 2 - 80%";
        case 46:
            return "46 - Long Double Sharp Tick 3 - 60%";
        case 47:
            return "47 - Buzz 1 - 100%";
        case 48:
            return "48 - Buzz 2 - 80%";
        case 49:
            return "49 - Buzz 3 - 60%";
        case 50:
            return "50 - Buzz 4 - 40%";
        case 51:
            return "51 - Buzz 5 - 20%";
        case 52:
            return "52 - Pulsing Strong 1 - 100%";
        case 53:
            return "53 - Pulsing Strong 2 - 60%";
        case 54:
            return "54 - Pulsing Medium 1 - 100%";
        case 55:
            return "55 - Pulsing Medium 2 - 60%";
        case 56:
            return "56 - Pulsing Sharp 1 - 100%";
        case 57:
            return "57 - Pulsing Sharp 2 - 60%";
        case 58:
            return "58 - Transition Click 1 - 100%";
        case 59:
            return "59 - Transition Click 2 - 80%";
        case 60:
            return "60 - Transition Click 3 - 60%";
        case 61:
            return "61 - Transition Click 4 - 40%";
        case 62:
            return "62 - Transition Click 5 - 20%";
        case 63:
            return "63 - Transition Click 6 - 10%";
        case 64:
            return "64 - Transition Hum 1 - 100%";
        case 65:
            return "65 - Transition Hum 2 - 80%";
        case 66:
            return "66 - Transition Hum 3 - 60%";
        case 67:
            return "67 - Transition Hum 4 - 40%";
        case 68:
            return "68 - Transition Hum 5 - 20%";
        case 69:
            return "69 - Transition Hum 6 - 10%";
        case 70:
            return "70 - Transition Ramp Down Long Smooth 1 - 100 to 0%";
        case 71:
            return "71 - Transition Ramp Down Long Smooth 2 - 100 to 0%";
        case 72:
            return "72 - Transition Ramp Down Medium Smooth 1 - 100 to 0%";
        case 73:
            return "73 - Transition Ramp Down Medium Smooth 2 - 100 to 0%";
        case 74:
            return "74 - Transition Ramp Down Short Smooth 1 - 100 to 0%";
        case 75:
            return "75 - Transition Ramp Down Short Smooth 2 - 100 to 0%";
        case 76:
            return "76 - Transition Ramp Down Long Sharp 1 - 100 to 0%";
        case 77:
            return "77 - Transition Ramp Down Long Sharp 2 - 100 to 0%";
        case 78:
            return "78 - Transition Ramp Down Medium Sharp 1 - 100 to 0%";
        case 79:
            return "79 - Transition Ramp Down Medium Sharp 2 - 100 to 0%";
        case 80:
            return "80 - Transition Ramp Down Short Sharp 1 - 100 to 0%";
        case 81:
            return "81 - Transition Ramp Down Short Sharp 2 - 100 to 0%";
        case 82:
            return "82 - Transition Ramp Up Long Smooth 1 - 0 to 100%";
        case 83:
            return "83 - Transition Ramp Up Long Smooth 2 - 0 to 100%";
        case 84:
            return "84 - Transition Ramp Up Medium Smooth 1 - 0 to 100%";
        case 85:
            return "85 - Transition Ramp Up Medium Smooth 2 - 0 to 100%";
        case 86:
            return "86 - Transition Ramp Up Short Smooth 1 - 0 to 100%";
        case 87:
            return "87 - Transition Ramp Up Short Smooth 2 - 0 to 100%";
        case 88:
            return "88 - Transition Ramp Up Long Sharp 1 - 0 to 100%";
        case 89:
            return "89 - Transition Ramp Up Long Sharp 2 - 0 to 100%";
        case 90:
            return "90 - Transition Ramp Up Medium Sharp 1 - 0 to 100%";
        case 91:
            return "91 - Transition Ramp Up Medium Sharp 2 - 0 to 100%";
        case 92:
            return "92 - Transition Ramp Up Short Sharp 1 - 0 to 100%";
        case 93:
            return "93 - Transition Ramp Up Short Sharp 2 - 0 to 100%";
        case 94:
            return "94 - Transition Ramp Down Long Smooth 1 - 50 to 0%";
        case 95:
            return "95 - Transition Ramp Down Long Smooth 2 - 50 to 0%";
        case 96:
            return "96 - Transition Ramp Down Medium Smooth 1 - 50 to 0%";
        case 97:
            return "97 - Transition Ramp Down Medium Smooth 2 - 50 to 0%";
        case 98:
            return "98 - Transition Ramp Down Short Smooth 1 - 50 to 0%";
        case 99:
            return "99 - Transition Ramp Down Short Smooth 2 - 50 to 0%";
        case 100:
            return "100 - Transition Ramp Down Long Sharp 1 - 50 to 0%";
        case 101:
            return "101 - Transition Ramp Down Long Sharp 2 - 50 to 0%";
        case 102:
            return "102 - Transition Ramp Down Medium Sharp 1 - 50 to 0%";
        case 103:
            return "103 - Transition Ramp Down Medium Sharp 2 - 50 to 0%";
        case 104:
            return "104 - Transition Ramp Down Short Sharp 1 - 50 to 0%";
        case 105:
            return "105 - Transition Ramp Down Short Sharp 2 - 50 to 0%";
        case 106:
            return "106 - Transition Ramp Up Long Smooth 1 - 0 to 50%";
        case 107:
            return "107 - Transition Ramp Up Long Smooth 2 - 0 to 50%";
        case 108:
            return "108 - Transition Ramp Up Medium Smooth 1 - 0 to 50%";
        case 109:
            return "109 - Transition Ramp Up Medium Smooth 2 - 0 to 50%";
        case 110:
            return "110 - Transition Ramp Up Short Smooth 1 - 0 to 50%";
        case 111:
            return "111 - Transition Ramp Up Short Smooth 2 - 0 to 50%";
        case 112:
            return "112 - Transition Ramp Up Long Sharp 1 - 0 to 50%";
        case 113:
            return "113 - Transition Ramp Up Long Sharp 2 - 0 to 50%";
        case 114:
            return "114 - Transition Ramp Up Medium Sharp 1 - 0 to 50%";
        case 115:
            return "115 - Transition Ramp Up Medium Sharp 2 - 0 to 50%";
        case 116:
            return "116 - Transition Ramp Up Short Sharp 1 - 0 to 50%";
        case 117:
            return "117 - Transition Ramp Up Short Sharp 2 - 0 to 50%";
        case 118:
            return "118 - Long buzz for programmatic stopping - 100%";
        case 119:
            return "119 - Smooth Hum 1 (No kick or brake pulse) - 50%";
        case 120:
            return "120 - Smooth Hum 2 (No kick or brake pulse) - 40%";
        case 121:
            return "121 - Smooth Hum 3 (No kick or brake pulse) - 30%";
        case 122:
            return "122 - Smooth Hum 4 (No kick or brake pulse) - 20%";
        case 123:
            return "123 - Smooth Hum 5 (No kick or brake pulse) - 10%";
        default:
            return "Unknown effect";
    }
}

void drv2605_user_button_cycle_effect_library() {
    usart_transmit_string("Configuring DRV2605 with waveform library effects and user button sequencing...", CR_LF);
    i2c_write_register_byte(I2C_DRV2605_address, 0x01, 0x00); // Internal trigger mode
    i2c_write_register_byte(I2C_DRV2605_address, 0x03, 0x02); // Waveform library selection
    // i2c_write_register_byte(I2C_DRV2605_address, 0x16, 234); // Rated voltage of 4.5V (4.5 / 21.33e-3)

    uint8_t current_waveform_effect = 1;
    uint8_t user_button_pressed = 0;
    while (1) {
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0) {
            if (user_button_pressed) {
                user_button_pressed = 0;
            } else {
                HAL_Delay(1);
                continue;
            }
        } else {
            user_button_pressed = 1;
            HAL_Delay(1);
            continue;
        }

        char *effect_waveform_name = map_effect_name(current_waveform_effect);
        char formatted_current_effect_string[50];
        sprintf(formatted_current_effect_string, "Playing effect number: (0x%.2x) %s", current_waveform_effect,
                effect_waveform_name);
        usart_transmit_string(formatted_current_effect_string, CR_LF);

        // Set waveform sequence effect number
        i2c_write_register_byte(I2C_DRV2605_address, 0x04, current_waveform_effect++);
        // Set GO bit
        i2c_write_register_byte(I2C_DRV2605_address, 0x0C, 0x01);
        // Wait for GO bit to clear
        while (i2c_read_register_byte(I2C_DRV2605_address, 0x0C)) {
        }

        if (current_waveform_effect > 123) {
            current_waveform_effect = 1;
        }
    }
}

void load_cell_test() {
    while (1) {
        uint32_t adc_value = HAL_ADC_GetValue(&hadc);
        char formatted_adc_value[50];
        sprintf(formatted_adc_value, "ADC: %d (0x%.8x)", adc_value, adc_value);
        usart_transmit_string(formatted_adc_value, CR_LF);
        HAL_Delay(250);
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
    MX_ADC_Init();
    MX_I2C1_Init();
    MX_USART1_Init();
    /* USER CODE BEGIN 2 */

    for (int i = 0; i < 3; i++) {
        usart_transmit_string("", CR_LF);
    }

    // drv2605_print_status();
    drv2605_user_button_cycle_effect_library();
    // load_cell_test();

    /*uint8_t control_3_register = i2c_read_register_byte(I2C_DRV2605_address, 0x1D);
    set_bit((uint32_t*) &control_3_register, 0, 5); // Closed Loop ERM mode
    i2c_write_register_byte(I2C_DRV2605_address, 0x1D, control_3_register);
    i2c_write_register_byte(I2C_DRV2605_address, 0x16, 0xFF); // Rated voltage of 4.5V (4.5 / 21.33e-3)
    i2c_write_register_byte(I2C_DRV2605_address, 0x17, 0xFF); // Overdrive voltage-clamp
    i2c_write_register_byte(I2C_DRV2605_address, 0x01, 0x05);
    i2c_write_register_byte(I2C_DRV2605_address, 0x02, 127);*/

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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI14 | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI14CalibrationValue = 16;
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
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC_Init(void) {

    /* USER CODE BEGIN ADC_Init 0 */

    /* USER CODE END ADC_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC_Init 1 */

    /* USER CODE END ADC_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&hadc) != HAL_OK) {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel to be converted.
     */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC_Init 2 */

    /* USER CODE END ADC_Init 2 */
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
    husart1.Init.BaudRate = 115200;
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
