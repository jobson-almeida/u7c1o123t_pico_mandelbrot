/*!
 * @file setup.h
 * @brief Header file contendo definições e declarações para a configuração inicial do hardware.
 *
 * Este arquivo define pinos GPIO e funções para configurar o hardware,
 * incluindo LED, botões e joystick.
 */

 #ifndef _SETUP_
 #define _SETUP_
 
 #include "ssd1306.h"
 
 /*! @brief Pino GPIO conectado ao LED. */
 #define LED 11
 
 /*! @brief Pino GPIO conectado ao botão A. */
 #define BUTTON_A 5
 
 /*! @brief Pino GPIO conectado ao botão B. */
 #define BUTTON_B 6
 
 /*! @brief Pino GPIO conectado ao botão SW (joystick). */
 #define SW 22
 
 /*! @brief Pino de leitura do eixo X do joystick (conectado ao ADC). */
 #define VRX 26
 
 /*! @brief Pino de leitura do eixo Y do joystick (conectado ao ADC). */
 #define VRY 27
 
 /*! @brief Canal ADC para o eixo X do joystick. */
 #define ADC_CHANNEL_0 0
 
 /*! @brief Canal ADC para o eixo Y do joystick. */
 #define ADC_CHANNEL_1 1
 
 /*!
  * @brief Configurações gerais do sistema.
  *
  * Esta função inicializa os pinos GPIO para os botões e o LED.
  */
 void setup_general();
 
 /*!
  * @brief Configurações para a comunicação I2C.
  *
  * Esta função inicializa e configura a comunicação I2C para o display SSD1306.
  */
 void setup_i2c();
 
 /*!
  * @brief Configurações para o joystick.
  *
  * Esta função inicializa e configura os pinos e o ADC para leitura dos dados do joystick.
  */
 void setup_joystick();
 
 #endif