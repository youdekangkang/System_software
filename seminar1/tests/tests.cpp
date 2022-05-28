#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <create_cancel.h>
#include <doctest.h>
#include <chrono>

TEST_CASE("task1_combinations") {
  REQUIRE(c(0, 0) == 1);
  REQUIRE(c(1000000, 1000000) == 1);
  REQUIRE(c(2, 3) == 3);
}

TEST_CASE("task1_performance") {
  std::chrono::steady_clock::time_point serial_begin =
      std::chrono::steady_clock::now();
  REQUIRE(serial_sum() == 2203961430);
  std::chrono::steady_clock::time_point serial_end =
      std::chrono::steady_clock::now();

  std::chrono::steady_clock::time_point parallel_begin =
      std::chrono::steady_clock::now();
  REQUIRE(parallel_sum() == 2203961430);
  std::chrono::steady_clock::time_point parallel_end =
      std::chrono::steady_clock::now();
  REQUIRE(serial_end - serial_begin > parallel_end - parallel_begin);
}

TEST_CASE("task2_join") {
  pthread_t tid_a = create_thread_a();
  pthread_t tid_b = create_thread_b(&tid_a);
  REQUIRE(tid_a != pthread_t{});
  pthread_join(tid_a, nullptr);
  pthread_join(tid_b, nullptr);
}

TEST_CASE("task3_immortal") {
  pthread_t t1 = create_immortal_thread();
  pthread_cancel(t1);
  REQUIRE(t1 != pthread_t{});
  struct timespec time {
    1, 0
  };
  REQUIRE(pthread_timedjoin_np(t1, nullptr, &time) != 0);
}

TEST_CASE("task4_signal") {
  pthread_t t1 = create_read_values_thread();
  REQUIRE(t1 != pthread_t{});
  pthread_cancel(t1);
  pthread_kill(t1, SIGTERM);
  pthread_join(t1, nullptr);
}
