/*
 * Copyright (c) 2000-2004
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* There are multiple implementations of this interface (see,
 * e.g. bounds.c, hash_bounds.c) */

#ifndef BOUNDS_H
#define BOUNDS_H

#include "linkage.h"
#include "banshee.h"
#include "stamp.h"
#include "hashset.h"
#include "list.h"
#include "hash.h"

EXTERN_C_BEGIN

typedef struct bounds_ *bounds;

/* Hack: throw in all the necessary scanner implementations */
struct bounds_scanner_ {
  struct list_scanner ls;
  hash_table_scanner hs;
}; /* Opaque type: do not edit the fields!  */

typedef struct bounds_scanner_ bounds_scanner;

/* Create a new bounds representation */
bounds bounds_create(region r);

/* Replace with iterator */
gen_e_list bounds_exprs(bounds);

void bounds_scan(bounds, bounds_scanner *);

bool bounds_next(bounds_scanner *, gen_e *e);

/* Returns true if the bound was already present */
bool bounds_add(bounds,gen_e,stamp);

/* Returns true if the bound was removed */
bool bounds_remove(bounds,gen_e,stamp);

/* Returns true if the bound was present */
bool bounds_query(bounds,stamp); 

/* Returns true if there are no bounds */
bool bounds_empty(bounds);

void bounds_delete(bounds);  

void bounds_set(bounds, gen_e_list);

int bounds_size(bounds);

EXTERN_C_END

#endif /* BOUNDS_H */
