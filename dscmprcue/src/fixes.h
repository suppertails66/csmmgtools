#ifndef FIXES_H
#define FIXES_H

/*
  CUE's otherwise excellent DS compression tools very unfortunately
  depend on some nonstandard Windows extensions of the C standard library.
  This file contains replacement functions for use on other systems.
*/

#ifndef _WIN32

  #include <stdio.h>
  #include <stdlib.h>
  #include <ctype.h>
  
  /* case-insensitive strcmp */
  int strcmpi(char* str1, char* str2) {
    /* doesn't remotely resemble the correct implementation but doesn't
       matter */
    
    while ((*str1) && (*str2)) {
      if (tolower(*str1++) != tolower(*str2++)) return 1;
    }
    
    return 0;
  }

  /* return size of file in bytes.
     original expects parameter to be an int from fileno() but i don't want
     to have to deal with that */
  unsigned int filelength(FILE* fp) {
    if (fp == NULL) return 0;
    
    long int pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long int end = ftell(fp);
    fseek(fp, pos, SEEK_SET);
    
    return (unsigned int)end;
  }
  
#endif

#endif
