/*
 * adc.c
 *
 * Created: 12/17/2019 12:09:35 AM
 * Author : Zsolt
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "uart.h"
#include "timer2.h"

/**
 * USART RX interrupt callback handle context
 */
struct USART_RXC_cb_ctx_t {
    uint8_t rx;  // received byte to be seved here
};

/**
 * USART RX interrupt callback handle
 * @param[inout] ctx user data for interrupt callback
 * When ISR occurs USART_RXC_cb will be called with ctx as parameter
 * UART RX data (UDR) should be saved in this function
 */
static void USART_RXC_cb_handle(void* ctx) {
    struct USART_RXC_cb_ctx_t* t_ctx = (struct USART_RXC_cb_ctx_t*)ctx;

    t_ctx->rx = UDR;
    printf("%c\n", t_ctx->rx);
}

/**
 * ADC and timer callback handle context
 */
struct cb_ctx_t {
    uint16_t adc[2];  // received adc
    uint8_t  toggle;
};

/**
 * ADC interrupt callback handle
 * @param[inout] ctx user data for interrupt callback
 * When ISR occurs ADC_cb will be called with ctx as parameter
 * ADC data (ADC) should be saved in this function
 */
static void ADC_cb_handle(void* ctx, uint16_t adc) {
    struct cb_ctx_t* t_ctx = (struct cb_ctx_t*)ctx;

    t_ctx->adc[t_ctx->toggle & 0x01] = adc;
    t_ctx->toggle++;

    ADMUX  &= 0xF8;  // clear admux
    ADMUX |= t_ctx->toggle & 0x01;
}

static void TIMER2_PWM_cb_handle(void* ctx) {
    struct cb_ctx_t* t_ctx = (struct cb_ctx_t*)ctx;

    OCR2 = 30 + (t_ctx->adc[0] >> 5);  // 0 ..1024 -> 30 .. 64 meaning Ton= [1..2] ms
}

int main(void) {
    // UART INIT
    //-------------------------------
    const uint16_t baud_rate = 38400;

    struct USART_RXC_cb_ctx_t USART_RXC_ctx = {};

    regiter_USART_RXC_cb(USART_RXC_cb_handle, &USART_RXC_ctx);

    USART_init(baud_rate);

    printf("Init Done UART baud: %u\n", (uint16_t)baud_rate);
    //-------------------------------

    // ADC INIT
    //-------------------------------
    struct cb_ctx_t ctx = {};

    regiter_ADC_isr_cb(ADC_cb_handle, &ctx);

    ADC_init(ADC_PS_128, 1);

    DDRB &= ~((1 << PC0) || (1 << PC1));  //set input for ADC

    printf("Init Done ADC\n");

    // TIMER2 init
    //-------------------------------
    regiter_TIMER2_isr_cb(TIMER2_PWM_cb_handle, &ctx);

    TIMER2_PWM_init(0, TIMER2_PS_PRESCALE_256);

    printf("Init Done TIMER2\n");
    //-------------------------------

    while (1) {
        printf("test %d %d\n", ctx.adc[0], ctx.adc[1]);
        _delay_ms(100);
    }
    return 0;
}
