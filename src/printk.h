#ifndef _SOMELIB_PRINTK
#define _SOMELIB_PRINTK
void printk(char *fmt, ...);

#define pr_info(fmt, ...) printk("%a[i] %n" fmt, ##__VA_ARGS__);
#define pr_warn(fmt, ...) printk("%w[w] %n" fmt, ##__VA_ARGS__);
#define pr_fail(fmt, ...) printk("%f[f] %n" fmt, ##__VA_ARGS__);

#define bug_on(cond, fmt, ...) do {					\
	if ((cond)) {							\
		pr_fail(fmt "\n", ##__VA_ARGS__);			\
		while(1);						\
	}								\
} while (0);
#endif
