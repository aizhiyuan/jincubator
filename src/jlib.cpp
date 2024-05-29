#include "jlib.h"
#define A 0
#define B 1
#define C 2
#define D 3

//----------------------------------------------------------------------------------------------------------
// 延迟函数
//----------------------------------------------------------------------------------------------------------
void wait_ms(unsigned int v_w_time)
{
  struct timeval tv;
  tv.tv_sec = v_w_time / 1000;
  tv.tv_usec = (v_w_time % 1000) * 1000;
  select(0, NULL, NULL, NULL, &tv);
  return;
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