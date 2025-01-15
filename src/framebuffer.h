#ifndef __SOMELIB_VGA
#define __SOMELIB_VGA

#include <stdint.h>
#include "console.h"
#include "multiboot2.h"

void init_framebuffer(struct multiboot_tag_framebuffer *fb);

extern struct console framebuffer_console;

#endif
