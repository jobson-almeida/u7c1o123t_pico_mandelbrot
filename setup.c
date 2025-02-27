#include "setup.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"

/*!
 * @brief Inicializa as configurações gerais do hardware, incluindo o LED.
 *
 * @details
 *  - Configurações de clock.
 *  - Inicialização do stdio.
 *  - Inicialização do LED.
 *
 * @note
 *  - Esta função é essencial para preparar o hardware para a execução do programa.
 */
void setup_general()
{
    gpio_init(BUTTON_A);             // inicializa o pino do botão A
    gpio_set_dir(BUTTON_A, GPIO_IN); // configura o pino do botão A como entrada
    gpio_pull_up(BUTTON_A);          // ativa o pull-up no pino do botão A

    gpio_init(BUTTON_B);             // inicializa o pino do botão B
    gpio_set_dir(BUTTON_B, GPIO_IN); // configura o pino do LED como saída
    gpio_pull_up(BUTTON_B);          // ativa o pull-up no pino do botão B

    gpio_init(LED);              // inicializa o pino do LED
    gpio_set_dir(LED, GPIO_OUT); // configura o pino do LED como saída
    gpio_put(LED, false);        // desliga o LED
}

/*!
 * @brief Configura a interface I2C para comunicação com o display SSD1306.
 *
 * @details
 *  - Inicializa o I2C.
 *  - Configura os pinos SDA e SCL.
 *  - Inicializa o display SSD1306 através da função `SSD1306_init()`.
 *
 * @note
 *  - Garante a comunicação correta com o display OLED.
 */
void setup_i2c()
{
    i2c_init(I2C_INST, SSD1306_I2C_CLK * 1000);    // inicializa a comunicação I2C.
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // configura o pino SDA para a função I2C.
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // configura o pino SCL para a função I2C.
    gpio_pull_up(I2C_SDA_PIN);                     // ativa o pull-up no pino SDA.
    gpio_pull_up(I2C_SCL_PIN);                     // ativa o pull-up no pino SCL.
    SSD1306_init();                                // executa o processo de inicialização completo do display SSD1306.
}

/*!
 * @brief Inicializa e configura o joystick.
 *
 * @details
 *  - Configura os pinos do joystick como entrada.
 *  - Define resistores de pull-up/pull-down, se necessário.
 *
 * @note
 *  - Permite a leitura dos valores do joystick.
 */
void setup_joystick()
{
    // inicializa o ADC e os pinos de entrada analógica
    adc_init();         // inicializa o módulo ADC
    adc_gpio_init(VRX); // configura o pino VRX (eixo X) para entrada ADC
    adc_gpio_init(VRY); // configura o pino VRY (eixo Y) para entrada ADC

    // inicializa o pino do botão do joystick
    gpio_init(SW);             // inicializa o pino do botão do joystick
    gpio_set_dir(SW, GPIO_IN); // configura o pino do botão como entrada
    gpio_pull_up(SW);          // ativa o pull-up no pino do botão para evitar efeito bounce
}
