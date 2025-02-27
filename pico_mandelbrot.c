#include <stdio.h>        // Inclui a biblioteca padrão de entrada e saída.
#include "pico/stdlib.h"  // Inclui a biblioteca SDK padrão do Raspberry Pi Pico, que fornece funcionalidades para programação básica.
#include <stdlib.h>       // Inclui a biblioteca para definições de tipos, variáveis e funções comuns.
#include <string.h>       // Inclui a biblioteca com funções para manipulação de strings e memória
#include "hardware/irq.h" // Inclui a biblioteca com funções para manipulação de interrupções de hardware.
#include "hardware/adc.h" // Inclui a biblioteca com funções para controlar o ADC do microcontrolador.
#include "ssd1306.h"      // Inclui a biblioteca que com definições e funções específicas para controlar o display OLED SSD1306.
#include "setup.h"        // Inclui a biblioteca com funções de configuração específicas de configuração e inicialização do hardware embarcado

uint32_t last_time = 0;        // variável de tempo, auxiliar À comtramedida deboucing
uint16_t vrx_value, vry_value; // variáveis para armazenar os valores do joystick (eixos X e Y) e botão
uint8_t buf[SSD1306_BUF_LEN];  // buffer com tamanho representa a área do display

volatile float real_start = -2.0; //  valor inicial da parte real do plano complexo para o cálculo do conjunto de Mandelbrot; este valor define o ponto de partida no eixo real para o desenho do fractal.
volatile float real_end = 1.0;    // valor final da parte real do plano complexo; este valor define o limite no eixo real até onde o cálculo do conjunto de Mandelbrot será realizado.
volatile float im_start = -1.5;   // valor inicial da parte imaginária do plano complexo; define o ponto de partida no eixo imaginário para o desenho do conjunto de Mandelbrot.
volatile float im_end = 1.5;      // valor final da parte imaginária do plano complexo; define o limite no eixo imaginário até onde o cálculo do conjunto de Mandelbrot será realizado

// variaveis auxiliares às variáveis acima declaradas
volatile float temp_real_start = 0;
volatile float temp_real_end = 0;
volatile float temp_im_start = 0;
volatile float temp_im_end = 0;

// variáveis que correspondem as coordenadas do cursor
volatile uint8_t new_x_position = 0;
volatile uint8_t new_y_position = 0;
volatile uint8_t temp_cursor_x_position = 0;
volatile uint8_t temp_cursor_y_position = 0;

// variavés que correspondem ao tamanho do cursor
volatile uint8_t new_height = 0;
volatile uint8_t new_width = 0;
volatile int new_cursor_size = 0;
volatile uint8_t temp_cursor_size = 0;

// variável que define o comportamento dos botões A e B.
// true = dimensionamento do cursor
// false = renderização do conjunto de Mandelbrot
volatile bool cursor_button_status = true;

render_area_t *render_area;

render_data_t *render_data;          // ponteiro de armazenamento dos dados do plano complexo que utilizados nos cálculos de renderização do conjunto de Mandelbrot
volatile int render_data_count = -1; // contador do ponteiro de armazenamento de dados de renderização

render_area_t frame_area = {
    start_col : 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
    end_page : SSD1306_NUM_PAGES - 1
};

// função para ler os eixos X e Y do joystick.
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
    // leitura do valor do eixo X do joystick
    adc_select_input(ADC_CHANNEL_0); // seleciona o canal ADC para o eixo X do joystick
    sleep_us(2);                     // pequeno delay para garantir estabilidade da leitura do ADC
    *vrx_value = adc_read();         // lê o valor do eixo X (0-4095)

    // leitura do valor do eixo Y do joystick
    adc_select_input(ADC_CHANNEL_1); // seleciona o canal ADC para o eixo Y do joystick
    sleep_us(2);                     // pequeno delay para estabilidade
    *vry_value = adc_read();         // lê o valor do eixo Y (0-4095)
}

// função que desenha o fractal e o cursor no centro do display
void controller(uint8_t x0, uint8_t y0)
{
    new_height = new_cursor_size; // define a altura do cursor (new_height) com base no tamanho atual do cursor (new_cursor_size).
    new_width = new_cursor_size;  // define a largura do cursor (new_width) também com base no tamanho atual do cursor (new_cursor_size).

    new_x_position = x0; // atualiza a posição X do cursor (new_x_position) para a coordenada X de entrada (x0)
    new_y_position = y0; // atualiza a posição Y do cursor (new_y_position) para a coordenada Y de entrada (y0).

    // compara as posições XY atuais do cursor (x0/y0) com as posições X temporárias armazenadas (temp_cursor_x_position/temp_cursor_y_position).
    // memcmp é usada para comparar os valores na memória, considerando que são ponteiros
    int check_cursor_x_position = memcmp(&x0, (int *)&temp_cursor_x_position, sizeof(uint8_t));
    int check_cursor_y_position = memcmp(&y0, (int *)&temp_cursor_y_position, sizeof(uint8_t));

    if (check_cursor_x_position != 0 || check_cursor_y_position != 0 || new_cursor_size != temp_cursor_size ||
        real_start != temp_real_start || real_end != temp_real_end || im_start != temp_im_start || im_end != temp_im_end)
    {

        memset(buf, 0, SSD1306_BUF_LEN); // limpa o buffer
        render(buf, &frame_area);        // renderiza o buffer limpo no display

        draw_mandelbrot(buf, real_start, real_end, im_start, im_end);
        draw_cursor(buf, new_y_position, new_x_position, new_width, new_height, true);

        render(buf, &frame_area); // renderiza o buffer atualizado

        // variáveis auxliares do cursor - coordenadas e tamanho
        temp_cursor_x_position = new_x_position;
        temp_cursor_y_position = new_y_position;
        temp_cursor_size = new_cursor_size;

        // variáveis auxliares e dados do plano complexo utilizados nos cálculos de renderização do conjunto de Mandelbrot
        temp_real_start = real_start;
        temp_real_end = real_end;
        temp_im_start = im_start;
        temp_im_end = im_end;
    }
}

void zoom_in(uint8_t left, uint8_t top, uint8_t width, uint8_t height)
{
    // calcula o centro do cursor
    uint8_t x0 = left + width / 2;
    uint8_t y0 = top + height / 2;

    int new_height_zoom = height / 2;
    int new_width_zoom = width / 2;

    // printf("%d %d %d %d\n", x0, y0, new_width_zoom, new_height_zoom);

    float real_range = real_end - real_start;
    float im_range = im_end - im_start;

    // printf("%f %f\n", real_range, im_range);

    float left_point = x0 - new_width_zoom;
    float right_point = x0 + new_width_zoom;
    float top_point = y0 - new_height_zoom;
    float bottom_point = y0 + new_height_zoom;

    // printf("%f %f %f %f\n", left_point, right_point, top_point, bottom_point);

    real_start = real_start + (real_range * left_point / SSD1306_WIDTH);
    real_end = real_start + (right_point - left_point) * real_range / SSD1306_WIDTH;
    im_start = im_start + (im_range * top_point / SSD1306_HEIGHT);
    im_end = im_start + (bottom_point - top_point) * im_range / SSD1306_HEIGHT;

    // printf("%f %f %f %f\n", real_start, real_end, im_start, im_end);
}

void undo_zoom_in(uint8_t left, uint8_t top, uint8_t width, uint8_t height)
{

    // printf("%f %f %f %f\n",
    //        render_data[render_data_count].real_start,
    //        render_data[render_data_count].real_end,
    //        render_data[render_data_count].im_start,
    //        render_data[render_data_count].im_end);

    if (render_data_count >= 0 && render_data_count <= 10) // condicional que limita o decremento e quantidade de itens no ponteiro
    {
        // coletando os dados anteriores a última ampliação para renderização retroativa fidedigna
        real_start = render_data[render_data_count].real_start;
        real_end = render_data[render_data_count].real_end;
        im_start = render_data[render_data_count].im_start;
        im_end = render_data[render_data_count].im_end;

        render_data = realloc(render_data, render_data_count * sizeof(render_data_t));
        if (render_data == NULL)
        {
            // erro de alocação de memória
            printf("falha ao alocar memória para render_data\n");
            return;
        }
        render_data_count--; // decrementa total de itens no ponteiro de renderizaçoes
    }
}

bool controller_repeating_timer_callback(struct repeating_timer *t)
{
    joystick_read_axis(&vrx_value, &vry_value); // lê os valores dos eixos do joystick

    // ajuste de 4095 -> 4084 e new_cursor_size é o tamanha atual do cursor
    // ATENÇÃO: os eixos da minha placa estão invertidos
    uint8_t x_cursor = (int)(((float)vry_value / 4084.0) * (127.0 - new_cursor_size));

    // ajuste de 4095 -> 4082 e new_cursor_size é o tamanho atual do cursor
    uint8_t y_cursor = (int)((63 - new_cursor_size) - (((float)vrx_value / 4082.0) * (63.0 - new_cursor_size)));

    // printf("%d %d\n", vrx_value, vry_value);
    controller(x_cursor, y_cursor);
}

// handler de interrupção dos botões
void button_interruption_gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    // verificar se passou tempo o bastante desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // atualiza o tempo do último evento

        if (gpio_get(BUTTON_A) == 0) // verifica se o botão A está pressionado (nível lógico baixo).
        {
            if (cursor_button_status)
            {
                new_cursor_size++; // se cursor_button_status for verdadeiro, incrementa o tamanho do cursor.
            }
            else
            {
                render_data_count++; // se cursor_button_status for falso, incrementa o contador de ampliações
                // armazena os valores atuais de real_start, real_end, im_start e im_end no array render_data na posição render_data_count.
                render_data[render_data_count].real_start = real_start;
                render_data[render_data_count].real_end = real_end;
                render_data[render_data_count].im_start = im_start;
                render_data[render_data_count].im_end = im_end;

                zoom_in(new_x_position, new_y_position, new_width, new_height); // chama a função zoom_in para realizar a ampliação.
            }
        }

        if (gpio_get(BUTTON_B) == 0) // verifica se o botão B está pressionado (nível lógico baixo).
        {
            if (cursor_button_status)
            {
                new_cursor_size--; // se cursor_button_status for verdadeiro, decrementa o tamanho do cursor.
                if(new_cursor_size < 0) new_cursor_size = 0;
                printf("%d\n", new_cursor_size);
            }
            else
            {
                // chama a função undo_zoom_in para desfazer a ampliação.
                undo_zoom_in(new_x_position, new_y_position, new_width, new_height);
            }
        }

        if (gpio_get(SW) == 0) // verifica se o botão SW está pressionado (nível lógico baixo).
        {
            cursor_button_status = !cursor_button_status; // inverte o valor de cursor_button_status.
            // indicação visual para ampliação e manipulação do cursor
            // - aceso indica habilitado para ampliação
            // - apagado indica habilitado para manipular tamanho do cursor
            gpio_put(LED, !gpio_get(LED)); // inverte o estado do LED, fornecendo uma indicação visual se a atual função do botões A e B.
        }
    }
    // limpa a interrupção do GPIO, permitindo que novas interrupções sejam detectadas.
    gpio_acknowledge_irq(gpio, events);
}

int main()
{
    stdio_init_all(); // inicializa as funções de entrada e saída padrão - stdio.h
    setup_general();  // configura as configurações gerais do hardware
    setup_joystick(); // inicializa e configura o joystick
    setup_i2c();      // inicializa e configura a interface I2C

    render_data = NULL; // garante que o ponteiro não contenha um endereço de memória aleatório ou inválido antes da alocação.
    // aloca a memória para amazenar os dados do plano complexo utilizados nos cálculos de renderização do conjunto de Mandelbrot
    render_data = malloc(10 * sizeof(render_data_t));
    if (render_data == NULL)
    {
        // erro de alocação de memória
        printf("falha ao alocar memória para os dados de renderização\n");
        return 1;
    }

    // habilita a interrupção para os botóes
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_interruption_gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_interruption_gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, &button_interruption_gpio_irq_handler);

    // inicializa a área de renderização para o frame inteiro (SSD1306_WIDTH pixels por SSD1306_NUM_PAGES páginas)
    calc_render_area_buflen(&frame_area); // chamada sempre que você modificar os parâmetros da área de renderização

    memset(buf, 0, SSD1306_BUF_LEN); // limpa o buffer
    render(buf, &frame_area);

    struct repeating_timer timer;
    // timer de controle do cursor e controle das renderizações
    add_repeating_timer_ms(48, controller_repeating_timer_callback, NULL, &timer);

    printf("started\n");

    // loop infinito
    while (true)
    {
        tight_loop_contents(); // função no-op - sem operação
    }
    free(render_data);
    render_data = NULL; // garante que o ponteiro não contenha um endereço de memória pendente após a liberação da memória

    return 0; // boas práticas
}