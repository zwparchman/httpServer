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

typedef struct {
  void ** dat;
  int len;
  int cap;
} Array;
void push_back_Array( Array * a , void * item );
void** begin_Array( Array a  );
void** end_Array( Array a );
void clean_shallow_Array( Array * );
void clean_deep_array( Array *);


typedef struct {
  char * dat;
  int len;
} String;

void make_String( String * s , const char * in );
void make_binary_String( String *s, const char *in , int i );
void binary_append_String( String *s , char * data, int i);
void concat_inplace_String( String * d, const String s );
String concat_String( const String a , const String b );
void clean_String( String *s );
int cmp_String( const String s , const String b );
String copy_String( const String a );




#endif   /* ----- #ifndef types_INC  ----- */
