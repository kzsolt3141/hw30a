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
 * ADC interrupt callback handle context
 */
struct ADC_cb_ctx_t {
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
    struct ADC_cb_ctx_t* t_ctx = (struct ADC_cb_ctx_t*)ctx;

    t_ctx->adc[t_ctx->toggle & 0x01] = adc;
    t_ctx->toggle++;

    ADMUX  &= 0xF8;  // clear admux
    ADMUX |= t_ctx->toggle & 0x01;
}

int main(void) {
    uint8_t sts = 0;

    // UART INIT
    //-------------------------------
    const uint16_t baud_rate = 38400;

    struct USART_RXC_cb_ctx_t USART_RXC_ctx = {};

    regiter_USART_RXC_cb(USART_RXC_cb_handle, &USART_RXC_ctx);

    sts = USART_init(baud_rate, 1);
    if (sts) return sts;

    printf("Init Done UART baud: %u\n", (uint16_t)baud_rate);
    //-------------------------------

    // ADC INIT
    //-------------------------------
    struct ADC_cb_ctx_t ADC_ctx = {};

    regiter_ADC_isr_cb(ADC_cb_handle, &ADC_ctx);

    sts = ADC_init(ADC_PS_128, 1, 1);
    if (sts) return sts;

    DDRB &= ~((1 << PC0) || (1 << PC1));  //set input for ADC

    printf("Init Done ADC\n");

    while (1) {
        printf("test %d %d\n", ADC_ctx.adc[0],ADC_ctx.adc[1]);
        _delay_ms(100);
    }
    return 0;
}
