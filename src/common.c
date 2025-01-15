#include "framebuffer.h"
#include "console.h"
#include "paging.h"
#include "bitmap_phy_alloc.h"

struct console *global_console = &framebuffer_console;
struct page_allocator *phy_page_allocator = &bitmap_phy_alloc;
