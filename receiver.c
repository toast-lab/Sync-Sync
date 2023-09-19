#include "config.h"

#define CPU 8
static int fd = 0;
#define BUFF_SIZE 1024
static char buffer[BUFF_SIZE];
static char *dpath = "./send.txt";

static bool f_write = false;
static bool f_ftruncate = false;

// result path, sleep time, type (0: only, 1: write, 2: ftruncate), spy path

int main(int argc, char const *argv[])
{

  char *filepath = dpath;
  if (argc > 1)
    filepath = argv[1];

  long long sleep_time = 0; // us
  if (argc > 2)
    sleep_time = atoll(argv[2]);

  int type = 0;

  if (argc > 3)
    type = atoi(argv[3]);

  switch (type)
  {
  case 0:
    f_write = false;
    f_ftruncate = false;
    break;
  case 1:
    f_write = true;
    f_ftruncate = false;
    break;
  case 2:
    f_write = false;
    f_ftruncate = true;
    break;
  default:
    printf("type err\n");
    return -1;
  }

  char *spy_filepath = SPY_FILE;
  if (argc > 4)
    spy_filepath = argv[4];

  printf("Hello, receiver! %lld us %s %d %s\n", sleep_time, filepath, type, spy_filepath);
  srand(0);

  if (set_cpu(CPU) != 0)
    return -1;

  FILE *fp;
  fp = fopen(filepath, "w");
  if (fp == NULL)
  {
    printf("open output file %s err\n", filepath);
    return -1;
  }

  fd = open(spy_filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd <= 0)
  {
    printf("open Error no.%d: %s\n", errno, strerror(errno));
    assert(fd <= 0);
    return -1;
  }
  fsync(fd);
  sleep(1);
  int last_size = 0, cur_size = 0;

  struct timespec timing_start;
  long long timing_num = 1;
  clock_gettime(CLOCK_MONOTONIC, &timing_start);

  while (1)
  {
    if (f_ftruncate)
      cur_size = rand() % (1024 * 1024 * 1024 * 5);
    else if (f_write)
      cur_size = rand() % ('z' - 'a' + 1) + 'a';

    if (last_size == cur_size)
    {
      if (f_ftruncate)
        cur_size += 1024 * 8;
      else if (f_write)
        cur_size += 2;
    }
    last_size = cur_size;

    if (f_ftruncate)
      ftruncate(fd, cur_size);
    else if (f_write)
    {
      memset(buffer, cur_size, BUFF_SIZE);
      pwrite(fd, buffer, BUFF_SIZE, 0);
    }

    clock_gettime(CLOCK_REALTIME, &tmp);
    clock_gettime(CLOCK_MONOTONIC, &start);
    fdatasync(fd);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long int times = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    fprintf(fp, "%lld, %lld\n",
            // (start.tv_sec * 1000000000 +  start.tv_nsec),
            (tmp.tv_sec * 1000000000 + tmp.tv_nsec),
            times);

    if (sleep_time > 0)
    {
      clock_gettime(CLOCK_MONOTONIC, &end);
      long long int sleep_for_0 = (timing_num + 1) * sleep_time -
                    (((end.tv_sec - timing_start.tv_sec) * 1000000000 +
                      (end.tv_nsec - timing_start.tv_nsec)) / 1000);
      timing_num += 1;
      if (sleep_for_0 > 0)
        usleep(sleep_for_0);
    }
  }

  return 0;
}
