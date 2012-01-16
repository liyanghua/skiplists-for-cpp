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
  Module: skiplists.h
  Version: 0.11
  Date: April 10, 2001
  Author: Chih-Hao Tsai
  Email: hao520@yahoo.com
  WWW: http://chtsai.org/
 */

#ifndef __SKIPLISTS_H__
#define __SKIPLISTS_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define SKIPLISTS_ALLOW_DUPLICATES
#define SKIPLISTS_MAX_NUMBER_OF_LEVELS 18
#define SKIPLISTS_INVERSE_P 2	/* p = .50 */
#define SKIPLISTS_TRAVERSE_CONTINUE 0
#define SKIPLISTS_TRAVERSE_TERMINATE 1
#define LAST_IN_FIRST_OUT 0
#define FIRST_IN_FIRST_OUT 1


/* Skiplists data structures */

  typedef char boolean;
  typedef void *SkiplistsKey;
  typedef void *SkiplistsValue;


  typedef struct skiplists_random_level_structure SkiplistsRandomStructure;
  typedef SkiplistsRandomStructure *SkiplistsRandom;
  struct skiplists_random_level_structure
  {
    int max_number_of_levels;
    int max_level;
    int inverse_p;
    int randoms_left;
    int random_bits;
    int bits_per_use;
    int bits_in_random;
  };


  typedef struct skiplists_node_structure SkiplistsNodeStructure;
  typedef SkiplistsNodeStructure *SkiplistsNode;
  struct skiplists_node_structure
  {
    char level;
    void *key;
    void *value;
    SkiplistsNode *forward;
  };


  typedef struct skiplists_list_structure SkiplistsListStructure;
  typedef SkiplistsListStructure *SkiplistsList;
  struct skiplists_list_structure
  {
    char level;
    SkiplistsNode Head;
    SkiplistsNode Tail;
    SkiplistsNode *finger;
  };

  typedef int (*SkiplistsCompareFunc) (SkiplistsKey, SkiplistsKey);
  typedef void *(*SkiplistsDuplicateFunc) (SkiplistsValue, SkiplistsValue);
  typedef int (*SkiplistsTraverseFunc) (SkiplistsKey, SkiplistsValue, void *);

  typedef struct skiplists_structure SkiplistsStructure;
  typedef SkiplistsStructure *Skiplists;
  struct skiplists_structure
  {
    int size;
    boolean init_fingers;
    boolean allow_duplicates;
    int duplicate_rule;
    SkiplistsCompareFunc compare_function;
    SkiplistsDuplicateFunc duplicate_function;
    SkiplistsRandom R;
    SkiplistsList L;
  };


/* Skiplists public functions */

  Skiplists skiplists_new (const SkiplistsCompareFunc);
  void skiplists_destroy (Skiplists);
  boolean skiplists_remove (const Skiplists, const SkiplistsKey);
  SkiplistsValue skiplists_lookup (const Skiplists, SkiplistsKey);
  SkiplistsValue skiplists_lookup_with_finger (const Skiplists,
						SkiplistsKey);
  boolean skiplists_insert (const Skiplists, SkiplistsKey, SkiplistsValue);
  void skiplists_traverse (const Skiplists, const SkiplistsTraverseFunc,
			   void *);
  int skiplists_size (const Skiplists);
  void skiplists_allow_duplicates (const Skiplists, const int);
  void skiplists_disallow_duplicates (const Skiplists,
				      const SkiplistsDuplicateFunc);

#ifdef __cplusplus
}
#endif				/* extern "C" */

#endif				/* __SKIPLISTS_H__ */
