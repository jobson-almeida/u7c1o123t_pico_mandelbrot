#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <complex.h>
#include "ssd1306.h"

uint8_t mandelbrot_cache[SSD1306_BUF_LEN];
float cached_real_start, cached_real_end, cached_im_start, cached_im_end;

/*!
 * @brief Calcula o tamanho do buffer para uma área de renderização.
 *
 * Esta função calcula o comprimento do buffer necessário para uma dada
 * área de renderização no display.
 *
 * @param area Ponteiro para a estrutura render_area_t contendo as dimensões da área.
 */
void calc_render_area_buflen(render_area_t *area)
{
    // calcular quanto tempo o buffer achatado será para uma área de renderização
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

/*!
 * @brief Envia um comando para o display SSD1306.
 *
 * Esta função envia um único comando para o display SSD1306 através da interface I2C.
 * O processo de escrita I2C espera um byte de controle seguido pelos dados.
 * Neste caso, o byte de controle indica que os dados são um comando.
 *
 * @param cmd O comando a ser enviado
 *
 * @details
 * - A função configura o byte de controle para indicar que um comando está sendo enviado (Co = 1, D/C = 0).
 * - Em seguida, envia o byte de controle e o comando através da interface I2C.
 * - A comunicação I2C é realizada através da função `i2c_write_blocking`.
 *
 * @note
 * Esta função é utilizada para configurar o display SSD1306, definindo parâmetros como
 * modo de endereçamento, taxa de contraste, etc.
 */
void SSD1306_send_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(I2C_INST, SSD1306_I2C_ADDR, buf, 2, false);
}

/*!
 * @brief Envia uma lista de comandos para o display SSD1306.
 *
 * Esta função itera sobre um array de comandos e os envia individualmente para o display SSD1306
 * através da função `SSD1306_send_cmd`.
 *
 * @param buf   Um ponteiro para o array de comandos a serem enviados.
 * @param num   O número de comandos no array
 *
 * @note
 * Esta função é útil para enviar uma sequência de comandos de configuração para o display SSD1306
 * de forma eficiente. É comumente utilizada durante a inicialização do display para configurar
 * diversos parâmetros como modo de endereçamento, taxa de contraste e outros. [2-4]
 */
void SSD1306_send_cmd_list(uint8_t *buf, int num)
{
    for (int i = 0; i < num; i++)
        SSD1306_send_cmd(buf[i]);
}

/*!
 * @brief Envia um buffer de dados para o display SSD1306.
 *
 * Esta função envia um buffer de dados para o display SSD1306 através da interface I2C.
 * No modo de endereçamento horizontal, o ponteiro de endereço da coluna é incrementado automaticamente
 * e, em seguida, volta para a próxima página, permitindo enviar o frame inteiro de uma só vez.
 *
 * @param buf     Um ponteiro para o buffer de dados a ser enviado
 * @param buflen  O tamanho do buffer de dados em bytes.
 *
 * @details
 *  - A função aloca um buffer temporário (`temp_buf`) com tamanho `buflen + 1` para adicionar o byte de controle.
 *  - O primeiro byte de `temp_buf` é definido como `0x40`, indicando que os dados a seguir são dados para a RAM do display.
 *  - Os dados do buffer original (`buf`) são copiados para `temp_buf` a partir do segundo byte.
 *  - A função utiliza `i2c_write_blocking` para enviar o conteúdo de `temp_buf` para o display.
 *  - Após a transmissão, a memória alocada para `temp_buf` é liberada.
 *
 * @note
 *  - Esta função é utilizada para atualizar o conteúdo do display SSD1306 com os dados presentes no buffer.
 *  - É importante garantir que o tamanho do buffer (`buflen`) corresponda à área de exibição desejada.
 *  - A função pressupõe que o display esteja configurado no modo de endereçamento horizontal para
 *    o correto funcionamento.
 */
void SSD1306_send_buf(uint8_t buf[], int buflen)
{
    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buf, buflen);

    i2c_write_blocking(I2C_INST, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

/*!
 * @brief Inicializa o display SSD1306.
 *
 * @note
 *  - Esta função executa o processo de inicialização completo do display SSD1306,
 *    configurando diversos parâmetros como modo de endereçamento, taxa de contraste e outros.
 *  - Alguns comandos não são estritamente necessários, pois o processo de reinicialização
 *    usa como padrão alguns deles, mas eles são mostrados aqui para demonstrar como é a sequência de inicialização.
 *  - Alguns valores de configuração são recomendados pelo fabricante da placa.
 */
void SSD1306_init()
{
    uint8_t cmds[] = {
        SSD1306_SET_DISP, // desliga o display
        /* mapeamento de memória */
        SSD1306_SET_MEM_MODE, // define o modo de endereço de memória 0 = horizontal, 1 = vertical, 2 = página
        0x00,                 // modo de endereçamento horizontal
        /* resolução e layout */
        SSD1306_SET_DISP_START_LINE,    // define a linha de início do display para 0
        SSD1306_SET_SEG_REMAP | 0x01,   // define o remapeamento do segmento, o endereço da coluna 127 é mapeado para SEG0
        SSD1306_SET_MUX_RATIO,          // define a proporção multiplex
        SSD1306_HEIGHT - 1,             // altura do display - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // define a direção da varredura de saída COM (comum). Varredura de baixo para cima, COM[N-1] para COM0
        SSD1306_SET_DISP_OFFSET,        // define o deslocamento do display
        0x00,                           // sem deslocamento
        SSD1306_SET_COM_PIN_CFG,        // define a configuração de hardware dos pinos COM (comuns). Número mágico específico da placa.
        0x12,                           // display 128x64.

        /* esquema de time e driving */
        SSD1306_SET_DISP_CLK_DIV, // define a taxa de divisão do clock de exibição
        0x80,                     // taxa de divisão de 1, frequência padrão
        SSD1306_SET_PRECHARGE,    // define o período de pré-carga
        0xF1,                     // Vcc gerado internamente em nossa placa
        SSD1306_SET_VCOM_DESEL,   // define o nível de desseleção VCOMH
        0x30,                     // 0,83 vcc
        /* display */
        SSD1306_SET_CONTRAST, // define o controle de contraste
        0xFF,
        SSD1306_SET_ENTIRE_ON,     // define a exibição inteira para seguir o conteúdo da RAM
        SSD1306_SET_NORM_DISP,     // define a exibição normal (não invertida)
        SSD1306_SET_CHARGE_PUMP,   // define a bomba de carga
        0x14,                      // vcc gerado internamente em nossa placa
        SSD1306_SET_SCROLL | 0x00, // desativa a rolagem horizontal se definida. Isso é necessário, pois as gravações na memória serão corrompidas se a rolagem estiver habilitada
        SSD1306_SET_DISP | 0x01,   // liga a exibição
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

/*!
 * @brief Atualiza uma porção do display com uma área de renderização.
 *
 * @param buf   Um ponteiro para o buffer contendo os dados a serem exibidos.
 * @param area  Um ponteiro para a estrutura `render_area_t` que define a área do display a ser atualizada.
 *
 * @note
 *  - Esta função é utilizada para atualizar uma parte específica do display SSD1306 com os dados fornecidos no buffer.
 *  - A estrutura `render_area_t` define a região do display que será afetada pela operação de renderização.
 *  - É importante garantir que o tamanho do buffer corresponda à área de renderização definida para evitar erros de exibição.
 */
void render(uint8_t *buf, render_area_t *area)
{
    // atualizar uma parte da exibição com uma área de renderização
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page};

    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}

/*!
 * @brief Define ou limpa um pixel específico no buffer do display.
 *
 * @param buf   Um ponteiro para o buffer que representa a memória do display.
 * @param x     A coordenada X do pixel (coluna).
 * @param y     A coordenada Y do pixel (linha).
 * @param on    Um valor booleano indicando se o pixel deve ser aceso (true) ou apagado (false).
 *
 * @details
 *  - A função calcula o índice do byte correto no buffer com base nas coordenadas X e Y do pixel.
 *  - O cálculo leva em consideração o modo de endereçamento horizontal do display, onde os pixels são organizados em linhas de 8 pixels de altura.
 *  - A função utiliza operações de bitwise para definir ou limpar o bit correspondente ao pixel no byte apropriado.
 *  - Se `on` for verdadeiro, o bit é definido (`byte |= 1 << (y % 8)`).
 *  - Se `on` for falso, o bit é limpo (`byte &= ~(1 << (y % 8))`).
 *  - O byte modificado é então escrito de volta no buffer.
 *
 * @note
 *  - Esta função é fundamental para manipular os pixels individualmente no display SSD1306.
 *  - As coordenadas X e Y devem estar dentro dos limites do display (0 <= x < SSD1306_WIDTH, 0 <= y < SSD1306_HEIGHT).
 */
void set_pixel(uint8_t *buf, int x, int y, bool on)
{
    assert(x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT);

    const int bytes_per_row = SSD1306_WIDTH; // x pixels, 1bpp, mas cada linha tem 8 pixels de altura, então (x / 8) * 8

    int byte_idx = (y / 8) * bytes_per_row + x;
    uint8_t byte = buf[byte_idx];

    if (on)
        byte |= 1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    buf[byte_idx] = byte;
}

/*!
 * @brief Desenha um cursor (quadrado) no buffer do display.
 *
 * @param buf    Um ponteiro para o buffer que representa a memória do display.
 * @param top    A coordenada Y do canto superior esquerdo do cursor (linha).
 * @param left   A coordenada X do canto superior esquerdo do cursor (coluna).
 * @param width  A largura do cursor em pixels.
 * @param height A altura do cursor em pixels.
 * @param on     Um valor booleano indicando se o cursor deve ser desenhado (true) ou apagado (false).
 *
 * @details
 *  - A função itera sobre as bordas superior e inferior do quadrado, definindo ou limpando os pixels correspondentes no buffer.
 *  - Em seguida, itera sobre as bordas esquerda e direita do quadrado, definindo ou limpando os pixels correspondentes no buffer.
 *  - Utiliza a função `set_pixel()` para definir ou limpar cada pixel individualmente.
 *
 * @note
 *  - As coordenadas e dimensões devem estar dentro dos limites do display (0 <= left < SSD1306_WIDTH, 0 <= top < SSD1306_HEIGHT, left + width <= SSD1306_WIDTH, top + height <= SSD1306_HEIGHT).
 *  - Essa função é utilizada para destacar uma área específica do display.
 */
void draw_cursor(uint8_t *buf, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool on)
{
    for (uint8_t x = left; x < left + width; ++x)
    {
        set_pixel(buf, x, top, on);
        set_pixel(buf, x, top + height - 1, on);
    }
    for (uint8_t y = top; y < top + height; ++y)
    {
        set_pixel(buf, left, y, on);
        set_pixel(buf, left + width - 1, y, on);
    }
}

/*!
 * @brief Calcula se um ponto pertence ao conjunto de Mandelbrot.
 *
 * @param c        Um número complexo representando o ponto a ser testado.
 *
 * @return int     O número de iterações que o ponto leva para escapar, ou max_iter se o ponto pertence ao conjunto.
 *
 * @details
 *  - Implementa o algoritmo para determinar se um ponto complexo pertence ao conjunto de Mandelbrot.
 *  - Inicia com z = 0 e itera z = z^2 + c até que o módulo de z seja maior que 2 ou o número máximo de iterações seja atingido.
 */
int mandelbrot(float complex c)
{
    float complex z = 0.0 + 0.0 * I;
    int n = 0;
    while (cabsf(z) <= 2 && n < MAX_ITER) // a função cabsf() calcula o valor absoluto (magnitude) de um número complexo do tipo float complex
    {
        z = z * z + c;
        n++;
    }
    return n;
}

/*!
 * @brief Renderiza o conjunto de Mandelbrot no buffer do display.
 *
 * @param buf        Um ponteiro para o buffer do display.
 * @param real_start O limite real inicial do plano complexo.
 * @param real_end   O limite real final do plano complexo.
 * @param im_start   O limite imaginário inicial do plano complexo.
 * @param im_end     O limite imaginário final do plano complexo.
 *
 * @details
 *  - Otimiza a renderização através do uso de cache.
 *  - Calcula o incremento para os eixos X e Y.
 *  - Itera sobre cada pixel do display, calculando a parte real e imaginária do ponto correspondente no plano complexo.
 *  - Utiliza a função `mandelbrot()` para determinar se o ponto pertence ao conjunto.
 *  - Atualiza o cache para uso futuro
 */
void draw_mandelbrot(uint8_t *buf, float real_start, float real_end, float im_start, float im_end)
{
    // verifica se os dados estão em cache
    if (real_start == cached_real_start && real_end == cached_real_end && im_start == cached_im_start && im_end == cached_im_end)
    {
        // utiliza os dados de renderização em cache
        memcpy(buf, mandelbrot_cache, SSD1306_BUF_LEN);
        return;
    }
    float stepX = (real_end - real_start) / SSD1306_WIDTH;
    float stepY = (im_end - im_start) / SSD1306_HEIGHT;

    // printf("stepX: %.2f\t stepY: %.2f\n", stepX, stepY);

    for (int x = 0; x < SSD1306_WIDTH; x++)
    {
        for (int y = 0; y < SSD1306_HEIGHT; y++)
        {
            float real = real_start + x * stepX;
            float imag = im_start + y * stepY;
            float complex c = real + imag * I;
            int m = mandelbrot(c);
            bool pixelOn = (m == MAX_ITER); // ajuste MAX_ITER conforme necessário

            set_pixel(buf, x, y, pixelOn); // define o pixel no buffer
        }
    }
    // atualiza o cache
    cached_real_start = real_start;
    cached_real_end = real_end;
    cached_im_start = im_start;
    cached_im_end = im_end;
    memcpy(mandelbrot_cache, buf, SSD1306_BUF_LEN);
}
