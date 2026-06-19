/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Than chuong trinh chinh
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * Da dang ky ban quyen.
  *
  * Phan mem nay duoc cap phep theo dieu khoan trong file LICENSE
  * tai thu muc goc cua thanh phan phan mem nay.
  * Neu khong co file LICENSE, phan mem duoc cung cap theo hien trang.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Cac file include ----------------------------------------------------------*/
#include "main.h"

/* Include rieng -------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Rte.h"
#include "system_config.h"
#include "App_AebLogic.h"
#include "HCSR04.h"
#include "uart.h"
#include "Throttle.h"
#include "L298N_Driver.h"
#include "Alerts.h"
/* USER CODE END Includes */

/* Kieu du lieu rieng --------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Dinh nghia rieng ----------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Tac vu cam bien va tac vu ung dung dong bo voi chu ky trigger 20 ms cua HC-SR04. */
#define SENSOR_TASK_PERIOD_MS    20U
#define APP_TASK_PERIOD_MS       20U
#define LOG_TASK_PERIOD_MS       100U

/* USER CODE END PD */

/* Macro rieng ---------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Bien rieng ----------------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint32_t sensorTaskLastTick = 0U;
static uint32_t appTaskLastTick = 0U;
static uint32_t logTaskLastTick = 0U;

/* USER CODE END PV */

/* Khai bao nguyen mau ham rieng ---------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
/*
 * In nhanh du lieu hien tai trong RTE qua UART.
 * Dat trong main de RTE chi luu du lieu, khong phu trach UART.
 */
static void Main_LogStatus(void);

/* USER CODE END PFP */

/* Code nguoi dung rieng -----------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  Diem vao cua ung dung.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Cau hinh MCU ------------------------------------------------------------*/

  /* Reset ngoai vi, khoi tao Flash interface va SysTick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Cau hinh xung nhip he thong. */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Khoi tao cac ngoai vi da cau hinh. */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  UART2_Init(&huart2);
  HCSR04_Init();
  L298N_Init();
  Throttle_Init();
  Alerts_Init();
  sensorTaskLastTick = HAL_GetTick();
  appTaskLastTick = sensorTaskLastTick;
  logTaskLastTick = sensorTaskLastTick;
  
  SystemConfig_t sys_config = {
      .warning_distance = DEFAULT_WARNING_DISTANCE,
      .danger_distance = DEFAULT_DANGER_DISTANCE,
      .safe_throttle_limit = DEFAULT_SAFE_THROTTLE_LIMIT
  };
  
  App_AebLogic_Init(sys_config);
  /* USER CODE END 2 */

  /* Vong lap vo han. */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint32_t now = HAL_GetTick();

      if ((uint32_t)(now - sensorTaskLastTick) >= SENSOR_TASK_PERIOD_MS)
      {
          sensorTaskLastTick = now;

          /* Doc cac ngo vao va luu gia tri moi vao RTE. */
          HCSR04_Process();
          Throttle_Process();
      }

      if ((uint32_t)(now - appTaskLastTick) >= APP_TASK_PERIOD_MS)
      {
          appTaskLastTick = now;

          /* Tinh trang thai, sau do thuc thi motor va canh bao. */
          App_AebLogic_Process();
          Throttle_Execute();
          Alert_Execute();
      }

      if ((uint32_t)(now - logTaskLastTick) >= LOG_TASK_PERIOD_MS)
      {
          logTaskLastTick = now;

          /* UART cham hon dieu khien, nen viec log chay bang tac vu rieng. */
          Main_LogStatus();
      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief Cau hinh xung nhip he thong.
  * @retval Khong co
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Cau hinh dien ap ngo ra cua bo dieu ap noi chinh.
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Khoi tao cac bo dao dong RCC theo tham so trong cau truc
  * RCC_OscInitTypeDef.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Khoi tao xung nhip cho CPU, bus AHB va bus APB.
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Ham khoi tao USART2.
  * @param Khong co
  * @retval Khong co
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief Ham khoi tao GPIO.
  * @param Khong co
  * @retval Khong co
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* Bat xung nhip cho cac port GPIO. */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Cau hinh muc ngo ra ban dau cho GPIO. */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /* Cau hinh chan GPIO: B1_Pin. */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /* Cau hinh chan GPIO: LD2_Pin. */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/*
 * Doi trang thai he thong sang chuoi de doc va in mot dong trang thai.
 */
static void Main_LogStatus(void)
{
  const char *state_str = "UNKNOWN";
  SystemState_t state = Rte_Read_SystemState();
  uint16_t raw_distance_cm10 = Rte_Read_RawDistanceCm10();

  switch (state)
  {
    case STATE_CRUISE:       state_str = "CRUISE"; break;
    case STATE_FCW:          state_str = "FCW WARNING"; break;
    case STATE_AEB:          state_str = "AEB BRAKING"; break;
    case STATE_SAFE_RELEASE: state_str = "SAFE RELEASE"; break;
    default:                 state_str = "UNKNOWN"; break;
  }

  UART_Printf("State: [%s] | Dist: %d cm (Raw: %d.%d cm) | Throttle: %d%% | Motor: %d%% | ADC: %d\r\n",
              state_str,
              Rte_Read_Distance(),
              raw_distance_cm10 / 10U,
              raw_distance_cm10 % 10U,
              Rte_Read_ThrottlePercent(),
              Rte_Read_MotorSpeed(),
              Rte_Read_ThrottleAdc());
}

/* USER CODE END 4 */

/**
  * @brief  Ham nay duoc goi khi xay ra loi.
  * @retval Khong co
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* Co the them xu ly rieng de bao trang thai loi HAL. */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Bao ten file nguon va so dong noi xay ra loi assert_param.
  * @param  file: con tro toi ten file nguon
  * @param  line: so dong xay ra loi assert_param
  * @retval Khong co
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* Co the them xu ly rieng de bao ten file va so dong,
     vi du: printf("Gia tri tham so sai: file %s dong %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
