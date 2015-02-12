#include "utility.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 


void stripDotDotSlash( char * s ){
  char * dst = s;
  char * src = s;

  while( *src == '.'){
    if( strncmp( src, "../", 3 ) == 0 ){
      src+=3;
    }
  }
  while( *src != '\0' ){
    if( *src == '/' ){
      if( strncmp( src , "/../", 4 ) == 0 ){
        //we are at the start of a /../
        src += 3 ;
      } 
    }
    *dst = *src ;
    dst ++;
    src ++;
  }

  *dst = *src;

}

char * nstrMemCopy( char * cpy , int n){
  char * ret = malloc( 1+n );
  strncpy( ret , cpy , n-1 );
  ret[n]=0;
  return ret ;

}

char * strMemCopy( char const * const cpy ){
  if( cpy == NULL ) LOG("Error");
  char * ret = malloc( 1+strlen( cpy ) );
  strcpy( ret , cpy );
  return ret ;
}

void readNewLongString_delimited( int fileDescriptor, const char delimiter , char ** output ){
  char c;

  char *buff ; 
  unsigned int size = 1024;
  unsigned int used = 0;
  buff = malloc( size );
  if( buff == NULL ){
    perror("buff was null in readNewLongString_delimited : first ");
    exit(1);
  }

  while (  1  ){
    int rcount = read( fileDescriptor, &c , 1 );
    if( rcount < 0 ){
      perror("reading failed in utility_readNewLongString_delimited\n");
    }

    if( used +1 >= size ){
      //make more room
      buff = realloc( buff , size * 2 );
      if( buff == NULL ){
        perror("buff was null in readNewLongString_delimited : third ");
        exit(1);
      }

      size = size * 2;
    }

    if( c == delimiter ){
      buff[used]=0;
      buff = realloc( buff , used+1 );
      if( buff == NULL ){
        perror("buff was null in readNewLongString_delimited : second ");
        exit(1);
      }

      (*output) = buff;
      return;
    } else {
      buff[used]=c;
      used ++;
    }
  }


  *output  = buff;
  
}


void writeString_nullTerminated( int fileDescriptor , char * str ){
  int toWrite = strlen( str );

  int n ;
  do{
    n = send( fileDescriptor , str , toWrite, MSG_NOSIGNAL);
    toWrite -= n;
    str += n;
    if( n<0 ){
      LOG("Broken pipe detected. Aborting write");
      return;
    }
  } while ( toWrite > 0 );
}


void writeCount( int fileDescriptor , char * dat , int count ){
  while( count > 0 ){
    int r = write( fileDescriptor , dat , count );
    count = count - r;
    dat += r;
  }
}

