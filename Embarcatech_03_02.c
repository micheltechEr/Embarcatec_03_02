#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// -- Definição de constantes
#define DISPLAY_I2C_PORT i2c1
#define DISPLAY_I2C_SDA 14
#define DISPLAY_I2C_SCL 15
#define DISPLAY_I2C_ENDERECO 0x3C

#define BUTTON_PIN_A 5
#define BUTTON_PIN_B 6
#define MATRIZ_LEDS_PIN 7
#define NUM_LEDS 25
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12

// Estrutura do pixel GRB (Padrão do WS2812)
struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

// Variáveis globais
npLED_t leds[NUM_LEDS];
PIO np_pio;
uint sm;
volatile bool usb_conexao = true;
static volatile uint32_t last_time = 0;
ssd1306_t ssd;

// Função para definir a cor de um LED específico
void cor(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (index < NUM_LEDS) {
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

// Função para desligar todos os LEDs
void desliga() {
    for (uint i = 0; i < NUM_LEDS; ++i) {
        cor(i, 0, 0, 0);
    }
}

// Função para enviar o estado atual dos LEDs ao hardware
void buffer() {
    for (uint i = 0; i < NUM_LEDS; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G << 8u | leds[i].R << 16u | leds[i].B);
    }
}

// Função para converter a posição da matriz para uma posição do vetor
int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x; // Linha par (esquerda para direita)
    } else {
        return y * 5 + (4 - x); // Linha ímpar (direita para esquerda)
    }
}

// Função que guarda os frames dos números
void num_matriz_leds(char num) {
    desliga();
    if (num >= '0' && num <= '9') {
        int frame[5][5][3] = {
            {{0, 0, 0}, {0, 245, 255}, {0, 245, 255}, {0, 245, 255}, {0, 0, 0}},
            {{0, 0, 0}, {0, 245, 255}, {0, 0, 0}, {0, 245, 255}, {0, 0, 0}},
            {{0, 0, 0}, {0, 245, 255}, {0, 0, 0}, {0, 245, 255}, {0, 0, 0}},
            {{0, 0, 0}, {0, 245, 255}, {0, 0, 0}, {0, 245, 255}, {0, 0, 0}},
            {{0, 0, 0}, {0, 245, 255}, {0, 245, 255}, {0, 245, 255}, {0, 0, 0}}
        };
        for (int linha = 0; linha < 5; linha++) {
            for (int coluna = 0; coluna < 5; coluna++) {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame[coluna][linha][0], frame[coluna][linha][1], frame[coluna][linha][2]);
            }
        }
        buffer();
    }
}

// Função de callback da interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000) {
        last_time = current_time;
        if (gpio == BUTTON_PIN_A) {
            gpio_put(LED_PIN_GREEN, !gpio_get(LED_PIN_GREEN));
            printf("Botão A pressionado. LED Verde %s.\n", gpio_get(LED_PIN_GREEN) ? "Ligado" : "Desligado");
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, gpio_get(LED_PIN_GREEN) ? "LED Verde Ligado" : "LED Verde Desligado", 0, 40);
            ssd1306_send_data(&ssd);
        } else if (gpio == BUTTON_PIN_B) {
            gpio_put(LED_PIN_BLUE, !gpio_get(LED_PIN_BLUE));
            printf("Botão B pressionado. LED Azul %s.\n", gpio_get(LED_PIN_BLUE) ? "Ligado" : "Desligado");
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, gpio_get(LED_PIN_BLUE) ? "LED Azul Ligado" : "LED Azul Desligado", 0, 52);
            ssd1306_send_data(&ssd);
        }
    }
}

// Função principal
int main() {
    stdio_init_all();

    // Inicialização do display SSD1306
    i2c_init(DISPLAY_I2C_PORT, 400 * 1000);
    gpio_set_function(DISPLAY_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_I2C_SDA);
    gpio_pull_up(DISPLAY_I2C_SCL);

    ssd1306_init(&ssd, 128, 64, false, DISPLAY_I2C_ENDERECO, DISPLAY_I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicialização dos botões e LEDs
    gpio_init(BUTTON_PIN_A);
    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);
    gpio_pull_up(BUTTON_PIN_B);

    gpio_init(LED_PIN_GREEN);
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);

    // Inicialização da matriz WS2812
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    uint offset = pio_add_program(pio0, &ws2818b_program);
    ws2818b_program_init(np_pio, sm, offset, MATRIZ_LEDS_PIN, 800000);

    desliga();

    // Estado inicial do display
    ssd1306_draw_string(&ssd, "Tarefa 03 02", 8, 4);
    ssd1306_draw_string(&ssd, "Caractere: ", 8, 22);
    ssd1306_draw_string(&ssd, "LED Verde Off", 8, 40);
    ssd1306_draw_string(&ssd, "LED Azul Off", 8, 52);
    ssd1306_send_data(&ssd);

    // Interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        if (stdio_usb_connected()) {
            if (usb_conexao) {
                printf("\n\n--- USB Conectado - Programa escutando ---\n - Envie um caractere por vez para aparecer no display \n - Entre 0-9, A-Z e a-z\n");
                usb_conexao = false;
            }

            char c;
            if (scanf("%c", &c) == 1) {
                printf("Caractere recebido: '%c'\n", c);

                ssd1306_fill(&ssd, false);
                char str[2] = {c, '\0'};
                ssd1306_draw_string(&ssd, str, 96, 22);
                ssd1306_send_data(&ssd);

                if (c >= '0' && c <= '9') {
                    num_matriz_leds(c);
                }
            }
        }
        sleep_ms(50);
    }
}