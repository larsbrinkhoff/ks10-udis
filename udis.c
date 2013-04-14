#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	     int length,
	     struct ks10_uinsn *ram)
{
  int i, address;

  for (address = 0; address < length; address++)
    {
      for (i = 0; i < field_num; i++)
	ram[address].field[i] = bits (word[address], &fields[i].bits);
    }
}

static void
print (int address, int print_J, struct ks10_uinsn *insn)
{
  int i, n, printed = 0;

  if (address >= 0)
    printf ("%04o:", address);
  putchar ('\t');

  n = 0;
  for (i = 0; i < field_num; i++)
    {
      int value = insn->field[i];

      if (fields[i].name[0] == ' ')
	continue;
      if (strcmp (fields[i].name, "J") == 0 && !print_J)
	continue;

      if (value != fields[i].def_ault &&
	  !(strcmp (fields[i].name, "RSRC") == 0 &&
	    value == get_field ("LSRC", insn)) &&
	  fields[i].print_field (i, insn))
	{
	  if (printed)
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

	  printed = 1;
	}
    }

  printf ("\n");
}

void
udis (int length, unsigned char word[RAM_SIZE][WORD_SIZE])
{
  int address;
  int words_done;
  int print_address = 1;

  disassemble (word, length, ram);

#ifdef PROGRAM_ORDER
  address = 0;
  words_done = 0;
  while (words_done < length)
#else
  for (address = 0; address < length; address++)
#endif
    {
      print (print_address ? address : -1,
	     ram[get_field ("J", &ram[address])].done,
	     &ram[address]);
      words_done++;
      ram[address].done = 1;

#ifdef PROGRAM_ORDER
      if (get_field ("SKIP", &ram[address]) != 070)
	{
	  int new_address = get_field ("J", &ram[address]);
	  if (!ram[new_address].done)
	    {
	      printf ("=0\n");
	      print (new_address, 1, &ram[new_address]);
	      words_done++;
	      ram[new_address].done = 1;
	      address = new_address + 1;
	      if (ram[address].done)
		address = get_field ("J", &ram[new_address]);
	      continue;
	    }
	}
#endif

      address = get_field ("J", &ram[address]);
      print_address = 0;

#ifdef PROGRAM_ORDER
      if (ram[address].done)
	{
	  int i;
	  address = -1;
	  for (i = 0; i < length; i++)
	    {
	      if (!ram[i].done)
		{
		  address = i;
		  print_address = 1;
		  break;
		}
	    }
	  if (address == -1 && words_done < length)
	    abort ();
	}
#endif
    }
}
