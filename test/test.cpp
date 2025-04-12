#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <regex>
#include <semaphore.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

using namespace std;

// TODO Make tests
TEST(Test, BasicTest) {
  EXPECT_EQ(1, 1) << "Make sure to initialize the account balance to 0";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
