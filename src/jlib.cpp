#include "jlib.h"
#define A 0
#define B 1
#define C 2
#define D 3

//----------------------------------------------------------------------------------------------------------
// 延迟函数
//----------------------------------------------------------------------------------------------------------
void wait_ms(long v_w_time)
{
  struct timeval tv;
  tv.tv_sec = v_w_time / 1000;
  tv.tv_usec = (v_w_time % 1000) * 1000;
  select(0, NULL, NULL, NULL, &tv);
  return;
}

void delay_ms(long v_ul_time)
{
  struct timespec req, rem;
  req.tv_sec = v_ul_time / 1000;              // 毫秒转化为秒
  req.tv_nsec = (v_ul_time % 1000) * 1000000; // 毫秒转化为纳秒

  while (nanosleep(&req, &rem) == -1)
  {
    if (errno == EINTR)
    {
      // 如果被信号中断，继续剩余的时间
      req = rem;
    }
    else
    {
      perror("nanosleep");
      break;
    }
  }
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 BA
//----------------------------------------------------------------------------------------------------------
void *ba(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是2的倍数或者小于等于0，返回空数据
  if (len % 2 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 2;
  while (len--)
  {
    // AB交换
    *(uc_addr + count + A) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + B);
    count += 2;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 ABDC
//----------------------------------------------------------------------------------------------------------
void *abdc(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // CD交换
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 BACD
//----------------------------------------------------------------------------------------------------------
void *bacd(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AB交换
    *(uc_addr + count + A) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + B);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 BADC
//----------------------------------------------------------------------------------------------------------
void *badc(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AB交换
    *(uc_addr + count + A) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + B);

    // CD交换
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 CDAB
//----------------------------------------------------------------------------------------------------------
void *cdab(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AC交换
    *(uc_addr + count + A) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + C);

    // BD交换
    *(uc_addr + count + B) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + D);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 CDBA
//----------------------------------------------------------------------------------------------------------
void *cdba(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AC交换
    *(uc_addr + count + A) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + C);

    // BD交换
    *(uc_addr + count + B) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + D);

    // CD交换
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 DCAB
//----------------------------------------------------------------------------------------------------------
void *dcab(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AD交换
    *(uc_addr + count + A) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + D);

    // BC交换
    *(uc_addr + count + B) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + C);

    // CD交换
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + D);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 数据交换 DCBA
//----------------------------------------------------------------------------------------------------------
void *dcba(void *buf, int len)
{
  unsigned char *uc_addr = (unsigned char *)buf;
  int count = 0;
  // 数据长度不是4的倍数或者小于等于0，返回空数据
  if (len % 4 != 0 || len <= 0)
  {
    return NULL;
  }
  len /= 4;
  while (len--)
  {
    // AD交换
    *(uc_addr + count + A) ^= *(uc_addr + count + D);
    *(uc_addr + count + D) ^= *(uc_addr + count + A);
    *(uc_addr + count + A) ^= *(uc_addr + count + D);

    // BC交换
    *(uc_addr + count + B) ^= *(uc_addr + count + C);
    *(uc_addr + count + C) ^= *(uc_addr + count + B);
    *(uc_addr + count + B) ^= *(uc_addr + count + C);
    count += 4;
  }
  return uc_addr;
}

//----------------------------------------------------------------------------------------------------------
// 解析文件路径
//----------------------------------------------------------------------------------------------------------
void parse_file_path(char *path, char **filename)
{
  // 使用strrchr函数查找路径中的最后一个斜杠
  char *lastSlash = strrchr(path, '/');

  // 如果找到了斜杠，将其后面的内容作为文件名
  if (lastSlash != NULL)
  {
    // 将文件名指针后移一个字符，去掉斜杠
    *filename = lastSlash + 1;
  }
  else
  {
    // 如果未找到斜杠，则整个字符串都是文件名
    *filename = (char *)path;
  }
}

//----------------------------------------------------------------------------------------------------------
// CO2验证数据位
//----------------------------------------------------------------------------------------------------------
unsigned short co2_check(unsigned char *data, int len)
{
  unsigned short v_us_co2_check_tmp = 0;
  int v_i_loop;
  for (v_i_loop = 0; v_i_loop < len; v_i_loop++)
  {
    v_us_co2_check_tmp += *(data + v_i_loop);
  }
  v_us_co2_check_tmp = ~v_us_co2_check_tmp;
  return (v_us_co2_check_tmp + 1) % 256;
}

//----------------------------------------------------------------------------------------------------------
// 时间格式化输出
//----------------------------------------------------------------------------------------------------------
void print_current_time(char *buf, size_t len)
{
  memset(buf, 0, len);
  struct timeval tv;
  gettimeofday(&tv, NULL);

  struct tm *tm_info;
  tm_info = localtime(&tv.tv_sec);
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  // 追加毫秒
  sprintf(buf, "%s.%03ld", buffer, tv.tv_usec / 1000);
}

//----------------------------------------------------------------------------------------------------------
// 定时器函数
//----------------------------------------------------------------------------------------------------------
int set_custom_timer_ms(int milliseconds, TimerCallback callback)
{
  struct sigaction sa;
  sa.sa_handler = callback;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);

  int seconds = milliseconds / 1000;
  int microseconds = (milliseconds % 1000) * 1000;

  struct itimerval new_value, old_value;
  new_value.it_value.tv_sec = seconds;
  new_value.it_value.tv_usec = microseconds;
  new_value.it_interval.tv_sec = seconds;
  new_value.it_interval.tv_usec = microseconds;

  if (setitimer(ITIMER_REAL, &new_value, &old_value) == -1)
  {
    return -1;
  }

  return 0;
}

//----------------------------------------------------------------------------------------------------------
// 设置系统时间的函数
//----------------------------------------------------------------------------------------------------------
int set_system_time(time_t timestamp)
{
  // 将时间戳转换为struct tm结构
  struct tm *timeinfo = gmtime(&timestamp);

  // 设置系统时间
  struct timeval tv;
  tv.tv_sec = timestamp;
  tv.tv_usec = 0;

  // 设置系统时间，需要超级用户权限
  if (settimeofday(&tv, NULL) == 0)
  {
    return 0; // 成功
  }
  else
  {
    return -1; // 失败
  }
}

//----------------------------------------------------------------------------------------------------------
// 定时器函数，接受时间戳参数，返回Time结构体
//----------------------------------------------------------------------------------------------------------
struct Time timer(time_t timestamp)
{
  struct Time result;

  // 计算天数、小时和分钟
  result.days = timestamp / (24 * 3600);
  timestamp %= (24 * 3600);
  result.hours = timestamp / 3600;
  timestamp %= 3600;
  result.minutes = timestamp / 60;

  return result;
}

//----------------------------------------------------------------------------------------------------------
// PID 过滤函数
//----------------------------------------------------------------------------------------------------------
int pid_filter(int pid, int *p, unsigned char num)
{
  unsigned char i;
  int sum;
  short ave;

  for (i = (num - 1); i > 0; i--)
  {
    p[i] = p[i - 1];
  }
  p[0] = pid;

  sum = 0;
  for (i = 0; i < num; i++)
  {
    sum += p[i];
  }
  ave = (sum / num);
  return (ave);
}

unsigned short get_sync_time(int pv, int sp, int p, int sycle, int rate)
{
  unsigned short v_us_time = 0;
  float v_s_en = abs(pv - sp);
  float v_s_p_2 = p / 2;
  float v_f_prop = (v_s_en / v_s_p_2) * sycle;
  v_f_prop = v_f_prop * 2 * rate / 100;
  v_us_time = (v_f_prop < 0) ? 0 : v_f_prop;
  return v_us_time;
}