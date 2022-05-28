#include "create_cancel.h"
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>

#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)

// 1. Task
//    Implement the recursive formula for the number of combinations c(m, n) =
//    c(m-1, n-1) + c(m, n-1); c(0,n)=c(n, n)=1, m <= n. Show that two parallel
//    calls to this function will run slower than two serial calls on big data

/*
Implement the function of counting the number of combinations
https://en.wikipedia.org/wiki/Combination
*/
uint64_t c(uint64_t m, uint64_t n) {
  UNUSED(n);
  UNUSED(m);
  if (m == n or m == 0 or n == 0) {
    return 1;
  }
  return c(m - 1, n - 1) + c(m, n - 1);
}

/*
Implement the wrapper function over the number of combinations function
implemented above, which you will need to use in the pthread_create
1. You can pass arguments n, m through a helper structure that can be extracted
from the variable arg and converted to a structure.
https://www.tutorialspoint.com/cprogramming/c_structures.htm
https://stackoverflow.com/questions/11626786/what-does-void-mean-and-how-to-use-it
2. Call the function c implemented above with arguments n m which are extracted
from arg
3. Allocate memory in a heap size unit64_t
https://www.tutorialspoint.com/c_standard_library/c_function_malloc.htm
4. Record the result of the number of combinations in the allocated memory
https://www.tutorialspoint.com/cprogramming/c_pointers.htm
5. Return the pointer to the allocated memory
*/

struct comb {
  uint64_t m;
  uint64_t n;
};

// define the struct of the thread
// define the c function in the posix_c

static void* posix_c(void* arg) {
  UNUSED(arg);
  uint64_t m = ((struct comb*)arg)->m;
  uint64_t n = ((struct comb*)arg)->n;
  uint64_t res = c(m, n);

  uint64_t* pointer = (uint64_t*)malloc(sizeof(uint64_t));
  *pointer = res;
  return pointer;
}

/*
1. Call the function c implemented above with arguments 15 33
2. Call the function c implemented above with arguments 16 33
3. Return the sum of the results of the functions called: c(15, 33) + c(16, 33)
*/

uint64_t serial_sum() { return c(15, 33) + c(16, 33); }

/*
Using the auxiliary function posix_c, count c(15, 33) and c(16, 33) in parallel
and return the result of the sum
1. Create a thread using pthread_create and pass there posix_c with argmuments
n=15, m=33
https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-pthread-create-create-thread
2. Create a thread using pthread_create and pass there posix_c with argmuments
n=16, m=33
3. Wait for the threads to finish using pthread_join
https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-pthread-join-wait-thread-end
4. Save the sum of the results counted in the threads to a local variable
5. Free up memory using free
https://www.tutorialspoint.com/c_standard_library/c_function_free.htm
6. Return the value of the sum recorded in the local variable
*/

// define the process of the thread counting
uint64_t parallel_sum() {
  UNUSED(posix_c);

  pthread_t t1, t2;

  struct comb comb1 = {15, 33};
  struct comb comb2 = {16, 33};
  struct comb* pointer1 = &comb1;
  struct comb* pointer2 = &comb2;

  int res1 = pthread_create(&t1, NULL, posix_c, (void*)pointer1);
  if (res1 != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }

  int res2 = pthread_create(&t2, NULL, posix_c, (void*)pointer2);
  if (res2 != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }

  void* comb_res1;
  void* comb_res2;
  int res3 = pthread_join(t1, &comb_res1);
  if (res3 != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }
  int res4 = pthread_join(t2, &comb_res2);
  if (res4 != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }

  uint64_t res = *((uint64_t*)comb_res1) + *((uint64_t*)comb_res2);
  free(comb_res1);
  free(comb_res2);
  return res;
}

static void* callback_a(void*) {
  sleep(1000);
  return nullptr;
}

/*
1. Create a thread in which you can simply call callback_a function. With use
function pthred_create
2. Return the pthread_t structures of the created thread
*/
pthread_t create_thread_a() {
  UNUSED(callback_a);
  pthread_t t{};
  int res = pthread_create(&t, NULL, callback_a, NULL);
  if (res != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }
  return t;
}

/*
callback_b takes the pthread_t structure of the thread a to the arg and then
calls the pthread_cancel
https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-pthread-cancel-cancel-thread
*/
static void* callback_b(void* arg) {
  UNUSED(arg);

  pthread_t* t = (pthread_t*)arg;

  int res = pthread_cancel(*t);
  if (res != 0) {
    fprintf(stderr, "ERROR\n");
    return nullptr;
  }

  return nullptr;
}

/*
1. Create a thread in which you can simply call callback_b function with
thread_a_id argument. With use function pthred_create
2. Return the pthread_t structures of the created thread
*/

pthread_t create_thread_b(pthread_t* thread_a_id) {
  UNUSED(thread_a_id);
  UNUSED(callback_b);

  pthread_t t;
  int res = pthread_create(&t, NULL, callback_b, thread_a_id);
  if (res != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }
  return t;
}

static void* callback_immortal(void*) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  while (1) {
    sleep(1);
  }
  return nullptr;
}

/*
1. Create a thread in which you can simply call callback_immortal function. With
use function pthred_create
2. Return the pthread_t structures of the created thread
*/

pthread_t create_immortal_thread() {
  UNUSED(callback_immortal);
  pthread_t t;
  int res = pthread_create(&t, NULL, callback_immortal, NULL);
  if (res != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }
  return t;
}

// =================================================================================

// 4. Task
//    Create a thread A that reads numbers separated by a space from the file,
//    and if the SIGINT/SIGTERM signal comes to the process, then the thread
//    terminates and correctly releases resources (look in the direction of
//    cleanup_push, cleanup_pop)

/*
Signal handler that executes thread stop logic
*/
void sig_handler(int) { pthread_exit(0); }

static void* callback_read_values(void*) {
  // 1. Register the signal handler for SIGINT/SIGTERM
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  // 2.Open the file in.txt
  FILE* file = fopen("/tests/in.txt", "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR\n");
    return nullptr;
  }

  // 3. Infinitely read from the file
  while (1) {
    fseek(file, 0, SEEK_SET);
    fgetc(file);
  }

  // 4. Close and leave
  fclose(file);
  return nullptr;
}

/*
1. Create a thread in which you can simply call callback_read_values function.
With use function pthred_create
2. Return the pthread_t structures of the created thread
*/

pthread_t create_read_values_thread() {
  UNUSED(sig_handler);
  UNUSED(callback_read_values);
  pthread_t t;
  int res = pthread_create(&t, NULL, callback_read_values, NULL);
  if (res != 0) {
    fprintf(stderr, "ERROR\n");
    return -1;
  }
  return t;
}
