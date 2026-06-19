# BTLCSDL - FCW/AEB STM32F401RE

Du an mo phong he thong FCW/AEB tren STM32F401RE.

## Hardware

- MCU: STM32F401RE
- Distance sensor: HC-SR04
- Throttle input: potentiometer on ADC
- Motor driver: L298N
- Alerts: green/yellow/red LED and buzzer
- UART log: USART2, 9600 baud

## Pin map

| Function | STM32F401RE pin |
|---|---|
| HC-SR04 TRIG | PA0 |
| HC-SR04 ECHO | PA1 |
| USART2 TX | PA2 |
| USART2 RX | PA3 |
| Throttle ADC | PA4 |
| Motor PWM | PA8 |
| L298N IN1 | PB0 |
| L298N IN2 | PB1 |
| Green LED | PC0 |
| Yellow LED | PC1 |
| Red LED | PC2 |
| Buzzer | PC3 |

## Project structure

| Folder | Purpose |
|---|---|
| `ASW/` | AEB and FCW application logic |
| `BSW_ECUAL/` | Sensor, motor, throttle control, alert drivers |
| `RTE/` | Runtime interface between ASW and BSW |
| `Config/` | System state, thresholds, shared config |
| `Core/` | STM32CubeIDE generated and user core files |
| `Drivers/` | STM32 HAL/CMSIS drivers |

## Team workflow

`main` is the final integrated branch. Members should not push directly to `main`.

Use feature branches:

| Role | Branch | Main files |
|---|---|---|
| Leader / Integration | `feature/leader-integration` | `RTE/`, `Config/`, `Core/Src/main.c`, `BTLCSDL.ioc` |
| ASW logic | `feature/asw-aeb-logic` | `ASW/` |
| Sensors and throttle | `feature/sensor-throttle` | `BSW_ECUAL/Sensors/`, `BSW_ECUAL/HMI/Throttle.*`, `Core/Src/adc.c`, `Core/Src/delay.c` |
| Motor and alerts | `feature/motor-alerts` | `BSW_ECUAL/Motor/`, `BSW_ECUAL/HMI/Alerts.*` |

Every feature branch should be merged into `main` through a Pull Request reviewed by the leader.

## Build

Open the project with STM32CubeIDE, then build the `Debug` configuration.

Do not commit generated build folders such as `Debug/` and `Release/`.
