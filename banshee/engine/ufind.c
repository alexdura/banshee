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

#include <stdio.h>
#include <assert.h>
#include "ufind.h"
#include "list.h"

enum uf_kind {uf_ecr,uf_link};
typedef enum uf_kind uf_kind;

DECLARE_LIST(elt_stack, uf_element);
DEFINE_LIST(elt_stack, uf_element);

typedef struct ustack_elt_ {
  uf_element nonroot;
  void *old_info;
} *ustack_elt;

DECLARE_LIST(union_stack,ustack_elt);
DEFINE_LIST(union_stack,ustack_elt);

struct uf_element {
  uf_kind kind;
  int rank;
  void *info;
  struct uf_element *link;
  elt_stack elt_stack;
};

static region stackregion = NULL;
static union_stack ustack = NULL;

struct uf_element *new_uf_element(region r, void *info)
{
  struct uf_element *result;

  result = ralloc(r, struct uf_element);

  result->kind = uf_ecr;
  result->rank = 0;
  result->info = info;
  result->link = NULL;
  result->elt_stack = NULL;

  return result;
}

static struct uf_element *find(struct uf_element *e)
{

  if (e->kind == uf_ecr)
    return e;
  
  else if (e->link->kind == uf_link)
    {
      struct uf_element *temp = e->link;
	
      e->link = temp->link;
      
      assert(temp->elt_stack);
      elt_stack_cons(temp,temp->elt_stack);

      return find(temp);
    }

  else
    return e->link;
}

static ustack_elt make_ustack_elt(uf_element e,void *info)
{
  ustack_elt result = ralloc(stackregion,struct ustack_elt_ *);
  result->nonroot = e;
  result->old_info = info;
  
  return result;
}

bool uf_union(struct uf_element *a, struct uf_element *b)
{
  struct uf_element *e1 = find(a);
  struct uf_element *e2 = find(b);

  assert(ustack);

  if ( e1 == e2 )
    return FALSE;

 else if (e1->rank < e2->rank)
    {
      ustack_elt ue = make_ustack_elt(e1,e2->info);
      e1->kind = uf_link;
      e1->link = e2;

      union_stack_cons(ue, ustack);
      assert(e1->elt_stack == NULL);
      e1->elt_stack = new_elt_stack(stackregion);

      return TRUE;
    }

  else if (e1->rank > e2->rank)
    {
      ustack_elt ue = make_ustack_elt(e2,e1->info);
      e2->kind = uf_link;
      e2->link = e1;

      union_stack_cons(ue, ustack);
      assert(e2->elt_stack == NULL);
      e2->elt_stack = new_elt_stack(stackregion);

      return TRUE;
    }
  
  else 
    {
      ustack_elt ue = make_ustack_elt(e1,e2->info);
      e2->rank++;
      
      e1->kind = uf_link;
      e1->link = e2;

      union_stack_cons(ue, ustack);
      assert(e1->elt_stack == NULL);
      e1->elt_stack = new_elt_stack(stackregion);

      return TRUE;
    }

}

bool uf_unify(combine_fn_ptr combine,
	      struct uf_element *a, struct uf_element *b)
{
  struct uf_element *e1 = find(a);
  struct uf_element *e2 = find(b);

  assert(ustack);

  if ( e1 == e2 )
    return FALSE;

  else if (e1->rank < e2->rank)
    {
      ustack_elt ue = make_ustack_elt(e1,e2->info);
      e2->info = combine(e2->info,e1->info);
      e1->kind = uf_link;
      e1->link = e2;
     
      union_stack_cons(ue, ustack);
      assert(e1->elt_stack == NULL);
      e1->elt_stack = new_elt_stack(stackregion);


      return TRUE;
    }

  else if (e1->rank > e2->rank)
    {
      ustack_elt ue = make_ustack_elt(e2,e1->info);
      e1->info = combine(e1->info,e2->info);
      e2->kind = uf_link;
      e2->link = e1;

      union_stack_cons(ue, ustack);
      assert(e2->elt_stack == NULL);
      e2->elt_stack = new_elt_stack(stackregion);

      return TRUE;
    }
  
  else 
    {      
      ustack_elt ue = make_ustack_elt(e1,e2->info);
      e2->info = combine(e2->info, e1->info);

      e2->rank++;
      e1->kind = uf_link;
      e1->link = e2;

      union_stack_cons(ue, ustack);
      assert(e1->elt_stack == NULL);
      e1->elt_stack = new_elt_stack(stackregion);

      return TRUE;
    }
}

void *uf_get_info(struct uf_element *e)
{
  return find(e)->info;
}


bool uf_eq(struct uf_element *e1,struct uf_element *e2)
{
  return (find(e1) == find(e2));
}

void uf_update(struct uf_element *e,uf_info i)
{
  e = find(e);
  ustack_elt ue = make_ustack_elt(e,e->info);
  union_stack_cons(ue,ustack);
  assert(e->elt_stack == NULL);

  e->info = i;
}

static void repair_elt_stack(struct uf_element *nonroot)
{
  elt_stack_scanner scan;
  uf_element temp;

  assert(nonroot->elt_stack);

  elt_stack_scan(nonroot->elt_stack,&scan);

  while(elt_stack_next(&scan,&temp)) {
    assert(temp->kind == uf_link);
    temp->link = nonroot;
  }
  nonroot->elt_stack = NULL;
}

/* Undo the last union operation */
static bool uf_backtrack_one()
{
  ustack_elt ue = NULL;

  if (union_stack_length(ustack) == 0) return FALSE;

  /* Pop the last unioned elt off the stack */
  ue = union_stack_head(ustack);
  union_stack_tail(ustack);

  if (ue == NULL) return FALSE;
  
  /* This happens when the last operation was an update */
  if (ue->nonroot->kind == uf_ecr) {
    /* Just roll back the old ecr's info */
    ue->nonroot->info = ue->old_info;
  }
  else {
    /* Make sure it's a link */
    assert(ue->nonroot->kind == uf_link);
    
    /* Roll back the old ecr's info */
    find(ue->nonroot)->info = ue->old_info;
    
    /* Deunion */
    ue->nonroot->kind = uf_ecr;
    ue->nonroot->link = NULL;
    
    /* Repair the element stack */
    repair_elt_stack(ue->nonroot);
  }
  
  return TRUE;
}

void uf_rollback()
{
  while (uf_backtrack_one());
}

void uf_backtrack()
{
  uf_backtrack_one();
}

void uf_tick()
{
  union_stack_cons(NULL,ustack);
}



void uf_init()
{
  stackregion = newregion();
  ustack = new_union_stack(stackregion);
}




