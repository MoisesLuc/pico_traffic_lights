#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

#define BTN_A_PIN 5

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

int A_state = 0;    //Botao A está pressionado?
int y = 0;

char *text[] = {    //Texto exibido no OLED
    "SINAL ABERTO",
    "ATRAVESSAR",

    "SINAL DE ATENCAO",
    "PREPARE-SE",

    "SINAL FECHADO",
    "AGUARDE"};

// Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};

uint8_t ssd[ssd1306_buffer_length];



// Função de ativação do display OLED
void display(int i) {
    // Resetar display a cada exibição
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
    y = 0;

    // Escrever no display
    ssd1306_draw_string(ssd, 5, y, text[i]);
    render_on_display(ssd, &frame_area);
    y += 8;
    ssd1306_draw_string(ssd, 5, y, text[i+1]);
    render_on_display(ssd, &frame_area);
}

void SinalAberto() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);   

    display(0);
}

void SinalAtencao() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    display(2);
}

void SinalFechado() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);

    display(4);
}

int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i = i+100) {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

int main() {
    
    // INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // INICIANDO BOTÄO
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    calculate_render_area_buffer_length(&frame_area);


    while(true) {

        SinalFechado();
        A_state = WaitWithRead(10000);   //espera com leitura do botäo


        if(A_state) {               //ALGUEM APERTOU O BOTAO - SAI DO SEMAFORO NORMAL
            //SINAL VERDE PARA OS PEDESTRES POR 8 SEGUNDOS
            SinalAberto();
            sleep_ms(8000);

            //SINAL AMARELO PARA OS PEDESTRE POR 2 SEGUNDOS, DEPOIS RETORNA AO SEMAFORO NORMAL
            SinalAtencao();
            sleep_ms(2000);

        }else {                          //NINGUEM APERTOU O BOTAO - CONTINUA NO SEMAFORO NORMAL
            //SINAL VERDE PARA OS PEDESTRES POR 8 SEGUNDOS             
            SinalAberto();
            sleep_ms(8000);

            //SINAL AMARELO PARA OS PEDESTRES POR 2 SEGUNDOS
            SinalAtencao();
            sleep_ms(2000);
        }
                
    }

    return 0;

}