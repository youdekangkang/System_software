#include <iostream>
#include <string>
#include "producer_consumer.h"
bool debugMode = false;
int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "Insufficient Parameters" << std::endl;
    return 1;
  }
  if (argc == 4 && std::string{argv[3]} == "-debug") debugMode = true;
  uint threadsCount = atoi(argv[1]);
  uint maxSleepTime = atoi(argv[2]);
  std::cout << runThreads(threadsCount, maxSleepTime, debugMode) << std::endl;
  return 0;
}