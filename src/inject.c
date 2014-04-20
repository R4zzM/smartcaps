#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/ioctl.h>
#include "dbg.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* Key event values are not defined in linux/input.h */
#define KEY_VALUE_RELEASE 0
#define KEY_VALUE_PRESS 1
#define KEY_VALUE_REPEAT 2

static int inject_event(int fd, int type, int code, int value) 
{
  int ret;
  struct input_event keyevent;
  struct input_event synevent;

  memset(&keyevent, 0, sizeof(keyevent));
  memset(&synevent, 0, sizeof(synevent));

  keyevent.type = type;
  keyevent.code = code;
  keyevent.value = value;

  synevent.type = EV_SYN;
  synevent.code = 0;
  synevent.value = 0;
  
  debug("fd = %d", fd);
  ret = write(fd, &keyevent, sizeof(keyevent));
  check(ret != -1, "Writing event failed: %s", strerror(errno));
  
  ret = write(fd, &synevent, sizeof(synevent));
  check(ret != -1, "Writing syn failed: %s", strerror(errno));

  return 0;

error:
  return ret;
}

static int create_device(fd) 
{
  int ret;
  struct uinput_user_dev uidev;

  memset(&uidev, 0, sizeof(uidev));
  
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Smartcaps");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x045e;
  uidev.id.product = 0x00dd;
  uidev.id.version = 0x0111;

  ret = write(fd, &uidev, sizeof(uidev));
  check(ret != -1, "Failed to write to data to uniput: %s", strerror(errno));

  ret = ioctl(fd, UI_DEV_CREATE);
  check(ret != -1, "ioctl UI_DEV_CREATE failed. %s", strerror(errno));

  return 0;

error:
  return ret;
}

static int configure_device(fd) 
{
  int ret; 

  ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
  check(ret != -1, "ioctl UI_SET_EVBIT, EV_KEY failed. %s", strerror(errno));

  ret = ioctl(fd, UI_SET_EVBIT, EV_SYN);
  check(ret != -1, "ioctl UI_SET_EVBIT, EV_SYN failed. %s", strerror(errno));
 
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTCTRL);
  check(ret != -1, "ioctl UI_SET_EVBIT, KEY_LEFTCTRL failed. %s", 
      strerror(errno));

  ret = ioctl(fd, UI_SET_KEYBIT, KEY_ESC);
  check(ret != -1, "ioctl UI_SET_EVBIT, KEY_ESC failed. %s", strerror(errno));

  return 0;

error:
  return ret;
}

static int open_uinput() 
{
  int fd;
  fd = open("/dev/uinput", O_WRONLY);
  return fd;
}

int inject_init()
{
  int ret = 0;
  int fd;

  fd = open_uinput();
  if(fd < 0) {
    printf("Could not open device. ret = %d", fd);
    goto error; 
  }

  ret = configure_device(fd);
  if(ret) {
    printf("Could not configure device. ret = %d", ret);
    goto error_close_uinput;
  }

  ret = create_device(fd);
  if(ret) {
    printf("Could not create device. ret = %d", ret);
    goto error_close_uinput;
  }

  debug("returning fd = %d", fd);
  return fd;

error_close_uinput:
  close(fd);
error:
  return ret; 
}

int inject_lctrl_down(int fd) 
{
  int type = EV_KEY; 
  int code = KEY_LEFTCTRL; 
  int value = KEY_VALUE_PRESS;
  debug("Injecting event: type=%d, code=%d, value=%d", type, code, value);
  return inject_event(fd, type, code, value);
}

int inject_lctrl_up(int fd) 
{
  int type = EV_KEY; 
  int code = KEY_LEFTCTRL; 
  int value = KEY_VALUE_RELEASE;
  debug("Injecting event: type=%d, code=%d, value=%d", type, code, value);
  return inject_event(fd, type, code, value);
}

int inject_escape_down(int fd)
{
  int type = EV_KEY; 
  int code = KEY_ESC; 
  int value = KEY_VALUE_PRESS;
  debug("Injecting event: type=%d, code=%d, value=%d", type, code, value);
  return inject_event(fd, type, code, value);
}

int inject_escape_up(int fd)
{
  int type = EV_KEY; 
  int code = KEY_ESC;
  int value = KEY_VALUE_RELEASE;
  debug("Injecting event: type=%d, code=%d, value=%d", type, code, value);
  return inject_event(fd, type, code, value);
}