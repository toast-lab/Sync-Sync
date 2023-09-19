#include "config.h"
#include <sys/mman.h>

#define RAND_SEND

#define S_TIME 40000

static int fd = 0;
#define BUFF_SIZE 1024
static char buffer[BUFF_SIZE];
#define SEND_SIZE 8000

static int last_size = 0, cur_size = 0;
static struct timespec trans_start, trans_end;
static unsigned long trans_num_bits = 0;
static int sleep_time = S_TIME;

static bool f_write = false;
static bool f_ftruncate = false;

static void output(int bit)
{
  unsigned long run_time = 0;
  trans_num_bits += 1;
  if (bit)
  {
    do
    {
      if (f_ftruncate)
        cur_size = rand() % (1024 * 1024 * 1024 * 5);
      else if (f_write)
        cur_size = rand() % ('~' - ' ' + 1) + ' ';
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
      fdatasync(fd);

      clock_gettime(CLOCK_MONOTONIC, &trans_end);
      run_time = ((trans_end.tv_sec - trans_start.tv_sec) * 1000000000 + (trans_end.tv_nsec - trans_start.tv_nsec));

    } while (run_time + 10000 < trans_num_bits * sleep_time);
  }

  clock_gettime(CLOCK_MONOTONIC, &trans_end);
  run_time = ((trans_end.tv_sec - trans_start.tv_sec) * 1000000000 + (trans_end.tv_nsec - trans_start.tv_nsec));

  long int sleep_for_0 = trans_num_bits * sleep_time - run_time;
  if (sleep_for_0 > 0)
    usleep(sleep_for_0 / 1000);

}

#define TRACE_FILE "../bits.trace"
#define TRACE_SIZE 8001

// sleep time, type (0: only, 1: write, 2: ftruncate), tro path, trace line

int main(int argc, char const *argv[])
{

  if (argc > 1)
  {
    sleep_time = atoi(argv[1]);
  }

  int type = 0;

  if (argc > 2)
    type = atoi(argv[2]);

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

  char *tro_filepath = TRO_FILE;
  if (argc > 3)
    tro_filepath = argv[3];

  int trace_line = 0;
  if (argc > 4)
  {
    trace_line = atoi(argv[4]);
  }
#ifdef RAND_SEND
  int trace_fd = 0;
  char *trace_str = NULL;

  trace_fd = open(TRACE_FILE, O_RDONLY);
  if (trace_fd <= 0)
  {
    printf("open trace Error no.%d: %s\n", errno, strerror(errno));
    return -1;
  }
  trace_str = (char *)mmap(NULL, 10 * TRACE_SIZE, PROT_READ,
                           MAP_SHARED | MAP_POPULATE, trace_fd, 0);
  if (trace_str == MAP_FAILED)
  {
    printf("mmap trace Error no.%d: %s\n", errno, strerror(errno));
    return -1;
  }
#endif

  printf("Hello, sender! time: %d ns %d %s %d\n", sleep_time, type, tro_filepath, trace_line);
  srand(0);

  fd = open(tro_filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd <= 0)
  {
    printf("open Error no.%d: %s\n", errno, strerror(errno));
    assert(fd <= 0);
    return -1;
  }

  if (f_write)
  {
    memset(buffer, cur_size, BUFF_SIZE);
    pwrite(fd, buffer, BUFF_SIZE, 0);
  }

  fsync(fd);

  sleep(2);

  clock_gettime(CLOCK_REALTIME, &start);
  clock_gettime(CLOCK_MONOTONIC, &trans_start);

#ifdef RAND_SEND
  for (int i = 0; i < SEND_SIZE; ++i)
  {
    output(trace_str[trace_line * TRACE_SIZE + i] == '1');
  }
#else
  for (int i = 1; i <= SEND_SIZE; ++i)
  {
    output(i % 2);
  }
#endif

  clock_gettime(CLOCK_REALTIME, &end);
  clock_gettime(CLOCK_MONOTONIC, &trans_end);

  printf("prefix, end:\n%lld, %lld\n",
         //  (trans_start.tv_sec * 1000000000 +  trans_start.tv_nsec),
         //  (trans_end.tv_sec * 1000000000 +  trans_end.tv_nsec));
         (start.tv_sec * 1000000000 + start.tv_nsec),
         (end.tv_sec * 1000000000 + end.tv_nsec));

  long long int times = (end.tv_sec - start.tv_sec) * 1000000000 
                      + (end.tv_nsec - start.tv_nsec);
  printf("send real times and num: %lld, %lld\n", times, times / sleep_time);

#ifdef RAND_SEND
  munmap(trace_str, 10 * TRACE_SIZE);
  close(trace_fd);
#endif
  close(fd);
  return 0;
}
