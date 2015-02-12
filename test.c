#include <stdio.h>
#include <stdlib.h>
#include "statusCodes.h"
#include <string.h>
#include <strings.h>
#include "types.h"

#include "utility.h"

#define alog(A,B) do{ \
  if( (A) != (B) ){\
    printf("%s %i : %s != %s \n", __FILE__,__LINE__,#A,#B);\
    good = 0 ;\
  }\
}while(0)

extern char *  processHeader_get_uri( char * req , char ** out , int * success );
int test_processHeader_get_uri(){
  int good = 1;
  {
    char * req = "/file/from/the/gods HTTP/1.1\r\nheader : bla\r\n";
    char * o;
    char * uri ; 
    char *euri ="file/from/the/gods";
    char *eo = "HTTP/1.1\r\nheader : bla\r\n";
    int s;
    uri = processHeader_get_uri( req , &o , &s );

    alog( strcmp(uri,euri) , 0 );
    alog( strcmp( eo , o ) , 0 );
    alog( s , 1 );
    free ( uri );
  }
  {
    char * req = " /file with/space\r\nheader : bla\r\n" ;
    int s;
    char * o;
    char * uri = processHeader_get_uri( req , &o , &s );

    alog( uri , NULL );
    alog( strcmp( o , req ) , 0 );
    alog( s , 0 );
    free ( uri );
  }
  {
    char * req = "file HTTP/1.1\r\nheader : bla\r\n" ;
    int s;
    char * o;
    char * eo = "HTTP/1.1\r\nheader : bla\r\n";
    char * uri = processHeader_get_uri( req , &o , &s );
    char * euri = "file";

    alog( uri != NULL , 1 );
    alog( strcmp( euri , uri ) , 0 );
    alog( strcmp( eo , o ) , 0 );
    alog( s , 1 );
    free ( uri );
  }{
    char * req = "/ HTTP/1.1\r\nheader : bla\r\n" ;
    int s;
    char * o;
    char * eo = "HTTP/1.1\r\nheader : bla\r\n";
    char * uri = processHeader_get_uri( req , &o , &s );
    char * euri = "/";

    alog( uri != NULL , 1 );
    alog( strcmp( euri , uri ) , 0 );
    alog( strcmp( eo , o ) , 0 );
    alog( s , 1 );
    free ( uri );
  }{
    char * req = "/\r\nheader : bla\r\n" ;
    int s;
    char * o;
    char * eo = "\r\nheader : bla\r\n";
    char * uri = processHeader_get_uri( req , &o , &s );
    char * euri = "/";

    alog( uri != NULL , 1 );
    alog( strcmp( euri , uri ) , 0 );
    alog( strcmp( eo , o ) , 0 );
    alog( s , 1 );
    free ( uri );
  }{
    char * req = "/bob\r\nheader : bla\r\n" ;
    int s;
    char * o;
    char * eo = "\r\nheader : bla\r\n";
    char * uri = processHeader_get_uri( req , &o , &s );
    char * euri = "bob";

    alog( uri != NULL , 1 );
    alog( strcmp( euri , uri ) , 0 );
    alog( strcmp( eo , o ) , 0 );
    alog( s , 1 );
    free ( uri );
  }
  return good;
}

extern int processRequestType( char * req , char ** out , int * success );
int test_processRequestType(){
  int good = 1 ;
  {
    int s;
    char * req  = "gEt /stuff\r\n";
    char * v = req ;
    alog ( processRequestType( req , &v , &s ) , Meathod_Get );
    alog ( v , req + 4 );
    alog ( s , 1 );
  }
  {
    int s;
    char * req  = "PosT /stuff\r\n";
    char * v;
    int t = processRequestType( req , &v , &s );
    alog ( t , Meathod_Post );
    alog ( v , req + 5 );
    alog ( s , 1 );
  }
  {
    int s;
    char * req  = "HeAd /stuff\r\n";
    char * v;
    alog ( processRequestType( req , &v , &s ) , Meathod_Head );
    alog ( v , req + 5 );
    alog ( s , 1 );
  }
  {
    int s;
    char * req  = "GET/stuff\r\n";
    char * v;
    alog ( processRequestType( req , &v , &s ) , -1 );
    alog ( v , req );
    alog ( s , 0 );
  }
  {
    int s;
    char * req  = "POST/stuff\r\n";
    char * v;
    alog ( processRequestType( req , &v , &s ) , -1 );
    alog ( v , req );
    alog ( s , 0 );
  }
  {
    int s;
    char * req  = "HEAD/stuff\r\n";
    char * v;
    alog ( processRequestType( req , &v , &s ) , -1 );
    alog ( v , req );
    alog ( s , 0 );
  }
  return good;
}



int * processHeader_get_http_version( char * req , char ** out , int * success );
int test_processHeader_get_http_version(){
  int good = 1 ;


  {
    int s;
    char * req  = "hTtp/1.1\r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( rr[0] , 1 );
    alog( rr[1] , 1 );
    alog ( 0 == strcmp("foo : bar\r\n" , v ) , 1 );
    alog ( s , 1 );
    free( rr );
  }{
    int s;
    char * req  = "hTtp/1.0\r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( rr[0] , 1 );
    alog( rr[1] , 0 );
    alog (  0 == strcmp("foo : bar\r\n" , v ) , 1 );
    alog ( s , 1 );
    free( rr );
  }{
    int s;
    char * req  = "hTtp/1.a\r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( rr , NULL  );
    alog ( s , 0 );
  }{
    int s;
    char * req  = "hTtp/a.1\r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( rr , NULL  );
    alog ( s , 0 );
  }{
    int s;
    char * req  = "hTtp/1.1 \r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( NULL != rr , 1  );
    alog( rr[0] , 1 );
    alog( rr[1] , 1 );
    alog( strcmp( v , "foo : bar\r\n" ) , 0 );
    alog ( s , 1 );
    free( rr );
  }{
    int s;
    char * req  = "\r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( NULL != rr , 1  );
    alog( rr[0] , 0 );
    alog( rr[1] , 9 );
    alog( strcmp( v , "foo : bar\r\n" ) , 0 );
    alog ( s , 1 );
    free( rr );
  }{
    int s;
    char * req  = " \r\nfoo : bar\r\n";
    char * v;
    int * rr = processHeader_get_http_version( req , &v , &s ) ;
    alog( rr , NULL  );
    alog ( s , 0 );
  }


  return good;
}


extern char *** processHeader_parse_options( char * req , char ** out , int * success );
int test_processHeader_parse_options(){
  int good = 1 ;

  {
    char * req = "host: localhost:12001\r\nConnection: keep-alive\r\n";
    char * o;
    char * eo = "\r\n";
    int s;

    char *** e  = malloc( sizeof( char ** ) * 3 );
    e[0] = malloc( sizeof( char * ) * 2 );
    e[0][0] = "host";
    e[0][1] = "localhost:12001";
    e[1] = malloc( 2 * sizeof( char * ) );
    e[1][0] = "Connection";
    e[1][1] = "keep-alive";
    e[2] = malloc( 2 * sizeof ( char * ) );
    e[2][0] = NULL;
    e[2][1] = NULL;

    char *** ret = processHeader_parse_options( req , &o , &s );

    alog( strcmp( o , eo ) , 0 );
    for( int i=0; i < 2 ; i ++ ){
      for( int j =0 ; j <2 ; j ++ ){
        alog( strcmp( ret[i][j] , e[i][j] ) , 0 );
        free( ret[i][j] );
      }
      free(ret[i]);
      free( e[i] );
    }

    for( int i = 0 ; i<2 ; i ++ ){
      alog( ret[2][i] , NULL );
    }

    free( ret[2] );
    free( e[2] );

    free (ret );
    free ( e );
    alog( s , 1 );
  }

  return good;
}

extern char* getTime();
int test_getTime(){
  int good = 1;

  char * out = getTime();
  //printf("%s\n",out);
  free(out);

  return good;
}


extern char * get_header_field_value( Request_header h , const char * toFind );
int test_get_header_field_value(){
  int good = 1;

  {
    Request_header h = {0};
    int s;
    char * o;
    char * req = "host: localhost:12001\r\nConnection: keep-alive\r\n";

    h.headers = processHeader_parse_options( req , &o , &s );

    char * hst = get_header_field_value( h , "host" );
    alog( strcasecmp(hst , "localhost:12001" ) , 0 );

    freeRequest_header( h );
    free( hst );
  }


  return good;
}

extern char * unencode(char *);
int test_unencode(){
  int good = 1;

  {
    char * i = unencode ( "bob+goes" );
    char * eo = "bob goes";
    alog( strcmp( i , eo ) , 0 );
    free( i );
  }

  {
    char *i = unencode("bob%20goes");
    char * eo = "bob goes";
    alog( strcmp( i , eo ) , 0 );
    free( i );
  }

  {
    char *i = unencode("space+plus%2B");
    char * eo = "space plus+";
    alog( strcmp( i , eo ) , 0 );
    free( i );
  }

  return good;
}

int test_shallow_copy_List ( ){
  int good=1; 

  List *lst = NULL;

  char * tt = "int_p";
  int* val1=malloc( sizeof(int));
  int* val2=malloc( sizeof(int));
  *val1=1;
  *val2=2;

  lst = add_front_List( val1, tt , lst );
  lst = add_front_List( val2, tt , lst );

  List * copy = shallow_copy_List( lst );

  alog( *((int*) (lst->data)) , 2 );
  alog( *((int*) (lst->next->data) ) , 1 );

  alog( *((int*) (copy->data)) , 2 );
  alog( *((int*) (copy->next->data) ) , 1 );

  free_copy_List( copy );
  free_List( lst );
  
  return good;


}
int test_add_front_List(){
  int good=1;

  char *tt = "int_p";

  int *val1= malloc(sizeof(int) );
  int *val2= malloc(sizeof(int) );

  *val1 = 1;
  *val2 = 2;

  List e1;
  e1.next= NULL;
  e1.data =  (void *) val1;
  e1.type = tt;


  List * lst = NULL;
  lst = add_front_List( val1 , tt , lst );

  alog( memcmp( lst , &e1 , sizeof( List ) ) , 0 );

  List e2;
  e2.next = lst;
  e2.data = (void *) val2;
  e2.type = tt;

  lst = add_front_List( val2 , tt , lst );

  alog( memcmp( lst, &e2 , sizeof( List ) ) , 0 );

  free_List( lst );

  return good;
}

int test_stripDotDotSlash(){
  int good=1;

  {
    char * e = "blaekd";
    char * s = strMemCopy( e );
    alog( strcmp( s , e ) , 0 );
    free ( s );
  }{
    char * given = "blaekd/../";
    char * e = "blaekd/";
    char * s = strMemCopy( given );
    stripDotDotSlash( s );
    alog( strcmp( s , e ) , 0 );
    free ( s );
  }{
    char * given = "blaekd/../f";
    char * e = "blaekd/f";
    char * s = strMemCopy( given );
    stripDotDotSlash( s );
    alog( strcmp( s , e ) , 0 );
    free ( s );
  }{
    char * given = "../blaekd";
    char * e = "blaekd";
    char * s = strMemCopy( given );
    stripDotDotSlash( s );
    alog( strcmp( s , e ) , 0 );
    free ( s );
  }{
    char * given = "../../blaekd/../box";
    char * e = "blaekd/box";
    char * s = strMemCopy( given );
    stripDotDotSlash( s );
    alog( strcmp( s , e ) , 0 );
    free ( s );
  }

  return good;
}

int test_make_String(){
  int good=1;

  String s={0};
  make_String( &s , "bob" );

  alog( s.len , 3 );
  alog( strcmp(s.dat,"bob") , 0 );
  clean_String(&s);

  return good;
}

int test_make_binary_String(){
  int good=1;

  String s={0};

  char b[] = {1,2,3,4};

  make_binary_String(&s, b, 4 );

  alog( s.len , 4 );
  alog( s.dat[0] , 1 );
  alog( s.dat[1] , 2 );
  alog( s.dat[2] , 3 );
  alog( s.dat[3] , 4 );

  clean_String(&s);

  return good;
}

int test_binary_append_String(){
  int good=1;

  String s={0};
  make_String(&s , "t" );
  binary_append_String( &s, "\n", 2 );

  alog( s.len , 4 );
  alog( s.dat[0] , 't');
  alog( s.dat[1] , 0);
  alog( s.dat[2] , '\n');
  alog( s.dat[3] , 0);

  clean_String( &s );

  return good;
}

int test_String(){
  int good=1;

  alog( test_make_String(), 1 );
  alog( test_make_binary_String(), 1 );


  /* TODO test these
  void concat_inplace_String( String * d, const String s );
  String concat_String( const String a , const String b );
  void clean_String( String *s );
  int cmp_String( const String s , const String b );
  String copy_String( const String a );
  */



  return good;
}



void test(){
  if ( test_processRequestType() 
      && test_processHeader_get_uri()  
      && test_processHeader_get_http_version()
      && test_processHeader_parse_options() 
      && test_getTime()
      && test_get_header_field_value()
      && test_unencode()
      && test_add_front_List()
      && test_shallow_copy_List()
      && test_stripDotDotSlash()
      && test_String()
      ){
    //exit(0);
    return;
  } else {
    exit(1);
  }

}

