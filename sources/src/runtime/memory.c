#include <string.h>
#include <strings.h>
#include "debugger.h"
#include "fail.h"
#include "freelist.h"
#include "gc.h"
#include "gc_ctrl.h"
#include "major_gc.h"
#include "memory.h"
#include "minor_gc.h"
#include "misc.h"
#include "mlvalues.h"

value *c_roots_head;

/* Allocate more memory from malloc for the heap.
   Return a block of at least the requested size (in words).
   Return NULL when out of memory.
*/
static char *expand_heap (request)
     mlsize_t request;
{
  char *mem;
  asize_t malloc_request;
  unsigned long ind,indins,page_deb,page_fin;

  malloc_request = round_heap_chunk_size (Bhsize_wosize (request));
  gc_message ("Growing heap to %ldk\n",
	      (stat_heap_size + malloc_request) / 1024);
  mem = aligned_malloc (malloc_request + sizeof (heap_chunk_head),
                        sizeof (heap_chunk_head));
  if (mem == NULL){
    gc_message ("No room for growing heap\n", 0);
    return NULL;
  }
  mem += sizeof (heap_chunk_head);
  (((heap_chunk_head *) mem) [-1]).size = malloc_request;
  Assert (Wosize_bhsize (malloc_request) >= request);
  Hd_hp (mem) = Make_header (Wosize_bhsize (malloc_request), 0, Blue);

  /* Tout ce qui suit a été refait */

  page_deb = Page(mem);
  page_fin = Page (mem + malloc_request);

  indins = 0;
  if (mem >= heap_start) while (page_table[indins] < page_deb) indins=indins+2;
  for (ind = bout_page_table;ind > indins;ind=ind-2) 
    {
      page_table[ind]=page_table[ind-2];
      page_table[ind+1]=page_table[ind-1];
    }
  bout_page_table =bout_page_table+2;
  page_table[indins] = page_deb;
  page_table[indins+1] = page_fin;

  if (mem < heap_start)
    {
      for (ind=1;ind<2*bout_page_table;ind++)
	{
	  if (page_table[ind] < ULONG_MAX) page_table[ind] = page_table[ind] - page_table[0];
	}
     page_table[0]=0;
    }

  /*Là on a fini de mettre à jour la table des pages 
    mis en place des entêtes */


  #ifndef SIXTEEN    
  if (mem < heap_start){
      (((heap_chunk_head *) mem) [-1]).next = heap_start;
      heap_start = mem;
  } else {
      char **last;
      char *cur;
      
      if (mem >= heap_end) heap_end = mem + malloc_request;
      last = &heap_start;
      cur = *last;
      while (cur != NULL && cur < mem){
	  last = &((((heap_chunk_head *) cur) [-1]).next);
	  cur = *last;
      }
      (((heap_chunk_head *) mem) [-1]).next = cur;
      *last = mem;
  }
      
  #else

  /* Simplified version for the 8086 */
  {
    char **last;
    char *cur;
    
    last = &heap_start;
    cur = *last;
    while (cur != NULL && (char huge *) cur < (char huge *) mem){
      last = &((((heap_chunk_head *) cur) [-1]).next);
      cur = *last;
    }
    (((heap_chunk_head *) mem) [-1]).next = cur;
    *last = mem;
  }
#endif   

/* fin de la modification */


  stat_heap_size += malloc_request;
  return Bp_hp (mem);
}

int search_in_table(p)
unsigned long p;
{
  int debut=0;
  int milieu,fin;

  fin=bout_page_table/2;
  while (fin-debut > 2) {
      milieu  = (fin +debut)/2;
    if (page_table[2*milieu] <= p) debut = milieu;
    else fin=milieu;
  }
  /* on a p >=page_table[2*debut] et p < page_table[2*fin] et fin=debut + 1 ou 2 */
  if (page_table[2*debut+2] <= p) debut +=1;
    
  return(debut); 
}

int is_in_heap(p) 
  addr p;
{
unsigned long page;
  if  ((addr)(p) >= (addr)heap_start && (addr)(p) < (addr)heap_end) {
      page=Page(p);
      return(page < page_table[2*search_in_table(page)+1]);
      }
  else return(0);
}
  
      

value alloc_shr (wosize, tag)
     mlsize_t wosize;
     tag_t tag;
{
  char *hp, *new_block;

  hp = fl_allocate (wosize);
  if (hp == NULL){
    new_block = expand_heap (wosize);
    if (new_block == NULL) raise_out_of_memory ();
    fl_add_block (new_block);
    hp = fl_allocate (wosize);
  }

  Assert (Is_in_heap (Val_hp (hp)));

  if (gc_phase == Phase_mark || (addr)hp >= (addr)gc_sweep_hp){
    Hd_hp (hp) = Make_header (wosize, tag, Black);
  }else{
    Hd_hp (hp) = Make_header (wosize, tag, White);
  }
  allocated_words += Whsize_wosize (wosize);
  if (allocated_words > Wsize_bsize (minor_heap_size)) urge_major_slice ();
  return Val_hp (hp);
}

/* Use this function to tell the major GC to speed up when you use
   finalized objects to automatically deallocate extra-heap objects.
   The GC will do at least one cycle every [max] allocated words;
   [mem] is the number of words allocated this time.
   Note that only [mem/max] is relevant.  You can use numbers of bytes
   (or kilobytes, ...) instead of words.  You can change units between
   calls to [adjust_collector_speed].
*/
void adjust_gc_speed (mem, max)
     mlsize_t mem, max;
{
  if (max == 0) max = 1;
  if (mem > max) mem = max;
  extra_heap_memory += ((float) mem / max) * stat_heap_size;
  if (extra_heap_memory > stat_heap_size){
    extra_heap_memory = stat_heap_size;
  }
  if (extra_heap_memory > Wsize_bsize (minor_heap_size) / 2){
    urge_major_slice ();
  }
}

/* You must use [initialize] to store the initial value in a field of
   a shared block, unless you are sure the value is not a young block.
   A block value [v] is a shared block if and only if [Is_in_heap (v)]
   is true.
*/
/* [initialize] never calls the GC, so you may call it while an object is
   unfinished (i.e. just after a call to [alloc_shr].) */
void initialize (fp, val)
     value *fp;
     value val;
{
  *fp = val;
  Assert (Is_in_heap (fp));
  if (Is_block (val) && Is_young (val)){
    *ref_table_ptr++ = fp;
    if (ref_table_ptr >= ref_table_limit){
      realloc_ref_table ();
    }
  }
}

/* You must use [modify] to change a field of an existing shared block,
   unless you are sure the value being overwritten is not a shared block and
   the value being written is not a young block. */
/* [modify] never calls the GC. */
void modify (fp, val)
     value *fp;
     value val;
{
  Modify (fp, val);
}

char *stat_alloc (sz)
     asize_t sz;
{
  char *result = (char *) xmalloc (sz);

  if (result == NULL) raise_out_of_memory ();
  return result;
}

void stat_free (blk)
     char * blk;
{
  xfree (blk);
}

char *stat_resize (blk, sz)
     char *blk;
     asize_t sz;
{
  char *result = (char *) xrealloc (blk, sz);

  if (result == NULL) raise_out_of_memory ();
  return result;
}

void init_c_roots ()
{
  c_roots_head = NULL;
}
