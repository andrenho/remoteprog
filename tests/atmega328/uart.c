#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>

static int uart_putchar(char c, FILE* f)
{
    (void) f;

    if (c == '\n')
        uart_putchar('\r', f);
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

static int uart_getchar(FILE* f)
{
    (void) f;

    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void uart_init(void)
{
    _delay_ms(100);
    
    // set speed
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // set config
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);   // Async-mode 
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);     // Enable Receiver and Transmitter

    static FILE uart = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
    stdin = stdout = &uart;

    _delay_ms(100);
}
