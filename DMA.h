#ifndef __DMA_H
#define __DMA_H

#include <stdint.h>

void dma_init(uint8_t channel, uint8_t trig);
void dma_copy_buffer(uint8_t channel, uint32_t *start, uint32_t *dest);

#endif
