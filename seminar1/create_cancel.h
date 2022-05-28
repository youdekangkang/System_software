#pragma once

#include <pthread.h>
#include <cstdint>

// 1. Task
uint64_t c(uint64_t m, uint64_t n);

uint64_t serial_sum();

uint64_t parallel_sum();

// 2. Task
pthread_t create_thread_a();

pthread_t create_thread_b(pthread_t* tid_b);

// 3. Task
pthread_t create_immortal_thread();

// 4. Task
pthread_t create_read_values_thread();
