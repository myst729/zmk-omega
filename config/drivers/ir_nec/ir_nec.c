#include "ir_nec.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define DT_DRV_COMPAT zmk_ir_nec

#define NEC_CARRIER_FREQ 38000
#define NEC_HDR_MARK 9000
#define NEC_HDR_SPACE 4500
#define NEC_BIT_MARK 560
#define NEC_ONE_SPACE 1690
#define NEC_ZERO_SPACE 560

struct ir_nec_config {
    struct gpio_dt_spec ir_pin;
};

struct ir_nec_data {
    const struct device *dev;
    struct k_timer carrier_timer;
    bool carrier_on;
};

static void carrier_timer_handler(struct k_timer *timer)
{
    struct ir_nec_data *data = CONTAINER_OF(timer, struct ir_nec_data, carrier_timer);
    const struct ir_nec_config *config = data->dev->config;
    
    data->carrier_on = !data->carrier_on;
    gpio_pin_set_dt(&config->ir_pin, data->carrier_on);
}

static void send_carrier(const struct device *dev, uint32_t time_us)
{
    struct ir_nec_data *data = dev->data;
    k_timer_start(&data->carrier_timer, K_USEC(13), K_USEC(13));
    k_busy_wait(time_us);
    k_timer_stop(&data->carrier_timer);
}

static void send_space(const struct device *dev, uint32_t time_us)
{
    const struct ir_nec_config *config = dev->config;
    gpio_pin_set_dt(&config->ir_pin, 0);
    k_busy_wait(time_us);
}

void ir_nec_send_code(const struct device *dev, uint32_t code)
{
    send_carrier(dev, NEC_HDR_MARK);
    send_space(dev, NEC_HDR_SPACE);
    
    for (int i = 31; i >= 0; i--) {
        send_carrier(dev, NEC_BIT_MARK);
        send_space(dev, (code & BIT(i)) ? NEC_ONE_SPACE : NEC_ZERO_SPACE);
    }
    
    send_carrier(dev, NEC_BIT_MARK);
}

static int ir_nec_init(const struct device *dev)
{
    const struct ir_nec_config *config = dev->config;
    struct ir_nec_data *data = dev->data;
    
    if (!device_is_ready(config->ir_pin.port)) {
        return -ENODEV;
    }
    
    data->dev = dev;
    gpio_pin_configure_dt(&config->ir_pin, GPIO_OUTPUT_INACTIVE);
    k_timer_init(&data->carrier_timer, carrier_timer_handler, NULL);
    
    return 0;
}

#define IR_NEC_INIT(n) \
    static const struct ir_nec_config ir_nec_config_##n = { \
        .ir_pin = GPIO_DT_SPEC_INST_GET(n, ir_gpios), \
    }; \
    static struct ir_nec_data ir_nec_data_##n; \
    DEVICE_DT_INST_DEFINE(n, ir_nec_init, NULL, \
                         &ir_nec_data_##n, \
                         &ir_nec_config_##n, POST_KERNEL, \
                         CONFIG_SENSOR_INIT_PRIORITY, \
                         NULL);

DT_INST_FOREACH_STATUS_OKAY(IR_NEC_INIT)
