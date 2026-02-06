// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>

MODULE_LICENSE("GPL");

static int keyboard_notifier_cb(struct notifier_block *nblock,
                                unsigned long action, void *param)
{
    struct keyboard_notifier_param *p = param;
    unsigned int keycode = p->value;
    int down = p->down;
    pr_info("funny-kbd: action = %lu keycode = %u  down = %d\n", action, keycode, down);
    return NOTIFY_OK;
}

static struct notifier_block kbd_nb = {
    .notifier_call = keyboard_notifier_cb};

static int __init funny_kbd_init(void)
{
    pr_info("funny-kbd: Module loaded\n");
    return register_keyboard_notifier(&kbd_nb);
}

static void __exit funny_kbd_exit(void)
{
    unregister_keyboard_notifier(&kbd_nb);
    pr_info("funny-kbd: Module unloaded\n");
    return;
}

module_init(funny_kbd_init);
module_exit(funny_kbd_exit);