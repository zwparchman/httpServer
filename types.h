#ifndef  types_INC
#define  types_INC
#include <stdlib.h>
#include "utility.h"

extern char * List_type_char_pp;
typedef struct List_t {
  struct List_t * next;
  void * data;
  char * type ;
} List ;

List * add_front_List( void * elem ,char * type ,  List * lst );
List * shallow_copy_List ( List * lst );
void free_copy_List( List * lst );
void free_List ( List * lst );

typedef struct threadArg_t {
  int socket;
  void * threadStruct;
} threadArg ;

typedef struct MeathodReturn_t{

  char ** simple;
  char * raw;
  long rawLen;

} MeathodReturn ; 
void free_MeathodReturn( MeathodReturn m );


typedef struct {
  int meathod ;
  int majorVersion;
  int minorVersion;

  char *** headers; // type :: [ [name , value] ] , ends in [NULL , NULL ]
  char * uri;
  char * raw;

  char * errorString;
  int fail;

} Request_header;
void freeRequest_header( Request_header r );


#endif   /* ----- #ifndef types_INC  ----- */
