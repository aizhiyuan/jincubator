#include "updater_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

// 确保目录存在
int ensure_directory_exists(const char *path)
{
    char *path_copy = strdup(path);
    if (!path_copy)
    {
        perror("strdup failed");
        return -1;
    }

    char *dir = dirname(path_copy);
    struct stat st;

    if (stat(dir, &st) == -1)
    {
        if (errno == ENOENT)
        {
            // 目录不存在，尝试创建
            if (mkdir(dir, DIR_MODE) == -1)
            {
                fprintf(stderr, "Failed to create directory %s: %s\n",
                        dir, strerror(errno));
                free(path_copy);
                return -1;
            }
            printf("Created directory: %s\n", dir);
        }
        else
        {
            perror("stat failed");
            free(path_copy);
            return -1;
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Path exists but is not a directory: %s\n", dir);
        free(path_copy);
        return -1;
    }

    free(path_copy);
    return 0;
}

// 安全写入文件
int write_file(const char *filename, const char *content, mode_t mode)
{
    // 确保目录存在
    if (ensure_directory_exists(filename) != 0)
    {
        return -1;
    }

    // 打开文件
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    // 写入内容
    if (fprintf(fp, "%s", content) < 0)
    {
        fprintf(stderr, "Failed to write to %s: %s\n", filename, strerror(errno));
        fclose(fp);
        return -1;
    }

    // 关闭文件
    if (fclose(fp) != 0)
    {
        perror("fclose failed");
        return -1;
    }

    // 设置权限
    if (chmod(filename, mode) == -1)
    {
        fprintf(stderr, "Failed to set permissions on %s: %s\n",
                filename, strerror(errno));
        return -1;
    }

    printf("Created file: %s (mode %04o)\n", filename, mode);
    return 0;
}

// 写入jlog配置文件
// 写入jlog配置文件
int write_jlog_config(void)
{
    const char *filename = "/root/shijing/jlog.conf";
    const char *content =
        "[global]\n"
        "#strict init = true\n"
        "buffer min = 1024\n"
        "buffer max = 2MB\n"
        "rotate lock file = /tmp/zlog.lock\n"
        "default format = \"%d.%us %-6V (%c:%F:%L) - %m%n\"\n"
        "file perms = 666\n\n"

        "[levels]\n"
        "#TRACE = 10\n"
        "#CRIT = 130, LOG_CRIT\n\n"

        "[formats]\n"
        "simple = \"%m%n\"\n"
        "normal = \"%d %m%n\"\n"
        "normalms = \"%d.%ms %m%n\"\n"
        " \n\n"

        "[rules]\n"
        "#default.*    >stdout; simple\n"
        "#*.*         \"tmp/log/%c.log\", 2MB*1; simple\n"
        "##my_.ERROR    >stderr;\n"
        "jmbtcps4pn.ERROR   \"/var/log/shijing/%c.log\",20MB*1; normalms\n"
        "jcoreio.ERROR   \"/var/log/shijing/%c.log\",20MB*1; normalms\n"
        "jhatcher.ERROR   \"/var/log/shijing/%c.log\",20MB*1; normalms\n"
        "jincubator.ERROR   \"/var/log/shijing/%c.log\",20MB*1; normalms\n"
        "jlldp.ERROR   \"/var/log/shijing/%c.log\",20MB*1; normalms\n";

    // 确保日志目录存在
    const char *log_dir = "/var/log/shijing";
    struct stat st;
    if (stat(log_dir, &st) == -1)
    {
        if (mkdir(log_dir, DIR_MODE) == -1)
        {
            fprintf(stderr, "Failed to create log directory %s: %s\n",
                    log_dir, strerror(errno));
            return -1;
        }
        printf("Created log directory: %s\n", log_dir);
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Path exists but is not a directory: %s\n", log_dir);
        return -1;
    }

    return write_file(filename, content, CONF_MODE);
}

// 写入rc.local文件
int write_rc_local(void)
{
    const char *filename = "/etc/rc.local";
    const char *content =
        "#!/bin/sh -e\n"
        "#\n"
        "# rc.local\n"
        "#\n"
        "# This script is executed at the end of each multiuser runlevel.\n"
        "# Make sure that the script will \"exit 0\" on success or any other\n"
        "# value on error.\n"
        "#\n"
        "# In order to enable or disable this script just change the execution\n"
        "# bits.\n"
        "#\n"
        "# By default this script does nothing.\n"
        "\n"
        "if [ -e /usr/bin/setqtenv ]; then\n"
        ". /usr/bin/setqtenv\n"
        "/usr/bin/lcd2usb_print \"CPU: {{CPU}}\" \"Mem: {{MEM}}\" \"IP: {{IP}}\" \"LoadAvg: {{LOADAVG}}\" 2>&1 > /dev/null&\n"
        "#/opt/QtE-Demo/run.sh&\n"
        "fi\n"
        "\n"
        "echo \"nameserver 114.114.114.114\" > /etc/resolv.conf\n"
        "\n"
        "sleep 10\n"
        "cp /root/sh/update.sh /root/tmp/update.sh\n"
        "/bin/bash /root/tmp/update.sh || true\n"
        "\n"
        "cd /root/shijing\n"
        "./jreadtempchannel &\n"
        "sleep 5\n"
        "./jcoreio &\n"
        "./jreadgpio11 &\n"
        "./jincubator &\n"
        "./jhatcher &\n"
        "sleep 5\n"
        "./jmbtcps4pn &\n"
        "./jdaemon >/dev/null &\n"
        "\n"
        "echo \"version 1.0.0\"\n"
        "exit 0\n";

    return write_file(filename, content, EXEC_MODE);
}

// 写入update.sh脚本
int write_update_script(void)
{
    const char *filename = "/root/sh/update.sh";
    const char *content =
        "#!/bin/bash\n\n"
        "MOUNT_DIR=\"/mnt\"\n"
        "PROGRAM_DIR=\"/root/shijing\"\n"
        "SH_DIR=\"/root/sh\"\n"
        "RCLOCAL_DIR=\"/etc/rc.local\"\n"
        "TMP_DIR=\"/root/tmp\"\n\n"

        "PROGRAMS=(\"jmbtcps4pn\" \"jmbtcps4pn.ini\" \"jincubator\" \"jincubator.ini\" \"jhatcher\" \"jhatcher.ini\" \"jdaemon\" \"jdaemon.ini\" \"jreadgpio11\" \"jreadtempchannel\" \"identifyusb\" \"rc.local\" \"update.sh\" \"jlog.conf\")\n"
        "MOUNTED=false\n\n"

        "[ -d $SH_DIR ] || mkdir -p $SH_DIR\n"
        "[ -d $TMP_DIR ] || mkdir -p $TMP_DIR\n\n"

        "for dev in /dev/sd[a-z][1-9] /dev/mmcblk[1-9][a-z][1-9]; do\n"
        "    if [[ $dev =~ ^/dev/sd[a-z][1-9]$ || $dev =~ ^/dev/mmcblk[1-9][a-z][1-9]$ ]]; then\n"
        "        echo \"dev:$dev\"\n"
        "    \n"
        "    filename=${dev##*/}\n"
        "    echo \"filename:$filename\"\n"
        "    \n"
        "    UPDATE_DIR=\"$MOUNT_DIR/$filename\"\n"
        "    echo \"UPDATE_DIR:$UPDATE_DIR\"\n\n"

        "        mkdir -p \"$UPDATE_DIR\"\n"
        "        mount $dev \"$UPDATE_DIR\" 2>/dev/null\n"
        "    echo \"result:$?\"\n"
        "        if [ $? -eq 0 ]; then\n"
        "            echo \"Mounted device $dev to $UPDATE_DIR\"\n"
        "            MOUNTED=true\n"
        "            break\n"
        "        fi\n"
        "    fi\n"
        "done\n\n"

        "if ! $MOUNTED; then\n"
        "    echo \"No update device detected\"\n"
        "    exit 1\n"
        "fi\n\n"

        "cd \"$UPDATE_DIR\" || exit 1\n\n"

        "for prog in \"${PROGRAMS[@]}\"; do\n"
        "    if [[ \"$prog\" == *.* ]]; then\n"
        "        ext=\"${prog##*.}\"\n"
        "    else\n"
        "        ext=\"\"\n"
        "    fi\n"
        "    echo \"prog:$prog\"    \n"
        "    gpio mode 1 out\n"
        "    gpio write 1 1\n"
        "    if [[ $prog == \"update.sh\" ]];then\n"
        "        TARGET=\"$SH_DIR/$prog\"\n"
        "    elif [[ $prog == \"rc.local\" ]]; then\n"
        "        TARGET=\"$RCLOCAL_DIR\"\n"
        "    else\n"
        "        TARGET=\"$PROGRAM_DIR/$prog\"\n"
        "    fi \n\n"
        "    gpio write 1 0\n"
        "    echo \"TARGET:$TARGET\"\n"
        "    \n"
        "        if [ -f $prog ]; then\n"
        "            echo \"$prog detected, preparing to update...\"\n\n"

        "            if [ -f \"$prog.sha256\" ]; then\n"
        "                sha256sum -c \"$prog.sha256\" || {\n"
        "                    echo \"[$prog] Verification failed, skipping update\"\n"
        "                    continue\n"
        "            }\n"
        "        fi\n\n"

        "        cp \"$prog\" \"$TARGET\"\n"
        "        chmod 755 \"$TARGET\"\n\n"

        "        echo \"[$prog] Update completed, start the new version\"\n"
        "    fi\n"
        "done\n"
        "echo \"UPDATE_DIR:$UPDATE_DIR\"\n"
        "sleep 1\n"
        "umount -l \"$UPDATE_DIR\"\n"
        "rm -r \"$UPDATE_DIR\"\n\n"

        "echo \"All updates completed\"\n"
        "echo \"version 1.0.0\"\n";

    return write_file(filename, content, EXEC_MODE);
}

// 创建必要的目录结构
int create_directories(void)
{
    const char *dirs[] = {
        "/root/sh",
        "/root/tmp",
        "/root/shijing",
        "/mnt"};

    for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++)
    {
        struct stat st;
        if (stat(dirs[i], &st) == -1)
        {
            if (errno == ENOENT)
            {
                if (mkdir(dirs[i], DIR_MODE) == -1)
                {
                    fprintf(stderr, "Failed to create %s: %s\n",
                            dirs[i], strerror(errno));
                    return -1;
                }
                printf("Created directory: %s\n", dirs[i]);
            }
            else
            {
                perror("stat failed");
                return -1;
            }
        }
        else if (!S_ISDIR(st.st_mode))
        {
            fprintf(stderr, "Path exists but is not a directory: %s\n", dirs[i]);
            return -1;
        }
    }
    return 0;
}

// 新增函数：检查rc.local是否包含update.sh相关命令
int check_rc_local_contains_update(void)
{
    const char *filename = "/etc/rc.local";
    const char *search_str = "cp /root/sh/update.sh /root/tmp/update.sh";
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, search_str))
        {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found)
    {
        printf("Found update command in %s\n", filename);
        return 1; // 找到
    }
    else
    {
        printf("Update command not found in %s\n", filename);
        return 0; // 未找到
    }
}

// 新增函数：检查update.sh文件是否存在
int check_update_script_exists(void)
{
    const char *filename = "/root/sh/update.sh";
    struct stat st;

    if (stat(filename, &st) == 0)
    {
        printf("File exists: %s\n", filename);
        return 1; // 存在
    }
    else
    {
        if (errno == ENOENT)
        {
            printf("File does not exist: %s\n", filename);
            return 0; // 不存在
        }
        else
        {
            perror("stat failed");
            return -1; // 错误
        }
    }
}

// 新增函数：检查update.sh是否包含jlog.conf
int check_update_script_contains_jlog(void)
{
    const char *filename = "/root/sh/update.sh";
    const char *search_str = "jlog.conf";
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, search_str))
        {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found)
    {
        printf("Found '%s' in %s\n", search_str, filename);
        return 1; // 找到
    }
    else
    {
        printf("'%s' not found in %s\n", search_str, filename);
        return 0; // 未找到
    }
}

// 新增函数：检查jlog.conf是否包含jincubator.ERROR配置
int check_jlog_contains_jincubator(void)
{
    const char *filename = "/root/shijing/jlog.conf";
    const char *search_str = "jincubator.ERROR";
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        // 跳过注释行和空行
        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }

        // 查找配置项
        if (strstr(line, search_str))
        {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found)
    {
        printf("Found '%s' in %s\n", search_str, filename);
        return 1; // 找到
    }
    else
    {
        printf("'%s' not found in %s\n", search_str, filename);
        return 0; // 未找到
    }
}

void remove_log_file()
{
    system("rm -f /var/log/shijing/*");
}