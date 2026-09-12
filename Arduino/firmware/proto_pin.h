#ifndef PROTO_PIN_H
#define PROTO_PIN_H

#include <stdint.h>

void proto_pin_begin(void);
void proto_pin_poll(uint32_t now_ms);

#endif
