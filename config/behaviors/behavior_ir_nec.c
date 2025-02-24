#include "behavior_ir_nec.h"
#include "../drivers/ir_nec/ir_nec.h"
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_ir_nec_data {};

struct behavior_ir_nec_config {
    uint32_t code;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    const struct device *ir_dev = DEVICE_DT_GET(DT_NODELABEL(ir_nec));
    
    if (!device_is_ready(ir_dev)) {
        return -ENODEV;
    }
    
    ir_nec_send_code(ir_dev, binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ir_nec_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = NULL,
};

static int behavior_ir_nec_init(const struct device *dev) {
    return 0;
}

ZMK_BEHAVIOR_DEFINE(behavior_ir_nec, behavior_ir_nec_init, NULL,
                   &behavior_ir_nec_driver_api);
