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

void push_back_Array( Array * a , void * item ){
  if( a->len >= a->cap ){
    if( a->cap == 0 ){ a->cap = 2 ; };
    a->cap *= 1.3 + 1;
    a->dat = realloc( a->dat , sizeof( void*) * a->cap );
  }
  a->dat[a->len] = item;
  a->len++;
}
void** begin_Array( Array a  ){
  return a.dat;
}
void** end_Array( Array a ){
  return ( a.dat + a.len );
}
void clean_shallow_Array( Array *a ){
  if( a->dat != NULL ){ free( a->dat ); }
  a->dat=NULL;
  a->len=0;
  a->cap=0;
}


void clean_deep_array( Array *a ){
  void ** d = a->dat;

  if( a->dat != NULL ){
    for( int i=0; i < a->len ; i ++ ){
      free( *d );
      d++;
    }
    free( a->dat );
  }

  a->dat = NULL;
  a->len = 0;
  a->cap = 0;
}

void make_String( String * s , const char * in ){
  if( s->dat != NULL ){ free( s->dat); };

  s->dat = strMemCopy( in );
  s->len = strlen( s->dat );
}


//TODO test
void binary_append_String( String *s , char * data, int i){
  int newsize = s->len + i ;
  s->dat = realloc(s->dat , newsize );
  if( s->dat == NULL ){ LOG("realloc error") ; exit(1); }

  memcpy( s->dat + s->len , data , i );
  s->len = newsize;
}

void make_binary_String( String *s, const char *in , int i ){
  if( s->dat != NULL ){ free( s->dat); };

  s->dat = malloc( sizeof( char ) * i );
  memcpy( s->dat, in , i );
  s->len = i;
}

int cmp_String( const String s , const String b ){
  if( s.dat == b.dat && b.dat == NULL ) return 0;
  if( s.dat == NULL ) return 1 ;
  if( b.dat == NULL ) return -1;
  int ml = s.len < b.len ? s.len : b.len;
  return memcmp( s.dat , b.dat , ml );
}

//TODO test
void concat_inplace_String( String * d, const String s ){
  char * where = d->dat + d->len;
  int newlen = d->len + s.len;

  d->dat = realloc( d->dat , newlen );
  if( d->dat == NULL ){ LOG("realloc error"); exit(1); }

  memcpy( where, s.dat, s.len );
  d->len = newlen;

}

String  concat_String( const String d , const String s ){
  String ret;
  ret.dat = calloc( d.len + s.len + 1 , 1 );
  int where = 0;
  if( d.len > 0 ){
    memcpy( ret.dat , d.dat , d.len );
    where += d.len;
  }
  if( s.len > 0 ){
    memcpy( ret.dat+where , s.dat, s.len);
    where += s.len;
  }

  ret.dat[ where ] = 0 ;
  ret.len = where;

  return ret;
}

void clean_String( String *s ){
  if(  s->dat != NULL ) { free ( s->dat ); }
  s->dat = NULL;
  s->len = 0;
}

String copy_String( const String a ){
  String b = {};
  String ret = {};

  ret = concat_String( a,b );

  return ret;
}
