// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init funny_kbd_init(void)
{
    pr_info("funny-kbd: Module loaded\n");
    return 0;
}

static void __exit funny_kbd_exit(void)
{
    pr_info("funny-kbd: Module unloaded\n");
    return;
}

module_init(funny_kbd_init);
module_exit(funny_kbd_exit);