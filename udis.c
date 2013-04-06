#include <stdio.h>
#include <stdlib.h>
#include "udis.h"

#define PROGRAM_ORDER

#define lengthof(A) ((int)(sizeof (A) / sizeof (A)[0]))

static struct ks10_uinsn ram[RAM_SIZE];

static int
addbit (int value, unsigned char *word, int index)
{
  value <<= 1;
  value += (word[index / 8] >> (7 - (index % 8))) & 1;
  return value;
}

static int
bits (unsigned char *word, union bits *bits)
{
  int value = 0;
  int i, j;

  if (bits->range.first >= 0)
    {
      for (i = bits->range.first; i <= bits->range.last; i++)
	{
	  value = addbit (value, word, i);
	}
    }
  else
    {
      for (j = 0; bits->list.list[j] >= 0; j++)
	{
	  i = bits->list.list[j];
	  value = addbit (value, word, i);
	}
    }

  return value;
}

static void
disassemble (unsigned char word[RAM_SIZE][WORD_SIZE],
	     struct ks10_uinsn *ram)
{
  int i, address;

  for (address = 0; address < RAM_SIZE; address)
    {
      for (i = 0; i < field_num; i++)
	{
	  int value = bits (word, &fields[i].bits);

	  ((int *)&ram[address])[fields[i].field_index] = value;
	}
    }
}

static void
print (int address, ks10_uinsn *insn)
{
  int i, n;

  if (address >= 0)
    printf ("%04o:", address);
  putchar ('\t');

  n = 0;
  LSRC = -1;
  for (i = 0; i < field_num; i++)
    {
      int value = bits (word, &fields[i].bits);

	}

      if (value != fields[i].def_ault &&
	  !(strcmp (fields[i].name, "RSRC") == 0 &&
	    value == LSRC))
	{
	  if (i > 0)
	    n += printf (", ");

	  if (n >= 40)
	    {
	      printf ("\n\t ");
	      n = 0;
	    }

	  n += printf ("%s/", fields[i].name);
	  if (fields[i].names)
	    n += printf ("%s", fields[i].names[value]);
	  else
	    n += printf ("%o", value);
	}
    }

  printf ("\n");
}

void
udis (int length, unsigned char word[RAM_SIZE][WORD_SIZE], int *word_done)
{
  int address;
  int words_done;
  int print_address = 1;

  disassemble (word, &ram);

#ifdef PROGRAM_ORDER
  address = 0;
  words_done = 0;
  while (words_done < length)
#else
  for (address = 0; address < length; address++)
#endif
    {
      print (print_address ? address : -1, &ram[address]);
      words_done++;
      word_done[address] = 1;

#ifdef PROGRAM_ORDER
      if (SKIP != 070 && !word_done[J])
	{
	  int skip_J, skip_SKIP;
	  disassemble (J, word[J], &skip_J, &skip_SKIP);
	  words_done++;
	  word_done[J] = 1;
	  J++;
	  if (word_done[J])
	    J = skip_J;
	}
#endif

#ifdef PROGRAM_ORDER
      if (word_done[J])
	{
	  int i;
	  address = -1;
	  for (i = 0; i < length; i++)
	    {
	      if (!word_done[i])
		{
		  address = i;
		  print_address = 1;
		  break;
		}
	    }
	  if (address == -1 && words_done < length)
	    abort ();
	}
      else
	{
	  address = J;
	  print_address = 0;
	}
#endif
    }
}
