#include "types.h"
#include <assert.h>
char * List_type_char_pp = "char **";
char * List_type_List_p = "List *";

List * add_front_List( void * elem , char * type , List * lst ){
  List *ret = malloc( sizeof(List) );
  ret->next = lst;
  ret->data = elem;
  ret->type = type;

  //ensuere the types have the same pointers,
  // + lexical equivlence is not enough
  if ( lst != NULL && type != lst->type ){
    LOG("Type error");
  }

  if( NULL == ret ){
    LOG("error");
  }
  return ret;
}

List * shallow_copy_List ( List * lst ){
  List * reverse = NULL ;
  List * ret = NULL ;

  List * a = lst;
  while( a != NULL ){
    reverse = add_front_List( a , List_type_List_p , reverse );
    a = a->next;
  }


  a = reverse;
  //first element
  while( a != NULL ){
    assert( a != NULL );
    assert( a->data != NULL );
    List * dat = a->data;

    ret = add_front_List( dat->data , dat->type , ret );
    a = a->next;
  }

  free_copy_List( reverse );
  return ret;
}
void free_copy_List( List * lst ){
  while( lst != NULL ){
    List * n = lst->next;
    free( lst );
    lst = n ;
  }
}

void free_List(List * lst ){

  while(  lst != NULL ){
    if( lst->data != NULL ){
      free( lst->data );
      List * n = lst->next;
      free( lst );
      lst = n;
    }
  }
}


void freeRequest_header( Request_header r ){
  if( r.headers != NULL ){
    int done = 0;
    for( int i=0; !done ; i++ ){
      for( int j=0; j < 2 ; j++){
        if( r.headers[i][j] == NULL ){ // invalid read size 8
          done = 1;
        } else {
          free( r.headers[i][j] );
        }
      }
      free( r.headers[i] );
    }
    free ( r.headers );
  }

  if( r.uri != NULL ) free(r.uri);
  if( r.raw != NULL ) free(r.raw);
}


void free_MeathodReturn( MeathodReturn m ){
  if( m.simple != NULL ){
    free( m.simple[0] );
    free( m.simple[1] );
    free( m.simple[2] );
    free( m.simple );
  }
  if( m.raw != NULL ){
    free( m.raw );
  }
  m.rawLen = -1;
}
