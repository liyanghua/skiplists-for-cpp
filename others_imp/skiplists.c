/*
  Skiplists: A C implementation of the skiplists data structure
  Copyright (C) 2001 Chih-Hao Tsai

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
*/

/*
  Skip lists are a relatively new data structure introduced in
  1990 by Bill Pugh in Communications of the ACM. It is a
  probabilistic alternative to the widely used balanced binary
  trees.

  Pugh, W. (1990). Skip lists: A probabilistic alternative to 
  balanced trees. Communications of the ACM, 33(6), 668-676.

  A PDF version of Pugh (1990) is available at:
  
  ftp://ftp.cs.umd.edu/pub/skipLists/skiplists.pdf

  Sample code is available at:

  ftp://ftp.cs.umd.edu/pub/skipLists/

  My implementation is basically based on the C sample code
  skipLists.c provided by Professor Pugh at the above-mentioned
  site. What I have done was mainly to augment the original code
  to make it capable of handling arbitrary data types of key and
  value. In addition, I have also eliminated global variables
  and made quite a few stylish changes to improve the usability
  and readability.
*/

/*
  Module: skiplists.c
  Version: 0.11
  Date: April 10, 2001
  Author: Chih-Hao Tsai
  Email: hao520@yahoo.com
  WWW: http://chtsai.org/
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "skiplists.h"


#define FALSE 0
#define TRUE 1

#ifndef RAND_MAX
#define RAND_MAX 32767
#endif


/* function prototypes */

static SkiplistsNode skiplists_new_node_of_level (int);
static void skiplists_remove_node (SkiplistsNode n);
static int random_level (const SkiplistsRandom);
static int count_bits (int);


SkiplistsNode skiplists_new_node_of_level (int l)
{
  SkiplistsNode n;

  n = (SkiplistsNode) malloc (sizeof (SkiplistsNodeStructure));
  n->level = l;
  n->forward = (SkiplistsNode *) malloc ((l + 1) * sizeof (SkiplistsNode));
  n->key = NULL;
  n->value = NULL;

  return (n);
}


void
skiplists_remove_node (SkiplistsNode n)
{
  free (n->key);
  free (n->value);
  free (n->forward);
  free (n);
}


Skiplists skiplists_new (const SkiplistsCompareFunc compare_function)
{
  Skiplists SL;
  int i;

  srand (time (0));

  SL = (Skiplists) malloc (sizeof (SkiplistsStructure));

  SL->size = 0;
  SL->init_fingers = TRUE;
#ifdef SKIPLISTS_ALLOW_DUPLICATES
  SL->allow_duplicates = TRUE;
#else
  SL->allow_duplicates = FALSE;
#endif
  SL->compare_function = compare_function;
  SL->duplicate_function = NULL;
  SL->duplicate_rule = LAST_IN_FIRST_OUT;

  SL->R = (SkiplistsRandom) malloc (sizeof (SkiplistsRandomStructure));

  SL->R->max_number_of_levels = SKIPLISTS_MAX_NUMBER_OF_LEVELS;
  SL->R->max_level = SL->R->max_number_of_levels - 1;
  SL->R->inverse_p = SKIPLISTS_INVERSE_P;
  SL->R->bits_per_use = count_bits (SL->R->inverse_p);
  SL->R->bits_in_random = count_bits (RAND_MAX);
  SL->R->random_bits = rand ();
  SL->R->randoms_left = SL->R->bits_in_random / SL->R->bits_per_use;

  SL->L = (SkiplistsList) malloc (sizeof (SkiplistsListStructure));

  SL->L->level = 0;
  SL->L->Head = skiplists_new_node_of_level (SL->R->max_number_of_levels);
  SL->L->Tail = skiplists_new_node_of_level (0);
  SL->L->finger =
    (SkiplistsNode *) malloc (SL->R->max_number_of_levels *
			      sizeof (SkiplistsNode));
  for (i = 0; i < SL->R->max_number_of_levels; i++)
    SL->L->Head->forward[i] = SL->L->Tail;

  return (SL);
}


void
skiplists_destroy (Skiplists SL)
{
  SkiplistsNode p, q;

  p = SL->L->Head;

  while (p != SL->L->Tail)
    {
      q = p->forward[0];
      skiplists_remove_node (p);
      p = q;
    }
  free (p->forward);
  free (p);

  free (SL->R);
  free (SL->L->finger);
  free (SL->L);
  free (SL);
}


int
random_level (const SkiplistsRandom R)
{
  register int level;
  register int b;

  level = b = 0;

  while (!b)
    {
      b = R->random_bits & (R->inverse_p - 1);
      if (!b)
	level++;
      R->random_bits >>= R->bits_per_use;
      if (--R->randoms_left == 0)
	{
	  R->random_bits = rand ();
	  R->randoms_left = R->bits_in_random / R->bits_per_use;
	}
    }

  return (level > R->max_level ? R->max_level : level);
}


boolean
skiplists_insert (const Skiplists SL, SkiplistsKey key, SkiplistsValue value)
{
  register int i, k;
  register boolean key_found;
  register SkiplistsValue old_value, new_value;
  static SkiplistsNode update[SKIPLISTS_MAX_NUMBER_OF_LEVELS];
  register SkiplistsNode p, q = NULL, last_identical;

  for (p = SL->L->Head, k = SL->L->level; k >= 0; update[k] = p, k--)
    for (; q = p->forward[k], q != SL->L->Tail; p = q)
      if (SL->compare_function (q->key, key) >= 0)
	break;

  SL->init_fingers = TRUE;

  key_found = FALSE;
  last_identical = NULL;

  if (q != SL->L->Tail)
    {
      key_found = SL->compare_function (q->key, key) == 0 ? TRUE : FALSE;
      last_identical = p = key_found ? q : NULL;
    }

  if (key_found && SL->duplicate_rule == FIRST_IN_FIRST_OUT)
    {
      for (p = last_identical; p != SL->L->Tail; p = p->forward[0])
	if (SL->compare_function (p->key, key) == 0)
	  {
	    last_identical = p;
	    for (k = (int) p->level; k >= 0; k--)
	      update[k] = last_identical;
	  }
	else
	  break;
    }

  if (!SL->allow_duplicates)
    if (key_found)
      {
	free (key);
	if (!SL->duplicate_function)
	  {
	    free (last_identical->value);
	    last_identical->value = value;
	    return (FALSE);
	  }
	else
	  {
	    old_value = last_identical->value;
	    new_value = value;
	    last_identical->value =
	      SL->duplicate_function (last_identical->value, value);
	    if (last_identical->value != old_value)
	      free (old_value);
	    if (last_identical->value != new_value)
	      free (new_value);
	    return (FALSE);
	  }
      }

  k = random_level (SL->R);

  /* if (k > SL->L->level)
     {
     k = ++SL->L->level;
     update[k] = SL->L->Head;
     } */

  if (k > SL->L->level)
    {
      for (i = k; i > SL->L->level; i--)
	update[i] = SL->L->Head;
      SL->L->level = k;
    }

  q = skiplists_new_node_of_level (k);

  q->key = key;
  q->value = value;

  while (k >= 0)
    {
      p = update[k];
      q->forward[k] = p->forward[k];
      p->forward[k] = q;
      k--;
    }

  SL->size++;

  return (TRUE);
}


boolean skiplists_remove (const Skiplists SL, const SkiplistsKey key)
{
  register int k, m;
  register boolean key_found;
  static SkiplistsNode update[SKIPLISTS_MAX_NUMBER_OF_LEVELS];
  register SkiplistsNode p, q = NULL, last_identical;

  for (p = SL->L->Head, k = m = SL->L->level; k >= 0; update[k] = p, k--)
    for (; q = p->forward[k], q != SL->L->Tail; p = q)
      if (SL->compare_function (q->key, key) >= 0)
	break;

  SL->init_fingers = TRUE;

  key_found = FALSE;
  last_identical = NULL;

  if (q != SL->L->Tail)
    {
      key_found = SL->compare_function (q->key, key) == 0 ? TRUE : FALSE;
      last_identical = p = key_found ? q : NULL;
    }

  if (key_found)
    {
      for (k = 0; k <= m && (p = update[k])->forward[k] == last_identical;
	   k++)
	p->forward[k] = last_identical->forward[k];
      skiplists_remove_node (last_identical);
      while (SL->L->Head->forward[m] == SL->L->Tail && m > 0)
	m--;
      SL->L->level = m;
      SL->size--;
      return (TRUE);
    }
  else
    return (FALSE);
}


void *
skiplists_lookup (const Skiplists SL, const SkiplistsKey key)
{
  register int k;
  register boolean key_found;
  register SkiplistsNode p, q = NULL, last_identical;

  for (p = SL->L->Head, k = SL->L->level; k >= 0; k--)
    for (; q = p->forward[k], q != SL->L->Tail; p = q)
      if (SL->compare_function (q->key, key) >= 0)
	break;

  key_found = FALSE;
  last_identical = NULL;

  if (q != SL->L->Tail)
    {
      key_found = SL->compare_function (q->key, key) == 0 ? TRUE : FALSE;
      last_identical = p = key_found ? q : NULL;
    }

  if (!key_found)
    return (NULL);

  return (last_identical->value);
}


/*
  Function skiplists_lookup_with_finger is an implementation of
  the "Search With Finger" algorithm reported in the following
  article:
  
  Pugh, W. (1990). A skip list cookbook. Tech. Rep. No. 
  CSPTRP2286.1. College Park, MD: Department of Computer Science,
  University of Maryland.

  ftp://ftp.cs.umd.edu/pub/skipLists/cookbook.pdf
*/

void *
skiplists_lookup_with_finger (const Skiplists SL, const SkiplistsKey key)
{
  register int k, level, compare;
  register boolean key_found;
  register SkiplistsNode p, q, last_identical;

  key_found = FALSE;
  last_identical = NULL;

  if (SL->init_fingers)
    {
      q = SL->L->Head;
      level = SL->L->level;
      SL->init_fingers = FALSE;
    }
  else
    {
      if (SL->compare_function (SL->L->finger[0]->key, key) < 0)
	{
	  for (k = 1; k <= SL->L->level; k++)
	    {
	      q = SL->L->finger[k]->forward[k];
	      if (q == SL->L->Tail)
		break;
	      if ((compare = SL->compare_function (q->key, key)) >= 0)
		{
		  if (compare == 0)
		    {
		      key_found = TRUE;
		      last_identical = q;
		    }
		  break;
		}
	    }
	  q = SL->L->finger[--k];
	  level = k;
	}
      else
	{
	  for (k = 1; k <= SL->L->level; k++)
	    {
	      q = SL->L->finger[k];
	      if (q == SL->L->Head)
		break;
	      if ((compare = SL->compare_function (q->key, key)) <= 0)
		{
		  if (compare == 0)
		    {
		      key_found = TRUE;
		      last_identical = q;
		    }
		  else
		    break;
		}
	    }

	  if (k > SL->L->level)
	    {
	      q = SL->L->Head;
	      level = SL->L->level;
	    }
	  else
	    {
	      q = SL->L->finger[k];
	      level = k;
	    }
	}
    }

  if (!key_found)
    {
      for (p = q, k = level; k >= 0; SL->L->finger[k] = p, k--)
	for (; q = p->forward[k], q != SL->L->Tail; p = q)
	  if (SL->compare_function (q->key, key) >= 0)
	    break;

      if (q != SL->L->Tail)
	{
	  key_found = SL->compare_function (q->key, key) == 0 ? TRUE : FALSE;
	  last_identical = key_found ? q : NULL;
	}
      if (!key_found)
	return (NULL);
    }

  return (last_identical->value);
}


void
skiplists_traverse (const Skiplists SL,
		    const SkiplistsTraverseFunc traverse_function, void *data)
{
  SkiplistsNode p;

  if (SL->size > 0)
    for (p = SL->L->Head->forward[0]; p != SL->L->Tail; p = p->forward[0])
      if (traverse_function (p->key, p->value, data))
	break;
}


int
skiplists_size (const Skiplists SL)
{
  return (SL->size);
}


void
skiplists_allow_duplicates (const Skiplists SL, const int duplicate_rule)
{
  SL->allow_duplicates = TRUE;
  SL->duplicate_rule = duplicate_rule;
}


void
skiplists_disallow_duplicates (const Skiplists SL,
			       const SkiplistsDuplicateFunc
			       duplicate_function)
{
  SL->allow_duplicates = FALSE;
  SL->duplicate_function = duplicate_function;
}


static int
count_bits (int n)
{
  int bits;

  for (bits = 1; (n >>= 1) > 0; bits++);

  return bits;
}
