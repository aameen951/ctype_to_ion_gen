#include <stdio.h>
#include <ctype.h>

typedef int fn_ptr(int);

typedef struct func_meta {
  char *flag_name;
  char *fn_name;
  fn_ptr *fn;
} func_meta;

func_meta meta[] = {
  {"ALNUM", "isalnum", isalnum},
  {"ALPHA", "isalpha", isalpha},
  {"BLANK", "isblank", isblank},
  {"CNTRL", "iscntrl", iscntrl},
  {"DIGIT", "isdigit", isdigit},
  {"GRAPH", "isgraph", isgraph},
  {"LOWER", "islower", islower},
  {"PRINT", "isprint", isprint},
  {"PUNCT", "ispunct", ispunct},
  {"SPACE", "isspace", isspace},
  {"UPPER", "isupper", isupper},
  {"XDIGIT", "isxdigit", isxdigit},
};
#define META_COUNT (sizeof(meta)/sizeof(meta[0]))

static unsigned short table[0x80] = {0};

int main()
{
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    unsigned short flag = (1 << m_idx);
    for(int i = 0; i<0x80; i++)
    {
      table[i] |= m->fn(i) ? flag : 0;
    }
  }
  for(int i = 0; i<0x80; i += 8)
  {
    for(int j = 0; j<8; j++)
    {
      int ch = i + j;
      for(int m_idx=0; m_idx < META_COUNT; m_idx++)
      {
        func_meta *m = meta + m_idx;
        int a = m->fn(ch) != 0;
        int b = (table[ch] & (1 << m_idx)) != 0;
        if(a != b)
        {
          printf("Error: %s(0x%02x) failed\n", m->fn_name, ch);
          abort(0);
        }
      }
    }
  }

  #define I "    "
  char OutputBuffer[0x10000];
  char *Out = OutputBuffer;


  // generate data table
  Out += sprintf(Out, "\n");
  Out += sprintf(Out, "const data_table: uint16 const[] = {\n");
  for(int i = 0; i<0x80; i += 16)
  {
    Out += sprintf(Out, I);
    for(int j = 0; j<16; j++)
    {
      int ch = i + j;
      Out += sprintf(Out, "0x%03x, ", table[ch]);
    }
    Out += sprintf(Out, "\n");
  }
  Out += sprintf(Out, "};\n");
  Out += sprintf(Out, "\n");

  // generate flags
  Out += sprintf(Out, "enum Flags{\n");
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    Out += sprintf(Out, I "%s," "\n", m->flag_name);
  }
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");

  // generate is-ascii function
  Out += sprintf(Out, "@inline\n");
  Out += sprintf(Out, "func isascii(c: char): bool {\n");
  Out += sprintf(Out, I "return (c & 0x80) ? false : true;" "\n");
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");

  // generate inline function
  Out += sprintf(Out, "@inline\n");
  Out += sprintf(Out, "func _is(c: char, f: Flags): bool {\n");
  Out += sprintf(Out, I "return isascii(c) && (data_table[c] & f) ? true : false;" "\n");
  Out += sprintf(Out, "}\n");
  Out += sprintf(Out, "\n");

  // generate functions
  for(int m_idx=0; m_idx < META_COUNT; m_idx++)
  {
    func_meta *m = meta + m_idx;
    Out += sprintf(Out, "func %s(c: char): bool {\n", m->fn_name);
    Out += sprintf(Out, I "return _is(c, %s);" "\n", m->flag_name);
    Out += sprintf(Out, "}\n");
  }

  // write to file
  FILE *f = fopen("gen/ctype.ion", "wb");
  if(f)
  {
    fwrite(OutputBuffer, Out-OutputBuffer, 1, f);
    fclose(f);
  }
}