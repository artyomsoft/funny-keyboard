# Funny Keyboard – a silly Linux kernel module

This is a playful out-of-tree Linux kernel module that intercepts keyboard input and adds some (hopefully) entertaining misbehavior to your typing.

**Warning**
- This module hooks into the keyboard input path → it affects **all** keyboards on the system (USB, built-in, Bluetooth…).
- It can be very annoying (that's the point).
- Not suitable for production machines or when you actually need to get work done.
- Some antivirus solutions might flag it as suspicious.
- Use at your own risk – you have been warned 😈.

## How to build & use (x86_64 most tested)

1. **Install build prerequisites**

   ```bash
   sudo apt update
   sudo apt install linux-headers-$(uname -r) make build-essential git
   ```
1. **Build**
   ```bash
   make
   ```
1. **Load the module** (be ready for chaos)
   ```bash
   sudo insmod ./funny-kbd.ko
   ```
   Watch dmesg for confirmation:
   ```bash
   dmesg | tail -n 20
   ```
1. **Enjoy (or suffer) the effects**
   Try typing in any text field — terminal, browser, editor, password prompt (!)
1. **Make it stop** (very important)
   ```bash
   sudo rmmod funny-kbd
   ```
   If it refuses to unload (because it's busy), you can force it (risky)
   ```bash
   sudo rmmod -f funny-kbd
   ```
   Or just reboot (the nuclear option).

## Current funny behavior (v1.0)

- **After every comma (,)** the module automatically inserts a silly string
This affects **all keyboards** on the system and **all applications** (terminal, browser, Discord, password fields(!), etc.)