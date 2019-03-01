#include <stdio.h>
#include <ctype.h>

typedef int fn_ptr(int);

typedef struct func_meta {
  char *flag_name;
  char *fn_name;
  fn_ptr *fn;
} func_meta;

func_meta meta[] = {
  {"BLANK", "isblank", isblank},
  {"CNTRL", "iscntrl", iscntrl},
  {"DIGIT", "isdigit", isdigit},
  {"UPPER", "isupper", isupper},
  {"LOWER", "islower", islower},
  {"PRINT", "isprint", isprint},
  {"PUNCT", "ispunct", ispunct},
  {"SPACE", "isspace", isspace},
  {"XDIGIT", "isxdigit", isxdigit},
};
#define META_COUNT (sizeof(meta)/sizeof(meta[0]))

typedef struct composite_func_meta {
  char *flag;
  char *flags;
  char *fn_name;
  fn_ptr *fn;
} composite_func_meta;

int my_isascii(int c)
{
  return (c & 0x80) == 0;
}

composite_func_meta composite_meta[] = {
  {"ALPHA", "UPPER | LOWER", "isalpha", isalpha},
  {"ALNUM", "ALPHA | DIGIT", "isalnum", isalnum},
  {"GRAPH", "PUNCT | ALNUM", "isgraph", isgraph},
  {"ASCII", "CNTRL | PRINT", "isascii", my_isascii},
};
#define COMPOSITE_META_COUNT (sizeof(composite_meta)/sizeof(composite_meta[0]))


static unsigned short table[0x100] = {0};

int main()
{
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    unsigned short flag = (1 << m_idx);
    for(int i = 0; i<0x100; i++)
    {
      table[i] |= m->fn(i) ? flag : 0;
    }
  }

  #define I "    "
  char OutputBuffer[0x10000];
  char *Out = OutputBuffer;

  int first_zero = 0x100;
  while(first_zero > 0 && table[first_zero-1] == 0)first_zero--;

  // generate data table
  Out += sprintf(Out, "\n");
  Out += sprintf(Out, "var data_table: uint16 const[0x100] = {\n");
  int done = 0;
  for(int i = 0; i<0x100; i += 16)
  {
    for(int j = 0; j<16; j++)
    {
      int ch = i + j;
      if(ch >= first_zero)
      {
        if(j)Out += sprintf(Out, "\n");
        done = 1;
        break;
      }
      Out += sprintf(Out, "%s0x%03x, ", !j?I:"", table[ch]);
    }
    if(done)break;
    Out += sprintf(Out, "\n");
  }
  if(first_zero < 0x100)Out += sprintf(Out, I "// [0x%02x to 0xff] are zeros\n", first_zero);
  Out += sprintf(Out, "};\n");
  Out += sprintf(Out, "\n");

  // generate flags
  Out += sprintf(Out, "enum Flags {\n");
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    Out += sprintf(Out, I "%s = 1 << %d," "\n", m->flag_name, m_idx);
  }
  for(int m_idx=0; m_idx < COMPOSITE_META_COUNT; m_idx++)
  {
    composite_func_meta *m = composite_meta + m_idx;
    Out += sprintf(Out, I "%s = %s," "\n", m->flag, m->flags);
  }
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");

  // generate inline function
  Out += sprintf(Out, "@inline\n");
  Out += sprintf(Out, "func isclass(c: int, f: Flags): bool {\n");
  Out += sprintf(Out, I "return data_table[uint8(c)] & f != 0;" "\n");
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");

  // generate functions
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    Out += sprintf(Out, "@inline\n");
    Out += sprintf(Out, "func %s(c: char): bool {\n", m->fn_name);
    Out += sprintf(Out, I "return isclass(c, %s);" "\n", m->flag_name);
    Out += sprintf(Out, "}\n");
  }
  for(int m_idx=0; m_idx < COMPOSITE_META_COUNT; m_idx++)
  {
    composite_func_meta *m = composite_meta + m_idx;
    Out += sprintf(Out, "@inline\n");
    Out += sprintf(Out, "func %s(c: char): bool {\n", m->fn_name);
    Out += sprintf(Out, I "return isclass(c, %s);" "\n", m->flags);
    Out += sprintf(Out, "}\n");
  }

  // write to file
  FILE *f = fopen("gen/ctype.ion", "wb");
  if(f)
  {
    fwrite(OutputBuffer, Out-OutputBuffer, 1, f);
    fclose(f);
  }

  Out = OutputBuffer;
  Out += sprintf(Out, "import libc\n");
  Out += sprintf(Out, "\n");
  Out += sprintf(Out, "func libc_isascii(c: char):bool {\n");
  Out += sprintf(Out, I "return uint8(c) & 0x80 == 0;\n");
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");
  Out += sprintf(Out, "func main() {\n");
  Out += sprintf(Out, I "for(i:=0; i<0x100; i++) {" "\n");
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    Out += sprintf(Out, I I "#assert(%s(i) == bool(libc.%s(i)));" "\n", m->fn_name, m->fn_name);
  }
  for(int m_idx=0; m_idx < COMPOSITE_META_COUNT; m_idx++)
  {
    composite_func_meta *m = composite_meta + m_idx;
    if(m_idx+1 == COMPOSITE_META_COUNT)
      Out += sprintf(Out, I I "#assert(%s(i) == bool(libc_%s(i)));" "\n", m->fn_name, m->fn_name);
    else
      Out += sprintf(Out, I I "#assert(%s(i) == bool(libc.%s(i)));" "\n", m->fn_name, m->fn_name);
  }
  Out += sprintf(Out, I "}" "\n");
  Out += sprintf(Out, "}\n");


  // write to file
  f = fopen("gen/test.ion", "wb");
  if(f)
  {
    fwrite(OutputBuffer, Out-OutputBuffer, 1, f);
    fclose(f);
  }
}