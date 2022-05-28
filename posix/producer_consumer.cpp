#include "producer_consumer.h"

#include <unistd.h>
#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <vector>

bool isDebug = false;
std::atomic<bool> prd{false};

pthread_cond_t CONSUMER = PTHREAD_COND_INITIALIZER;
pthread_cond_t PRODUICER = PTHREAD_COND_INITIALIZER;

pthread_mutex_t ARG = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t SUM = PTHREAD_MUTEX_INITIALIZER;

uint getSleepTime(uint max) {
  srand(time(nullptr));
  int sleep_time = 0;
  if (max) {
    sleep_time = (rand() % max + 1) * 1000;
  }
  return sleep_time;
}

uint getTid() {
  static uint nums = 1;
  thread_local std::unique_ptr<uint> Tid;
  if (Tid == nullptr) {
    Tid = std::make_unique<uint>(nums++);
  }
  return *Tid;
}

void* producerRoutine(void* args) {
  auto prod = static_cast<Producer*>(args);
  while (true) {
    pthread_mutex_lock(&ARG);
    if (!(std::cin >> *prod->arg)) {
      prd.store(false, std::memory_order_relaxed);
      pthread_mutex_unlock(&ARG);
      break;
    }
    pthread_cond_signal(&CONSUMER);
    pthread_cond_wait(&PRODUICER, &ARG);
    pthread_mutex_unlock(&ARG);
  }
  sleep(1);

  pthread_mutex_lock(&ARG);
  pthread_cond_broadcast(&CONSUMER);
  pthread_mutex_unlock(&ARG);
  return nullptr;
}

void* consumerRoutine(void* args) {
  auto* con = static_cast<Consumer*>(args);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  thread_local int result = 0;

  while (true) {
    pthread_mutex_lock(&ARG);
    if (*con->arg == 0) pthread_cond_wait(&CONSUMER, &ARG);
    if (!prd.load(std::memory_order_relaxed)) {
      pthread_mutex_unlock(&ARG);
      break;
    }
    result = result + *con->arg;
    *con->arg = 0;
    pthread_cond_signal(&PRODUICER);
    if (isDebug)
      std::cout << "(" << getTid() << ", " << result << ")" << std::endl;
    pthread_mutex_unlock(&ARG);
    uint rst = getSleepTime(*con->sleepTime);
    usleep(rst);
  }
  pthread_mutex_lock(&SUM);
  *con->sum += result;
  pthread_mutex_unlock(&SUM);
  return nullptr;
}

void* consumerInterruptorRoutine(void* args) {
  auto con = static_cast<ConsumerInterruptor*>(args);
  while (prd.load(std::memory_order_relaxed)) {
    int Tid = rand() % con->count;
    pthread_cancel(con->threads[Tid]);
    usleep(1);
  }
  return nullptr;
}

int runThreads(uint count, uint max, bool enableDebug) {
  if (enableDebug) {
    isDebug = true;
  }
  int arg = 0;
  int sum = 0;
  pthread_t prod;
  auto pa = Producer{&arg};
  pthread_create(&prod, nullptr, producerRoutine, &pa);
  prd.store(true, std::memory_order_relaxed);
  std::vector<pthread_t> consumerThreads(count);
  Consumer consumerArgs{};
  for (uint i = 0; i < count; ++i) {
    consumerArgs = Consumer{&max, &arg, &sum};
    pthread_create(&consumerThreads[i], nullptr, consumerRoutine,
                   &consumerArgs);
  }
  pthread_t interruptor;
  auto ia = ConsumerInterruptor{consumerThreads.size(), consumerThreads.data()};
  pthread_create(&interruptor, nullptr, consumerInterruptorRoutine, &ia);
  pthread_join(prod, nullptr);
  for (uint i = 0; i < count; ++i) pthread_join(consumerThreads[i], nullptr);
  pthread_join(interruptor, nullptr);
  pthread_cond_destroy(&CONSUMER);
  pthread_cond_destroy(&PRODUICER);
  pthread_mutex_destroy(&ARG);
  pthread_mutex_destroy(&SUM);
  return sum;
}
