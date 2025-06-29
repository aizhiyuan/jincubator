#ifndef __MD5_FILE_H__
#define __MD5_FILE_H__
#include "../include/openssl/md5.h"
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------------------------------------
// 计算文件的MD5哈希值，并以字符串形式返回
//----------------------------------------------------------------------------------------------------------
int compute_md5_file(const char *filename, char outputBuffer[33]);

//----------------------------------------------------------------------------------------------------------
// 计算输入字符串的MD5哈希值，并以字符串形式返回
//----------------------------------------------------------------------------------------------------------
void compute_md5(const char *str, char outputBuffer[33]);

#endif //__MD5_FILE_H__