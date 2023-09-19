#ifndef FSYNC_CHANNEL_CONFIG_H
#define FSYNC_CHANNEL_CONFIG_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define __USE_GNU
#include <sched.h>

int set_cpu(int core_num) {
  cpu_set_t mask;

  CPU_ZERO(&mask);
  CPU_SET(core_num, &mask);

  int ret = sched_setaffinity(0, sizeof(mask), &mask);
  if (ret == -1)
  {
    printf("warning: could not set CPU affinity %d\n", errno);
  }

  return ret;
}

#define SPY_FILE "./ext4/spy.txt"

#define TRO_FILE "./ext4/tro.txt"

#define str_len_ 1048576
static char str_data_[str_len_];
static int str_pos_;

char* generate(int len) {
  if (str_pos_ + len > str_len_) {
    str_pos_ = 0;
  }
  str_pos_ += len;
  return str_data_ + str_pos_ - len;
}

void rand_init_str() {
  srand(0);
  for (size_t i = 0; i < str_len_; i++)
  {
    str_data_[i] = 32 + rand() % (126 - 32 + 1);
  }
}



struct timespec start, end, tmp;


#endif //FSYNC_CHANNEL_CONFIG_H
