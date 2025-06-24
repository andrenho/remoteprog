#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"

#include <i2c_fifo.h>
#include <i2c_slave.h>

static uint8_t data[256];
static uint8_t current = 0;

// 
// LED
// 

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

static void pico_led_init()
{
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_init();
#endif
}

static bool update_led_cb(repeating_timer_t* rt)
{
    static bool led_state = false;
    led_state = !led_state;
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_put(PICO_DEFAULT_LED_PIN, led_state);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
#endif
}

//
// SPI
// 

static void recv_spi()
{
    while (!spi_is_readable(spi1) && !spi_is_writable(spi1)) {}

    uint8_t r;
    spi_write_read_blocking(spi1, &data[current++], &r, 1);
}

static void spi_slave_init()
{
    spi_init(spi1, 1000 * 1000);
    spi_set_format(spi1, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_set_slave(spi1, true);

    gpio_set_function(10, GPIO_FUNC_SPI);
    gpio_set_function(11, GPIO_FUNC_SPI);
    gpio_set_function(12, GPIO_FUNC_SPI);
    gpio_set_function(13, GPIO_FUNC_SPI);

    spi1_hw->imsc = SPI_SSPIMSC_RTIM_BITS | SPI_SSPIMSC_RXIM_BITS;

    irq_set_enabled(SPI1_IRQ, true);
    irq_set_exclusive_handler(SPI1_IRQ, recv_spi);
}

//
// I2C
//

static void i2c_handle(i2c_inst_t* i2c, i2c_slave_event_t event)
{
    switch (event) {
        case I2C_SLAVE_RECEIVE:  // master has written data
            i2c_read_byte(i2c);
            break;
        case I2C_SLAVE_REQUEST:  // master requests data
            i2c_write_byte(i2c, data[current++]);
            break;
        case I2C_SLAVE_FINISH:   // master has signalled stop
            break;
    }
}


static void setup_i2c_slave()
{
    static const uint I2C_SLAVE_ADDRESS = 0x27;

    // https://github.com/vmilea/pico_i2c_slave/blob/master/example_mem/example_mem.c
    gpio_init(4);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_pull_up(4);

    gpio_init(5);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(5);

    i2c_init(i2c0, 100000);
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, i2c_handle);
}

//
// main
// 

int main()
{
    for (size_t i = 0; i < 256; ++i)
        data[i] = i;

    stdio_uart_init();
    pico_led_init();
    spi_slave_init();
    setup_i2c_slave();

    repeating_timer_t timer;
    add_repeating_timer_ms(500, update_led_cb, NULL, &timer);

    printf("Hello world\n");

    while (true)
        tight_loop_contents();  // everything happens through callbacks
}
