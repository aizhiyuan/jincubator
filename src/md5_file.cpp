#include "md5_file.h"

//----------------------------------------------------------------------------------------------------------
// 计算文件的MD5哈希值，并以字符串形式返回
//----------------------------------------------------------------------------------------------------------
int compute_md5_file(const char *filename, char outputBuffer[33])
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    unsigned char buffer[1024];
    MD5_CTX context;
    FILE *file;
    size_t bytesRead;

    // 打开文件
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return -1;
    }

    // 初始化MD5上下文
    MD5_Init(&context);

    // 读取文件内容并更新MD5上下文
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        MD5_Update(&context, buffer, bytesRead);
    }

    // 完成哈希计算
    MD5_Final(digest, &context);

    // 关闭文件
    fclose(file);

    // 将哈希值转换为字符串
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        sprintf(&outputBuffer[i * 2], "%02x", (unsigned int)digest[i]);
    }
    // 添加字符串终止符
    outputBuffer[32] = '\0';

    return 0;
}

//----------------------------------------------------------------------------------------------------------
// 计算输入字符串的MD5哈希值，并以字符串形式返回
//----------------------------------------------------------------------------------------------------------
void compute_md5(const char *str, char outputBuffer[33])
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX context;

    // 初始化MD5上下文
    MD5_Init(&context);
    // 更新MD5上下文
    MD5_Update(&context, str, strlen(str));
    // 完成哈希计算
    MD5_Final(digest, &context);

    // 将哈希值转换为字符串
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        sprintf(&outputBuffer[i * 2], "%02x", (unsigned int)digest[i]);
    }
    // 添加字符串终止符
    outputBuffer[32] = '\0';
}