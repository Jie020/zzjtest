#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/device.h>

#define IOCTL_ALLOC_MEMORY _IO('k', 1)    // 定义申请内存的ioctl命令
#define IOCTL_STORE_DATA _IOW('k', 2, char *)  // 定义将数据存储到内核空间的ioctl命令
#define IOCTL_EXPORT_DATA _IOR('k', 3, char *) // 定义将数据导出到用户空间的ioctl命令
#define DEVICE_NAME "ioctl_example"
#define CLASS_NAME "myclass"

static int major;
static struct class *class = NULL;
static struct device *device = NULL;
static char *kernel_buffer = NULL; // 内核空间的缓冲区指针
static int buffer_size = 0;        // 缓冲区大小

// ioctl处理函数
static long ioctl_example(struct file *filp, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    char *tmp_buffer = NULL;

    switch (cmd) {
        case IOCTL_ALLOC_MEMORY:
            // 接收用户空间传来的缓冲区大小参数
            buffer_size = arg;
            // 在内核空间申请一块大小为buffer_size的内存
            kernel_buffer = kmalloc(buffer_size, GFP_KERNEL);
            if (!kernel_buffer) {
                ret = -ENOMEM; // 内存申请失败
                break;
            }
            memset(kernel_buffer, 0, buffer_size); // 清零内存区域
            break;

        case IOCTL_STORE_DATA:
            tmp_buffer = (char *) arg; // 用户空间传来的数据缓冲区指针
            // 从用户空间复制数据到内核空间的缓冲区
            if (copy_from_user(kernel_buffer, tmp_buffer, buffer_size)) {
                ret = -EFAULT; // 复制失败
            }
            break;

        case IOCTL_EXPORT_DATA:
            tmp_buffer = (char *) arg; // 用户空间传来的数据缓冲区指针
            // 将内核空间的数据缓冲区内容复制到用户空间的缓冲区
            if (copy_to_user(tmp_buffer, kernel_buffer, buffer_size)) {
                ret = -EFAULT; // 复制失败
            }
            break;

        default:
            ret = -ENOTTY; // 不支持的ioctl命令
            break;
    }

    return ret;
}

static const struct file_operations fops = {
    .unlocked_ioctl = ioctl_example, // 指定ioctl处理函数
};

static int __init ioctl_init(void) {

    // 注册字符设备驱动，动态获取主设备号
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major;
    }
    //class 就是设备类的句柄
	class = class_create (THIS_MODULE, CLASS_NAME);
    if (IS_ERR(class)) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the class\n");
        return PTR_ERR(class);
    }
    
    //注册设备文件
    device = device_create(class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        class_destroy(class);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(device);
    }

    printk(KERN_INFO "ioctl_example module loaded with major number %d\n", major);
    return 0;
}

static void __exit ioctl_exit(void) {
    //注销设备文件
	device_destroy (class, MKDEV(major, 0));
	//注销设备类
	class_destroy (class);
    // 注销字符设备驱动
    unregister_chrdev(major, DEVICE_NAME);
    if (kernel_buffer) {
        kfree(kernel_buffer); // 释放内核空间的缓冲区内存
    }
    printk(KERN_INFO "ioctl_example module unloaded\n");
}

module_init(ioctl_init);
module_exit(ioctl_exit);

MODULE_LICENSE("GPL");//设置为开源

