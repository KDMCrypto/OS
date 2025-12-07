#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/timekeeping.h>

#define PROCFS_NAME "tsulab"

// Период орбиты Восток-1 примерно 89.34 минут = 5360 секунд
#define ORBIT_PERIOD_SEC 5360

static struct proc_dir_entry *proc_file; 

// Функция для начала отсчета времени 12.04.1961 06:21 UTC
static long gagarin_launch_timestamp(void)
{
    struct tm t; // труктура для хранения календарного времени
    time64_t timestamp;

    memset(&t, 0, sizeof(t));
    t.tm_year = 61; // год
    t.tm_mon  = 3; // месяц
    t.tm_mday = 12; // число
    t.tm_hour = 6; // часы
    t.tm_min  = 21; // минуты
    t.tm_sec  = 0; // секунды

    // время выхода на орбиту
    timestamp = mktime64(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec); 

    return timestamp;
}

// Чтение /proc/tsulab 
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset) {
    char msg[128];
    int len;
    long now = ktime_get_real_seconds(); // текущее время в секундах
    long o_timestamp = gagarin_launch_timestamp(); // время выхода Гагарина на орбиту
    long diff = now - o_timestamp; // разница времени

    long revolutions = diff / ORBIT_PERIOD_SEC; // число оборотов

    if (*offset > 0)
        return 0;

    len = snprintf(msg, sizeof(msg), "Gagarin would have completed %ld orbits by now.\n", revolutions);

    if (copy_to_user(buffer, msg, len)) 
        return -EFAULT;

    *offset = len;
    return len;
}

// Создание структуры инициализации
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read, 
};
#endif

// Инициализация модуля
static int __init tsulab_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");

    proc_file = proc_create(PROCFS_NAME, 0444, NULL, &proc_file_fops);
    if (!proc_file) {
        pr_err("failed to create /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

// Выгрузка модуля
static void __exit tsulab_exit(void)
{
    proc_remove(proc_file); // удаляю файл
    pr_info("Tomsk State University forever!\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);

MODULE_LICENSE("GPL");