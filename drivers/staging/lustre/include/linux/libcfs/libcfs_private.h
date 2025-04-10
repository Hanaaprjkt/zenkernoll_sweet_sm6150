/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2011, 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * libcfs/include/libcfs/libcfs_private.h
 *
 * Various defines for libcfs.
 *
 */

#ifndef __LIBCFS_PRIVATE_H__
#define __LIBCFS_PRIVATE_H__

#ifndef DEBUG_SUBSYSTEM
# define DEBUG_SUBSYSTEM S_UNDEFINED
#endif

/*
 * When this is on, LASSERT macro includes check for assignment used instead
 * of equality check, but doesn't have unlikely(). Turn this on from time to
 * time to make test-builds. This shouldn't be on for production release.
 */
#define LASSERT_CHECKED (0)

#define LASSERTF(cond, fmt, ...)					\
do {									\
	if (unlikely(!(cond))) {					\
		LIBCFS_DEBUG_MSG_DATA_DECL(__msg_data, D_EMERG, NULL);	\
		libcfs_debug_msg(&__msg_data,				\
				 "ASSERTION( %s ) failed: " fmt, #cond,	\
				 ## __VA_ARGS__);			\
		lbug_with_loc(&__msg_data);				\
	}								\
} while (0)

#define LASSERT(cond) LASSERTF(cond, "\n")

#ifdef CONFIG_LUSTRE_DEBUG_EXPENSIVE_CHECK
/**
 * This is for more expensive checks that one doesn't want to be enabled all
 * the time. LINVRNT() has to be explicitly enabled by
 * CONFIG_LUSTRE_DEBUG_EXPENSIVE_CHECK option.
 */
# define LINVRNT(exp) LASSERT(exp)
#else
# define LINVRNT(exp) ((void)sizeof !!(exp))
#endif

#define KLASSERT(e) LASSERT(e)

void __noreturn lbug_with_loc(struct libcfs_debug_msg_data *msg);

#define LBUG()							  \
do {								    \
	LIBCFS_DEBUG_MSG_DATA_DECL(msgdata, D_EMERG, NULL);	     \
	lbug_with_loc(&msgdata);					\
} while (0)

#ifndef LIBCFS_VMALLOC_SIZE
#define LIBCFS_VMALLOC_SIZE	(2 << PAGE_SHIFT) /* 2 pages */
#endif

#define LIBCFS_ALLOC_PRE(size, mask)					\
	LASSERT(!in_interrupt() || ((size) <= LIBCFS_VMALLOC_SIZE &&	\
				    !gfpflags_allow_blocking(mask)))

#define LIBCFS_ALLOC_POST(ptr, size)					    \
do {									    \
	if (unlikely(!(ptr))) {						    \
		CERROR("LNET: out of memory at %s:%d (tried to alloc '"	    \
		       #ptr "' = %d)\n", __FILE__, __LINE__, (int)(size));  \
	} else {							    \
		memset((ptr), 0, (size));				    \
	}								    \
} while (0)

/**
 * allocate memory with GFP flags @mask
 */
#define LIBCFS_ALLOC_GFP(ptr, size, mask)				    \
do {									    \
	LIBCFS_ALLOC_PRE((size), (mask));				    \
	(ptr) = (size) <= LIBCFS_VMALLOC_SIZE ?				    \
		kmalloc((size), (mask)) : vmalloc(size);	    \
	LIBCFS_ALLOC_POST((ptr), (size));				    \
} while (0)

/**
 * default allocator
 */
#define LIBCFS_ALLOC(ptr, size) \
	LIBCFS_ALLOC_GFP(ptr, size, GFP_NOFS)

/**
 * non-sleeping allocator
 */
#define LIBCFS_ALLOC_ATOMIC(ptr, size) \
	LIBCFS_ALLOC_GFP(ptr, size, GFP_ATOMIC)

/**
 * allocate memory for specified CPU partition
 *   \a cptab != NULL, \a cpt is CPU partition id of \a cptab
 *   \a cptab == NULL, \a cpt is HW NUMA node id
 */
#define LIBCFS_CPT_ALLOC_GFP(ptr, cptab, cpt, size, mask)		    \
do {									    \
	LIBCFS_ALLOC_PRE((size), (mask));				    \
	(ptr) = (size) <= LIBCFS_VMALLOC_SIZE ?				    \
		kmalloc_node((size), (mask), cfs_cpt_spread_node(cptab, cpt)) :\
		vmalloc_node(size, cfs_cpt_spread_node(cptab, cpt));	    \
	LIBCFS_ALLOC_POST((ptr), (size));				    \
} while (0)

/** default numa allocator */
#define LIBCFS_CPT_ALLOC(ptr, cptab, cpt, size)				    \
	LIBCFS_CPT_ALLOC_GFP(ptr, cptab, cpt, size, GFP_NOFS)

#define LIBCFS_FREE(ptr, size)					  \
do {								    \
	if (unlikely(!(ptr))) {						\
		CERROR("LIBCFS: free NULL '" #ptr "' (%d bytes) at "    \
		       "%s:%d\n", (int)(size), __FILE__, __LINE__);	\
		break;						  \
	}							       \
	kvfree(ptr);					  \
} while (0)

/******************************************************************************/

void libcfs_debug_dumplog(void);
int libcfs_debug_init(unsigned long bufsize);
int libcfs_debug_cleanup(void);
int libcfs_debug_clear_buffer(void);
int libcfs_debug_mark_buffer(const char *text);

/*
 * allocate a variable array, returned value is an array of pointers.
 * Caller can specify length of array by count.
 */
void *cfs_array_alloc(int count, unsigned int size);
void  cfs_array_free(void *vars);

#define LASSERT_ATOMIC_ENABLED	  (1)

#if LASSERT_ATOMIC_ENABLED

/** assert value of @a is equal to @v */
#define LASSERT_ATOMIC_EQ(a, v)			\
	LASSERTF(atomic_read(a) == v, "value: %d\n", atomic_read((a)))

/** assert value of @a is unequal to @v */
#define LASSERT_ATOMIC_NE(a, v)		\
	LASSERTF(atomic_read(a) != v, "value: %d\n", atomic_read((a)))

/** assert value of @a is little than @v */
#define LASSERT_ATOMIC_LT(a, v)		\
	LASSERTF(atomic_read(a) < v, "value: %d\n", atomic_read((a)))

/** assert value of @a is little/equal to @v */
#define LASSERT_ATOMIC_LE(a, v)		\
	LASSERTF(atomic_read(a) <= v, "value: %d\n", atomic_read((a)))

/** assert value of @a is great than @v */
#define LASSERT_ATOMIC_GT(a, v)		\
	LASSERTF(atomic_read(a) > v, "value: %d\n", atomic_read((a)))

/** assert value of @a is great/equal to @v */
#define LASSERT_ATOMIC_GE(a, v)		\
	LASSERTF(atomic_read(a) >= v, "value: %d\n", atomic_read((a)))

/** assert value of @a is great than @v1 and little than @v2 */
#define LASSERT_ATOMIC_GT_LT(a, v1, v2)			 \
do {							    \
	int __v = atomic_read(a);			   \
	LASSERTF(__v > v1 && __v < v2, "value: %d\n", __v);     \
} while (0)

/** assert value of @a is great than @v1 and little/equal to @v2 */
#define LASSERT_ATOMIC_GT_LE(a, v1, v2)			 \
do {							    \
	int __v = atomic_read(a);			   \
	LASSERTF(__v > v1 && __v <= v2, "value: %d\n", __v);    \
} while (0)

/** assert value of @a is great/equal to @v1 and little than @v2 */
#define LASSERT_ATOMIC_GE_LT(a, v1, v2)			 \
do {							    \
	int __v = atomic_read(a);			   \
	LASSERTF(__v >= v1 && __v < v2, "value: %d\n", __v);    \
} while (0)

/** assert value of @a is great/equal to @v1 and little/equal to @v2 */
#define LASSERT_ATOMIC_GE_LE(a, v1, v2)			 \
do {							    \
	int __v = atomic_read(a);			   \
	LASSERTF(__v >= v1 && __v <= v2, "value: %d\n", __v);   \
} while (0)

#else /* !LASSERT_ATOMIC_ENABLED */

#define LASSERT_ATOMIC_EQ(a, v)		 ((void)0)
#define LASSERT_ATOMIC_NE(a, v)		 ((void)0)
#define LASSERT_ATOMIC_LT(a, v)		 ((void)0)
#define LASSERT_ATOMIC_LE(a, v)		 ((void)0)
#define LASSERT_ATOMIC_GT(a, v)		 ((void)0)
#define LASSERT_ATOMIC_GE(a, v)		 ((void)0)
#define LASSERT_ATOMIC_GT_LT(a, v1, v2)	 ((void)0)
#define LASSERT_ATOMIC_GT_LE(a, v1, v2)	 ((void)0)
#define LASSERT_ATOMIC_GE_LT(a, v1, v2)	 ((void)0)
#define LASSERT_ATOMIC_GE_LE(a, v1, v2)	 ((void)0)

#endif /* LASSERT_ATOMIC_ENABLED */

#define LASSERT_ATOMIC_ZERO(a)		  LASSERT_ATOMIC_EQ(a, 0)
#define LASSERT_ATOMIC_POS(a)		   LASSERT_ATOMIC_GT(a, 0)

#define CFS_ALLOC_PTR(ptr)      LIBCFS_ALLOC(ptr, sizeof(*(ptr)))
#define CFS_FREE_PTR(ptr)       LIBCFS_FREE(ptr, sizeof(*(ptr)))

/* max value for numeric network address */
#define MAX_NUMERIC_VALUE 0xffffffff

/* implication */
#define ergo(a, b) (!(a) || (b))
/* logical equivalence */
#define equi(a, b) (!!(a) == !!(b))

/* --------------------------------------------------------------------
 * Light-weight trace
 * Support for temporary event tracing with minimal Heisenberg effect.
 * --------------------------------------------------------------------
 */

#define MKSTR(ptr) ((ptr)) ? (ptr) : ""

static inline size_t cfs_size_round4(int val)
{
	return (val + 3) & (~0x3);
}

#ifndef HAVE_CFS_SIZE_ROUND
static inline size_t cfs_size_round(int val)
{
	return (val + 7) & (~0x7);
}

#define HAVE_CFS_SIZE_ROUND
#endif

static inline size_t cfs_size_round16(int val)
{
	return (val + 0xf) & (~0xf);
}

static inline size_t cfs_size_round32(int val)
{
	return (val + 0x1f) & (~0x1f);
}

static inline size_t cfs_size_round0(int val)
{
	if (!val)
		return 0;
	return (val + 1 + 7) & (~0x7);
}

static inline size_t cfs_round_strlen(char *fset)
{
	return cfs_size_round((int)strlen(fset) + 1);
}

#endif
