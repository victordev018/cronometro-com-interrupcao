#include <stdio.h>
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"

// definições da pinagem do display
#define DISPLAY_WIDTH 128       // largura
#define DISPLAY_HEIGHT 64       // altura
#define I2C_SDA 14              // canal i2c de dados
#define I2C_SCL 15              // canal i2c do clock
#define DISPLAY_ADDRESS 0x3C    // endereço do display
ssd1306_t display;              // instância do display

// pinagem do botão B
#define BTN_B 6
bool button_is_active = false;

// configurações do timer
const int TIMER = 10;                       // tempo do cronômetro em segundos
struct repeating_timer timer_cronometro;    // estrutura de dados do timer para criar o cronômetro

// função que inicializa o display
void display_init() {

    // Inicializa I2C no canal 1
    i2c_init(i2c1, 400 * 1000);                     // 400 kHz de frequênica
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);      // configura o SDA (sáida de dados)
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);      // configura o SCL (saída de clock)
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display
    if (!ssd1306_init(&display, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_ADDRESS, i2c1)) { 
        printf("Falha ao inicializar o display SSD1306\n");
        return;
    }
    
    // limpa o display, caso tenha algum conteúdo
    ssd1306_clear(&display);
    printf("Display SSD1306 inicializado com sucesso!\n");
}

void draw_initial_screen() {
    ssd1306_clear(&display);    // limpa o que tiver no display

    // escrevendo o menu com o indicativo da funcionalidade do botão B
    ssd1306_draw_string(&display, 20, 0, 1, "Menu Cronometro");
    ssd1306_draw_string(&display, 7, 25, 1, "B - Iniciar timer");

    ssd1306_show(&display);     // coloca conteúdo no display
}

// função para iniciar o botão B
void button_init() {
    gpio_init(BTN_B);               // inicia o pino gpio
    gpio_set_dir(BTN_B, GPIO_IN);   // configura o botão como entrada
    gpio_pull_up(BTN_B);            // estado padrão HIGH (alto)
    button_is_active = true;        // altera o valor da variável de controle
}

// Callback que atualiza o cronômetro no display a cada segundo
bool update_cronometro() {
    // variável statica (persiste o valor a cada chamada da função
    // para indicar o segundo atual do cronômetro
    static int current_second = TIMER;

    // verificar se o cronômetro já chegou ao fim
    if (current_second == 0) {
        current_second = TIMER;     // reseta o timer
        button_is_active = true;    // ativa o botão
        draw_initial_screen();      // desenha a tela inicial
        return false;               // finaliza a execução do cronômetro, finalizando as chamadas do callback
    }

    // preparando mensagem no cronômetro para o display
    char timer_msg[20];
    snprintf(timer_msg, 20, "Timer: %d", current_second);

    // escreve a mensagem no display
    ssd1306_clear(&display);                                    // limpa o atual conteúdo do display
    ssd1306_draw_string(&display, 10, 20, 2, timer_msg);        // escreve o segundo atual no display
    ssd1306_show(&display);                                     // atualiza o display ccom o segundo atual do cronômetro

    // decrementa o segundo atual
    current_second--;
    return true;    // retorna true para o callback continuar sendo executado
}

// Callback executado ao pressionar o botão
void button_pressed_callback(uint pin_gpio, uint32_t event) {
    // caso o botão estiver bloqueado, finaliza a ação
    if (!button_is_active) return;

    // bloqueia o botão
    button_is_active = false;

    // criar o nosso cronômetro que é atualizado a cada segundo
    add_repeating_timer_ms(-1000, update_cronometro, NULL, &timer_cronometro);
}

// função que inicializa todos os dispositivos
void setup() {
    // inicializa a comunicação serial
    stdio_init_all();

    // inicializar o nosso display
    display_init();

    // inicializar o botão B
    button_init();

    // configurar interrupção para o botão quando houver uma descida de borda (1 para 0)
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, button_pressed_callback);
    
}

int main()
{
    // inicializa comunicação serial, dispositivos e configura interrupção para o botão
    setup();

    // saudações, exibe um bem vindo no display na inicialização do sistema, por 1.5 segundos
    ssd1306_draw_string(&display, 35, 25, 1, "Bem vindo...");
    ssd1306_show(&display);
    sleep_ms(1500);

    // desenha no display o menu inicial
    draw_initial_screen();

    while (true) {
        tight_loop_contents();
    }

    return 0;
}
