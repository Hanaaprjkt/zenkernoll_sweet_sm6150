# SPDX-License-Identifier: GPL-2.0
lib-y		:= bitops.o clear_user.o delay.o copy_from_user.o	\
		   copy_to_user.o copy_in_user.o copy_page.o		\
		   clear_page.o memchr.o memcpy.o memmove.o memset.o	\
		   memcmp.o strcmp.o strncmp.o strlen.o strnlen.o	\
		   strchr.o strrchr.o tishift.o

ifeq ($(CONFIG_KERNEL_MODE_NEON), y)
obj-$(CONFIG_XOR_BLOCKS)        += xor-neon.o
CFLAGS_REMOVE_xor-neon.o        += -mgeneral-regs-only
CFLAGS_xor-neon.o               += -ffreestanding
endif


lib-$(CONFIG_ARCH_HAS_UACCESS_FLUSHCACHE) += uaccess_flushcache.o
