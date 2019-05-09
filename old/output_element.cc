#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>

const char* FMT() { return "%lg/"; }
int final_OFFSET() { return -1; }

template <typename T>
int output_element(T* p, int idx, char* buf, int &offset, int chremain , const char* (*fmt)())
{
    const T& src = p[ idx ];
    int headroom = chremain - 1 ;
    int i = headroom ;
    if (headroom > 0) { 
	i = snprintf(buf + offset, headroom ,fmt(), src);
    }
    if  (i >= headroom) {
        int ellipsis = 3;
        int last = offset + headroom;
        buf [last] = '\0';
        while (--ellipsis >= 0 && --last >= 0) { buf[last] = '.'; }
        return 0;
    }
    offset += i;
    return 1;
}

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))

int 
main(int argc,char *argv[])
{
  double values [3] = {3e300,4,5}; 
  char buf[ ] = { 'x', 'x', 'x', 'x', 'x',
                  'x', 'x', 'x', 'x', 'x',
                  'x', 'x', 'x', 'x', 'x',
                  'x', 'x', 'x', 'x', 'x' };

if (argc > 1) {
   char *endptr = 0;
   double val = strtod( argv[1], &endptr );
   if (endptr > argv[1]) values[0] = val;
}
  size_t bufsize = 15;
  if (bufsize < ARRAY_SIZE(buf)) buf[bufsize] ^= 0x20;
  int idx = 0;
  int  ofs = 0;
  int stat = 0;

  while (idx < ARRAY_SIZE(values)) 
  {
    stat = output_element( values, idx++, buf, ofs, bufsize - ofs, FMT );
    if (stat == 0) break;
  }
  if (stat) buf [ ofs + final_OFFSET() ] = '\0';

  for (int i=0;i<sizeof(buf);i++) {
      printf("%03o ",buf[i]);
  }
  printf("\n");
  for (int i=0;i<sizeof(buf);i++) {
      int ch = buf[i];
      printf("%3c ",isprint(ch)?ch:' ');
  }
  printf("\n");
  return 0;
}
