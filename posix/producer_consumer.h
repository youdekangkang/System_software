#include <pthread.h>
#include <cstdlib>

struct Producer {
  int* arg;
};

struct Consumer {
  uint* sleepTime;
  int* arg = nullptr;
  int* sum;
};

struct ConsumerInterruptor {
  size_t count;
  pthread_t* threads;
};

uint getTid();
uint getSleepTime(uint max);

void* producerRoutine(void* args);
void* consumerRoutine(void* args);
void* consumerInterruptorRoutine(void* args);
int runThreads(uint count, uint time, bool enableDebug);
