#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// Definição de constantes
#define DISPLAY_I2C_PORT   i2c1
#define DISPLAY_I2C_SDA    14
#define DISPLAY_I2C_SCL    15
#define DISPLAY_I2C_ADDR   0x3C
#define BUTTON_PIN_A     5
#define BUTTON_PIN_B     6
#define MATRIX_LED_PIN   7
#define NUM_LEDS         25
#define LED_PIN_GREEN    11
#define LED_PIN_BLUE     12

// Declaração do display
ssd1306_t ssd;

// Estrutura para armazenar a cor (ordem GRB, conforme WS2812)
typedef struct {
    uint8_t G, R, B;
} pixel_t;
pixel_t leds[NUM_LEDS];

// Mapeamento físico dos LEDs na matriz:
//Usado anteriormente em projetos

const uint8_t led_map[NUM_LEDS] = {
    20, 21, 22, 23, 24,    // Linha superior
    15, 16, 17, 18, 19,
    10, 11, 12, 13, 14,
     5,  6,  7,  8,  9,
     0,  1,  2,  3,  4     // Linha inferior
};

//Instancia do PIO para controlar os LEDS e da State Machine
PIO ledPIO;
uint sm;

volatile bool usbConnected = true;
static volatile uint32_t lastTime = 0;


// Função que atribui uma cor (com ordem GRB) ao LED de índice 'index'
void setLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index] = (pixel_t){ g, r, b };
}

// Desliga (zera) todos os LEDs da matriz
void clearMatrix() {
    for (uint i = 0; i < NUM_LEDS; i++) {
        setLED(i, 0, 0, 0);
    }
}

// Envia o buffer de LEDs para o hardware via PIO
void sendLedBuffer() {
    for (uint i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(ledPIO, sm, leds[i].G);
        pio_sm_put_blocking(ledPIO, sm, leds[i].R);
        pio_sm_put_blocking(ledPIO, sm, leds[i].B);
    }
}

// Desenha um frame (matriz 5x5) usando o mapeamento.
void drawFrame(const int frame[5][5][3]) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int pos = led_map[row * 5 + col];
            setLED(pos, frame[row][col][0], frame[row][col][1], frame[row][col][2]);
        }
    }
    sendLedBuffer();
}

//Definição dos frames de 0 a 9
//Ela segue o modelo RGB e seu mapeamento é em ZIG ZAG

const int frames[10][5][5][3] = {
    // Dígito '0'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '1'
    {
        {{0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '2'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '3'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '4'
    {
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito '5'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '6'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '7'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito '8'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    },
    // Dígito '9'
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}}
    }
};


//Envio dos pixels a matriz de leds
//Ela é responsável por realizar o cálculo do indice correspondente na matriz frames e assim selecionar o frame

void displayDigitMatrix(char digit) {
    if (digit >= '0' && digit <= '9') {
        clearMatrix();
        drawFrame(frames[digit - '0']);
    }
}

// Função de callback para as interrupções dos botões
void buttonInterruptHandler(uint gpio, uint32_t events) {
    uint32_t currentTime = to_us_since_boot(get_absolute_time());
    // Debouncing (200ms)
    if (currentTime - lastTime > 200000) {
        lastTime = currentTime;
        if (gpio == BUTTON_PIN_A) {
            gpio_put(LED_PIN_GREEN, !gpio_get(LED_PIN_GREEN));
            const char* state = gpio_get(LED_PIN_GREEN) ? "On" : "Off";
            printf("LED Verde %s!\n", state);
            ssd1306_draw_string(&ssd, gpio_get(LED_PIN_GREEN) ? "LED Green   On" : "LED Green  Off", 8, 40);
        } else if (gpio == BUTTON_PIN_B) {
            gpio_put(LED_PIN_BLUE, !gpio_get(LED_PIN_BLUE));
            const char* state = gpio_get(LED_PIN_BLUE) ? "On" : "Off";
            printf("LED Azul %s!\n", state);
            ssd1306_draw_string(&ssd, gpio_get(LED_PIN_BLUE) ? "LED Blue    On" : "LED Blue   Off", 8, 52);
        }
        ssd1306_send_data(&ssd);
    }
}

// Realiza as inicializações (I2C, display, botões, LEDs, PIO, etc.)
void initialize_sets() {
    i2c_init(DISPLAY_I2C_PORT, 400 * 1000);
    gpio_set_function(DISPLAY_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_I2C_SDA);
    gpio_pull_up(DISPLAY_I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_I2C_ADDR, DISPLAY_I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);
    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_B);
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);

    ledPIO = pio0;
    sm = pio_claim_unused_sm(ledPIO, true);
    uint offset = pio_add_program(pio0, &ws2818b_program);
    ws2818b_program_init(ledPIO, sm, offset, MATRIX_LED_PIN, 800000);
    clearMatrix();

    ssd1306_draw_string(&ssd,"----------------------------",8,4);
    ssd1306_draw_string(&ssd, "Tarefa Embarcatech   03 02", 8, 4);
    ssd1306_draw_string(&ssd, "Caractere  ", 8, 22);
    ssd1306_draw_string(&ssd, "LED Green  Off", 8, 40);
    ssd1306_draw_string(&ssd, "LED Blue   Off", 8, 52);
    ssd1306_send_data(&ssd);

    gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, buttonInterruptHandler);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true, buttonInterruptHandler);
}


// Função chamada repetidamente no loop principal (verifica USB e processa caractere)
void setup() {
    if (stdio_usb_connected()) {
        if (usbConnected) {
            printf("\n\n--- USB Conectado - Programa escutando ---\n");
            printf(" - Envie um caractere por vez para aparecer no display \n");
            printf(" - Entre 0-9, A-Z e a-z\n");
            usbConnected = false;
        }
        char c;
        if (scanf("%c", &c) == 1) {
            printf("Enviando para a matriz de LED o numero : '%c'\n", c);
            ssd1306_draw_char(&ssd, c, 96, 22);
            ssd1306_send_data(&ssd);
            if (c >= '0' && c <= '9') displayDigitMatrix(c);
        }
    }
    sleep_ms(50);
}

int main() {
    stdio_init_all();
    initialize_sets();
    while (true) {
        setup();
    }
    return 0;
}
