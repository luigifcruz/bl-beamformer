#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cuda_runtime.h>

#include "module.h"

#define SYNC_MODE 1

int main(int argc, char **argv) {
    if (argc == 3 && (cudaSetDevice(atoi(argv[2])) != 0)) {
        printf("failed to set device\n");
        return 1;
    }

    size_t number_of_workers = 2;
    blade_module_t mod = blade_initialize(number_of_workers);

    void** input_buffers = (void**)malloc(number_of_workers * sizeof(void*));
    void** output_buffers = (void**)malloc(number_of_workers * sizeof(void*));

    for (int i = 0; i < number_of_workers; i++) {
        size_t input_byte_size = blade_get_input_size(mod) * sizeof(int8_t) * 2;
        input_buffers[i] = (void*)malloc(input_byte_size);
        blade_pin_memory(mod, input_buffers[i], input_byte_size);

        size_t output_byte_size = blade_get_output_size(mod) * sizeof(int16_t) * 2;
        output_buffers[i] = (void*)malloc(output_byte_size);
        blade_pin_memory(mod, output_buffers[i], output_byte_size);
    }

#if SYNC_MODE

    for (int i = 0; i < 255; i++) {
        blade_process(mod, input_buffers, output_buffers);
    }

#else

    int runs = 0;
    int head = 0;
    int tail = 0;
    while(runs < 510) {
        if (blade_async_query(mod, head)) {
            blade_async_process(mod, head, input_buffers[head], output_buffers[head]);
            head = (head + 1) % batch_size;
        }

        if (tail != head && !blade_async_query(mod, tail)) {
            runs += 1;
            tail = (tail + 1) % batch_size;
        }
    }

#endif

    blade_terminate(mod);

    for (int i = 0; i < number_of_workers; i++) {
        free(input_buffers[i]);
        free(output_buffers[i]);
    }

    free(input_buffers);
    free(output_buffers);
}