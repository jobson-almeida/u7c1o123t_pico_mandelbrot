/*!
 * @file ssd1306.h
 * @brief Header file contendo definições e declarações para o controle do display OLED SSD1306.
 *
 * Este arquivo define constantes, comandos e funções para interagir com o display SSD1306 via I2C.
 */

 #ifndef _SSD1306_
 #define _SSD1306_
 
 #include <complex.h>
 #include "pico/stdlib.h"
 #include <stdio.h>
 
 /*! @brief Altura do display SSD1306 em pixels. */
 #define SSD1306_HEIGHT 64
 
 /*! @brief Largura do display SSD1306 em pixels. */
 #define SSD1306_WIDTH 128
 
 /*! @brief Altura de uma página do display SSD1306 em pixels. */
 #define SSD1306_PAGE_HEIGHT _u(8)
 
 /*! @brief Número de páginas do display SSD1306. */
 #define SSD1306_NUM_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
 
 /*! @brief Define o tamanho do buffer utilizado para armazenar os dados do display SSD1306.. */
 #define SSD1306_BUF_LEN (SSD1306_NUM_PAGES * SSD1306_WIDTH * 2)

 /*! @brief Pino SDA para a comunicação I2C. */
 #define I2C_SDA_PIN 14
 
 /*! @brief Pino SCL para a comunicação I2C. */
 #define I2C_SCL_PIN 15
 
 /*! @brief Instância I2C utilizada. */
 #define I2C_INST i2c1
 
 /*! @brief Endereço I2C do display SSD1306. */
 #define SSD1306_I2C_ADDR _u(0x3C)
 
 /*! @brief Clock da comunicação I2C (kHz). */
 #define SSD1306_I2C_CLK 400
 
 // Comandos (ver datasheet)
 /*! @brief Define o modo de endereço de memória. */
 #define SSD1306_SET_MEM_MODE _u(0x20)
 
 /*! @brief Define o endereço da coluna. */
 #define SSD1306_SET_COL_ADDR _u(0x21)
 
 /*! @brief Define o endereço da página. */
 #define SSD1306_SET_PAGE_ADDR _u(0x22)
 
 /*! @brief Configura rolagem horizontal. */
 #define SSD1306_SET_HORIZ_SCROLL _u(0x26)
 
 /*! @brief Ativa/desativa rolagem. */
 #define SSD1306_SET_SCROLL _u(0x2E)
 
 /*! @brief Define a linha de início do display. */
 #define SSD1306_SET_DISP_START_LINE _u(0x40)
 
 /*! @brief Define o controle de contraste. */
 #define SSD1306_SET_CONTRAST _u(0x81)
 
 /*! @brief Define a bomba de carga. */
 #define SSD1306_SET_CHARGE_PUMP _u(0x8D)
 
 /*! @brief Define o remapeamento do segmento. */
 #define SSD1306_SET_SEG_REMAP _u(0xA0)
 
 /*! @brief Define a exibição inteira como ligada. */
 #define SSD1306_SET_ENTIRE_ON _u(0xA4)
 
 /*! @brief Define todos os pixels como ligados. */
 #define SSD1306_SET_ALL_ON _u(0xA5)
 
 /*! @brief Define a exibição normal (não invertida). */
 #define SSD1306_SET_NORM_DISP _u(0xA6)
 
 /*! @brief Define a exibição invertida. */
 #define SSD1306_SET_INV_DISP _u(0xA7)
 
 /*! @brief Define a proporção de multiplexação. */
 #define SSD1306_SET_MUX_RATIO _u(0xA8)
 
 /*! @brief Liga/desliga o display. */
 #define SSD1306_SET_DISP _u(0xAE)
 
 /*! @brief Define a direção de saída COM (comum). */
 #define SSD1306_SET_COM_OUT_DIR _u(0xC0)
 
 /*! @brief Define a direção de saída COM (comum) invertida. */
 #define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)
 
 /*! @brief Define o deslocamento do display. */
 #define SSD1306_SET_DISP_OFFSET _u(0xD3)
 
 /*! @brief Define a divisão do clock do display. */
 #define SSD1306_SET_DISP_CLK_DIV _u(0xD5)
 
 /*! @brief Define o período de pré-carga. */
 #define SSD1306_SET_PRECHARGE _u(0xD9)
 
 /*! @brief Define a configuração dos pinos COM. */
 #define SSD1306_SET_COM_PIN_CFG _u(0xDA)
 
 /*! @brief Define o nível de deseleção VCOMH. */
 #define SSD1306_SET_VCOM_DESEL _u(0xDB)
 
 /*! @brief Modo de escrita. */
 #define SSD1306_WRITE_MODE _u(0xFE)
 
 /*! @brief Modo de leitura. */
 #define SSD1306_READ_MODE _u(0xFF)
 
 /*! @brief Número máximo de iterações. */
 #define MAX_ITER 80
 
 /*!
  * @brief Estrutura para definir a área de renderização.
  */
 typedef struct {
     uint8_t start_col;    /*!< Coluna inicial. */
     uint8_t end_col;      /*!< Coluna final. */
     uint8_t start_page;   /*!< Página inicial. */
     uint8_t end_page;     /*!< Página final. */
     int buflen;           /*!< Comprimento do buffer. */
 } render_area_t;
  
/*!
 * @brief Estrutura de dados utilizada para armazenar os limites do plano complexo
 *        durante a renderização do conjunto de Mandelbrot.
 */
 typedef struct {
    float real_start;
    float real_end;
    float im_start;
    float im_end;
} render_data_t;

void calc_render_area_buflen(render_area_t *area);

void SSD1306_send_cmd(uint8_t cmd);

void SSD1306_send_cmd_list(uint8_t *buf, int num);

void SSD1306_send_buf(uint8_t buf[], int buflen);

void SSD1306_init();

void render(uint8_t *buf, render_area_t *area);

void set_pixel(uint8_t *buf, int x, int y, bool on);

void draw_cursor(uint8_t *buf, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool on);

int mandelbrot(float complex c);

void draw_mandelbrot(uint8_t *buf, float real_start, float real_end, float im_Start, float im_end);

#endif