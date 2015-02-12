
#ifndef  utility_INC
#define  utility_INC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define elog( M ) do{ \
  printf("%s %i : %s: %s\n",__FILE__,__LINE__, (M) , strerror(errno) );\
}while(0);


#define LOG(M) do {\
  printf("%s %i : %s \n",__FILE__,__LINE__,#M);\
}while(0)



//reads an arbitrarly long string from a filedescriptor until a delimiter is found
void readNewLongString_delimited( int fileDescriptor, const char delimiter , char ** output );
void writeString_nullTerminated( int fileDescriptor , char * );

void writeCount( int fileDescriptor , char * dat , int count );

//copy a string and return a new memory location
char * nstrMemCopy( char * cpy , int n);
char * strMemCopy( char const * const cpy );


void stripDotDotSlash( char * );
#endif   /* ----- #ifndef utility_INC  ----- */
