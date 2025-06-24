#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"

#include <i2c_fifo.h>
#include <i2c_slave.h>

static uint8_t next = 0xfe;

static const uint I2C_SLAVE_ADDRESS = 0x27;

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 500
#endif

// Perform initialisation
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

static bool update_led_cb(repeating_timer_t* rt)
{
    static bool led_state = false;
    led_state = !led_state;
    pico_set_led(led_state);
}

static void recv_spi()
{
    uint8_t r;
    spi_read_blocking(spi1, 0, &r, 1);
    printf("> %02X\n", &r);
    next = r + 1;
    spi_write_blocking(spi1, &next, 1);
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


static void i2c_handle(i2c_inst_t* i2c, i2c_slave_event_t event)
{
    printf("I2C\n");
}


static void setup_i2c_slave()
{
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


int main()
{
    stdio_uart_init();

    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

    spi_slave_init();
    // setup_i2c_slave();

    repeating_timer_t timer;
    add_repeating_timer_ms(LED_DELAY_MS, update_led_cb, NULL, &timer);

    while (true) {
        printf("Hello world\n");
        sleep_ms(1000);
    }
}
