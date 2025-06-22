#ifndef CONFIG_H
#define CONFIG_H

#include <sys/stat.h>

#define DIR_MODE   0755  // rwxr-xr-x (目录权限)
#define EXEC_MODE  0755  // rwxr-xr-x (可执行文件权限)
#define CONF_MODE  0644  // rw-r--r-- (配置文件权限)

// 函数声明
int ensure_directory_exists(const char *path);
int write_file(const char *filename, const char *content, mode_t mode);
int write_jlog_config(void);
int write_rc_local(void);
int write_update_script(void);
int create_directories(void);

int check_rc_local_contains_update(void);  // 新增函数声明
int check_update_script_contains_jlog(void);   // 新增：检查update.sh是否包含jlog.conf
int check_jlog_contains_jincubator(void);  // 新增函数声明

void remove_log_file(); // 删除日志
#endif // CONFIG_H