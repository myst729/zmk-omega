#pragma once

#include <zephyr/device.h>

void ir_nec_send_code(const struct device *dev, uint32_t code);
