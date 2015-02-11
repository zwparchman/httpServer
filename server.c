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
#include "utility.h"
#include <pthread.h>
#include <assert.h>
#include <strings.h>

#include "types.h"
#include "statusCodes.h"
#include <ctype.h>
#include "utility.h"


MeathodReturn internalError( Request_header h , int reason );
void * serve_client(void * );
extern void test();

void startServer( int port );
int main(int argc , char *argv[]){
  test();

  if( argc == 1 ){
    startServer( 12000 );
  } else if ( argc == 2 ){
    startServer( atoi( argv[1] ) );
  } else {
    printf ("Invalid number of arguments\n");
  }
  return 0;
}

void startServer( int port ){
  //for listening for a new client
  int listen_fd  = socket(  AF_INET , SOCK_STREAM , 0 );

  int yes = 1;
  if ( setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) {
      perror("setsockopt");
  }

  //for client communication
  int sock  ;

  {
    struct sockaddr_in lsockaddr = {0};
    lsockaddr.sin_family = AF_INET;
    lsockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    for( int i=0; i<10 ; i ++ ){
      lsockaddr.sin_port = htons( port+i );
      if( bind(listen_fd , ( __CONST_SOCKADDR_ARG )&lsockaddr , sizeof( lsockaddr )) != 0 ){
        printf("An error occured when binding\n");
        printf("Trying port %i\n" , port + 1 + i );
      } else {
        break;
      }
      if( i == 9 ){
        exit(1);
      }
    }

    if( listen(listen_fd , 1 ) < 0 ){
      printf("An erro occured when setting up listening\n" );
    }
  }

  while ( 1 ) {
    //get a new connection
    sock = accept( listen_fd , NULL , NULL );
    int err;

    pthread_t *thread = malloc( sizeof( pthread_t ) );
    pthread_attr_t attr;
    threadArg *args = malloc( sizeof( threadArg ) );
    args -> socket = sock;
    args -> threadStruct = thread;

    err = pthread_attr_init( &attr );
    if( err != 0 ){
      errno = err;
      elog("attr_init");
    }
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED );
    if( err != 0 ){
      printf( "%s %s\n" , strerror( errno ), "pthread_attr_setdetachedstate(&attr, PTHREAD_CREATE_DETACHED );" );
    }

    err = pthread_create( thread , &attr , serve_client , args );
    if ( err != 0 ){
      errno = err;
      elog("pthread_create");
    }

  }

  return;
}



int processRequestType( char * req , char ** out , int * success ){
  assert( out != NULL );
  *success = 1;
  (*out) = req ;
  int ret;

  if( ! req || strlen( req ) == 0 ){ goto error; }

  switch( req[0] ){
    case 'g': case 'G' :
      if( 0 == strncasecmp( req , "get " , 4 )  ) {
        ret = Meathod_Get ;
        (*out) += 4;
      } else {
        goto error;
      }
      break;

    case 'p': case 'P':
      if( 0 == strncasecmp( req , "post " , 5 )  ) {
        ret = Meathod_Post ;
        (*out) += 5;
      } else {
        goto error;
      }
      break;

    case 'h': case 'H':
      if( 0 == strncasecmp( req , "head " , 5 )  ) {
        ret = Meathod_Head;
        (*out) += 5;
      } else {
        goto error;
      }
      break;
    default :
      goto error;
  }

  return ret;
error: 
       *success=0;
       return -1 ;
}

char * processHeader_get_uri( char * req , char ** out , int * success ){
  assert( out != NULL );
  *success = 1;
  (*out) = req;
  char * cur = req ;
  char * ret = NULL;

  if( cur[0] != '/' && isalpha( cur[0] ) == 0 ){ goto error; };
  if( cur[0] == '/' && cur[1] == '\r'){ 
    ret = malloc( 2 );
    strcpy( ret , "/" );
    *out = cur+1;
    return ret;
  }

  if( cur[0] == '/' && cur[1] != ' ' ){ cur ++; };

  //find first ' '
  char * cr = cur;
  int simple = 0;
  while( *cr != ' ' ){
    if( *cr == 0 ) goto error;
    if( *cr == '\r' ){ //for simple requests
      simple = 1;
      break;
    }
    cr ++;

  }

  ret = malloc( cr - cur + 1 );
  strncpy( ret , cur , cr - cur );
  ret[cr-cur] = 0;
  if( 0 == simple  ){
    cur = cr+1;
  } else {
    cur = cr;
  }
  *(out) = cur;

  return ret;

error:
  if ( ret != NULL ) { free( ret ) ; };
  *success=0;
  return NULL;
}

char *** processHeader_parse_options( char * req , char ** out , int * success ){
  //TODO enable folding
  *success = 1 ;
  int reqlen = strlen( req );
  int capacity3 = 1024;
  int size3 = 0;
  char *** ret = malloc( capacity3 * sizeof( char ** ) ) ;
  char * cur = req;
  *(out) = cur;
  char * end;

  char * str = cur ;
  while( str != NULL ){
    //get an option
    end = strchr( cur , '\n' );
    if ( end == NULL ) goto error;
    if ( (end - req + 4) > reqlen ){
      str = NULL;
    } else {
      str = end + 1;
    }

    if( size3 == capacity3 ){
      capacity3 *= 1.3;
      ret = realloc( ret , capacity3 );
    }

    {
      char * colon = strchr( cur , ':' );
      if( colon != NULL ) {
        char * first = malloc( (int)( colon - cur + 1 )  );
        memmove( first, cur , (size_t) ( colon - cur ) );
        first[colon - cur ]=0;

        char * snd = malloc ( ( end-colon - 2 ) * sizeof( char ) );
        memmove( snd , colon + 2 ,  end - colon - 3  );
        snd[ end - colon - 3 ] = 0;



        char ** slot = malloc ( 2 * sizeof( char* ) );
        slot[0] = first;
        slot[1] = snd;
        ret[size3] = slot;
        size3 ++ ;
        cur = str;
        *out = end-1;
      }
    }
  }
  char ** slot = malloc ( 2 * sizeof( char * ) );
  slot[0] = NULL;
  slot[1] = NULL;
  ret = realloc( ret , (size3 + 2)*sizeof( char ** ) );
  ret[ size3 ] = slot;
  


  return ret;
  
error:
  if ( ret != NULL ) { free( ret ) ; };
  *success=0;
  return NULL;
}

int * processHeader_get_http_version( char * req , char ** out , int * success ){
  assert( out != NULL );
  *success = 1;
  (*out) = req;
  char * cur = req ;

  int * ret = malloc( 2 * sizeof( int ) );
  if( ret == NULL ){ goto error; }

  char * http = "HTTP/";
  int hashttp = strncasecmp( req , http , strlen(http) );
  if( 0 == hashttp ){
    cur += strlen(http);
  } else if ( cur[0] == '\r' && cur[1] == '\n' ) {
    //it is an http/0.9 message
    ret[0] = 0;
    ret[1] = 9;
    *out = cur + 2;
    *success = 1 ;
    return ret;
  }

  char * cr = strchr( cur, '\r');
  if( cr==NULL || *(cr+1) != '\n' ){ goto error; }
  int major = strtol( cur , out , 10 );
  if( cur == *out || **out != '.' ) goto error;
  cur = *out;
  cur ++;
  int minor = strtol( cur, out , 10 );

  if( *out == cur ){ goto error ; };

  *out = cr+2;

  ret[0] = major;
  ret[1] = minor;
  return ret;

error:
  if ( ret != NULL ) { free( ret ) ; };
  *success=0;
  return NULL;
}

Request_header processHeader( char * req , int * success ){
  Request_header ret={0};
  ret.raw = req;
  *success = 1;
  assert( req != NULL );
  char * _out;
  char ** out = & _out ;
  (*out) = req ;
  char * cur = req;


  ret.meathod = processRequestType( cur , out , success );
  if( ! success ){ LOG( "success==0" ) ; goto error ; }
  cur = *out;

  ret.uri = processHeader_get_uri( cur , out , success );
  if( ! success ){ LOG( "success==0" ) ; goto error ; }
  cur = *out;

  {
    int * ver = processHeader_get_http_version( cur , out , success );
    cur = *out;
    if( *success ){
      ret.majorVersion = ver[0];
      ret.minorVersion = ver[1];
      free( ver );
    } else {
      LOG("success==0") ; goto error; 
    }
  }

  ret.headers = processHeader_parse_options( cur , out , success );
  if( ret.headers == NULL ) goto error;


  return ret;
error: *success=0;
       ret.fail = 1;
       return ret;
}


char * getHeader( int sock ){
  char * goal = "\r\n\r\n";

  int capacity = 1024;
  int size = 0;
  char * ret = malloc( capacity );
  if ( ret == NULL ){
    perror("realloc returned NULL");
    exit(2);
  }

  read( sock , ret , 4 );
  size+=4;
  ret[4] = 0 ;
  while( strcmp( goal , ret + size-strlen(goal) - 0 ) != 0  ){
    if( capacity < size + 1 ){
      ret = realloc( ret , capacity * 1.3 );
      if( ret == NULL ){
        perror("realloc returned NULL");
        exit(2);
      }
      capacity *= 1.3;
    } else {
      read( sock , ret + size , 1 );
      ret[size+1]=0;
      size++;
    }
  }
  return ret;
}

char * output = "HTTP/1.0 200 OK\r\nDate: Fri, 03 Oct 2014 15:23:19 GMT\r\nServer: Mine\r\nContent-Length: "; //missing the double new line


size_t strlenlist( char ** l ){
  size_t ret=0;
  while( *l != NULL ){
    ret += strlen( *l );
    l ++ ;
  }
  return ret;
}


char * concat_string_list( char ** lst ){
  int len = strlenlist( lst ) ;
  char * ret = calloc( len+1 , 1 );
  char * cur = ret;
  while( *lst != NULL ){
    strcpy( cur , *lst );
    cur += strlen( *lst );
    lst ++;
  }
  return ret;
}
char * getServerName( ){
  char * name = "Zack_server";
  char * ret = calloc( strlen( name ) + 1 , 1 );
  strcpy( ret , name );
  return ret;
}

char * getTime(){
  const int sz = 1024;
  char * ret = calloc( sz  , 1 );

  time_t rawTime;
  time( &rawTime );
  struct tm  *res ;
  res = gmtime( &rawTime );
  //Sun, 06 Nov 1994 08:49:37 GMT
  char* format = "%a, %d %b %Y %H:%M:%S GMT";
  strftime( ret , sz- 1 , format , res );

  return ret;
}

char * genPreHeader( int status ){
  char * crlf = "\r\n";
  char * ret;

  char * lst[11] = {0} ;
  switch( status ){
    case status_OK : 
      lst[0] = status_string[ status_OK ];
      break;
    case status_Not_Found:
      lst[0] = status_string[ status_Not_Found ];
      break;
    case status_Internal:
      lst[0] = status_string[ status_Internal ] ;
      break;
    case status_Bad_Request:
      lst[0] = status_string[ status_Bad_Request ] ;
      break;

    default:
      return NULL;
  }

  lst[1] = crlf;
  lst[2] = "Date: ";
  lst[3] = getTime();
  lst[4] = crlf;
  lst[5] = "Server: ";
  lst[6] = getServerName();
  lst[7] = crlf;
  lst[8] = "Content-Length: ";
  lst[9] = NULL;

  ret = concat_string_list( lst );
  free( lst[3] );
  free( lst[6] );

  return ret;
}

MeathodReturn getGet( Request_header h ){
  assert( h.meathod == Meathod_Get );
  int retcap = 4;
  int retsize = 1;
  int renamed = 0;
  assert( retsize <= retcap );
  MeathodReturn _ret = {0};
  _ret.simple = calloc ( retcap , sizeof( char * ) );
  char * uri = h.uri;

  if( uri[0] == '/' ) uri ++;
  if( uri[0] == '/' ) {
    free( _ret.simple );
    return internalError(h, status_Bad_Request);
  } else if ( uri[0] == '\0' ){
    uri = strMemCopy( "index.html" );
    renamed = 1 ;
  }

  //try to open the resource
  char * string ;
  stripDotDotSlash( uri );
  FILE* file = fopen( uri, "rb" );
  long fsize = -1;
  if( file == NULL )  { //opening failed 
    //return 404
    _ret.simple[0] = genPreHeader( status_Not_Found );

    int len = strlen(h.raw);
    string = malloc( len+1 );
    strcpy( string , h.raw );
    string[len]=0;
    _ret.simple[2] = string;
  } else { // success, give the file

    _ret.simple[0] = genPreHeader( status_OK );
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    string = malloc(fsize + 1);
    if( string == NULL ) return internalError(h , Malloc_Error );
    fread(string, fsize, 1, file);
    fclose(file);
    string[fsize] = 0;
    _ret.simple[2] = string;

    int payloadLen = fsize;
    int headerLen = strlen( _ret.simple[0] );

    char * lenst = calloc( 32 , 1 );
    sprintf(lenst , "%i\r\n\r\n", payloadLen );
    _ret.simple[1] = lenst;

    _ret.rawLen = payloadLen + headerLen + strlen( lenst ) ;
    //_ret.raw = concat_string_list( _ret.simple );
    _ret.raw = malloc( _ret.rawLen + 1 ); //extra byte required for null on simple requests
    char * cur = _ret.raw;

    if( h.majorVersion != 0 && h.minorVersion!=9 ){
      memcpy( cur , _ret.simple[0] , headerLen );
      cur += headerLen;

      memcpy( cur , _ret.simple[1] , strlen( lenst ) );
      cur += strlen( lenst );
    } else {
      _ret.rawLen = payloadLen;
    }

    memcpy( cur , _ret.simple[2] , payloadLen );
    cur += payloadLen;

    cur[0] = 0 ;

    if( renamed ){
      free( uri );
    }

  }


  //get size of charpp
  if( _ret.simple[1] == NULL ){
    char *  sizeStr = malloc( 32 );
    if( sizeStr == NULL ) return internalError( h , Malloc_Error ); 
    if( fsize > 0 ){
      sprintf( sizeStr, "%i\r\n\r\n" , (int) fsize );
    } else {
      sprintf( sizeStr, "%i\r\n\r\n" , (int) strlen( _ret.simple[2] ) );
    }
    _ret.simple[1] =  sizeStr;
  }

  return _ret;
}
MeathodReturn getHead( Request_header h ){
  assert( h.meathod == Meathod_Head );

  h.meathod = Meathod_Get;
  MeathodReturn _ret = getGet( h );
  h.meathod = Meathod_Head;

  char * sizeStr = malloc ( 32 );
  if( sizeStr == NULL ) return internalError( h , Malloc_Error );
  sprintf( sizeStr, "0\r\n\r\n" );

  free( _ret.simple[2] );
  _ret.simple[2] = NULL;

  return _ret;
}


char * get_header_field_value( Request_header h , const char * toFind ){
  char * ret = NULL;

  char *** cur = h.headers;
  while(  *cur != NULL ){
    if( 0 == strncasecmp( (*cur)[0] , toFind , strlen( (*cur)[0])  ) ){
      ret = strMemCopy ( (*cur)[1] );
      return ret;
    }
    cur ++;
  }

  return ret;
}

int atoid( const char * in , int def ){
  int ret;
  char *end;

  ret = strtol( in , &end , 10 );
  if( in == end ){
    ret = def;
  }

  return ret;

}

//un url encode
char * unencode( char * s ){
  char * ret = strMemCopy( s );

  char * cur = ret;
  while( *cur != 0 ){
    switch( *cur ){
      case '+':
        *cur = ' ';
        cur++;
        break;
      case '%':
        {
          char *out;
          char temp = cur[3];
          cur[3] = 0;
          int val = strtol( cur+1, &out , 16 );
          cur[3] = temp;
          if( out == NULL ){
            LOG("Error a");
            return ret;
          }
          if( 3 != out - cur ){
            LOG("Error b");
          }
          *cur = val;
          for( char* vv = cur+3 ; vv[ -1 ] != 0 ; vv++){
            vv[-2] = *vv;
          }
          cur = out-2;
        }
        break;
      default:
        cur++;
    } // end switch
  } // end while
  return ret;
}

List * parse_post_args ( char * b ){

  List * ret = NULL ;
  {
    char *cur=b;
    int again = 1;
    do{
      char * old = cur;
      cur = strstr( cur , "&" );

      if( cur == NULL ){
        again = 0;
        cur = strlen( old ) + old ;
      }

      char * pair = nstrMemCopy( old , 1+cur - old );
      pair[cur-old] = 0 ;

      //get first half
      char * firstEnd = strstr( pair , "=" );
      if( firstEnd == NULL ){ LOG("firstEnd was null"); }
      char * first = nstrMemCopy( pair , firstEnd - pair + 1);
      first[ firstEnd - pair ]  = 0;
      char * sndStart = firstEnd + 1 ;
      char * snd = strMemCopy ( sndStart );

      char * uf = unencode( first );
      char * us = unencode( snd );
      char ** out = calloc( 3 , sizeof ( char * ) );
      out[0] = uf;
      out[1] = us;
      out[2] = NULL;

      free( first );
      free( snd );
      free( pair );

      ret = add_front_List( out , List_type_char_pp , ret );

      cur ++ ;
    }while( cur != NULL && again == 1 );
  }

  return ret;
}

MeathodReturn makeFile ( Request_header h, const int sock ){
  //get arguments
  char * cl_string = get_header_field_value( h , "Content-Length" );

  int cl= -1;
  if( cl_string != NULL ){
    cl = atoid( cl_string, -1  );
    free ( cl_string );

  } else {
    return internalError( h , status_Bad_Request );
  }


  char * buff;
  buff = calloc( cl + 1 , sizeof( char )  );


  if ( 0 > read( sock , buff , cl ) ){
    LOG("Error reading");
    return internalError( h, status_Internal );
  }
  buff[cl]=0;
  List *parsed = parse_post_args( buff );
  free(buff);

  char *fname;
  char *contents;
  {
    List *cpy = shallow_copy_List( parsed );
    List* cur = cpy;

    while( cur != NULL ){
      char ** pair = cur->data;
      if( 0 == strcasecmp( pair[0] , "fname" ) ){
        fname = strMemCopy( pair[1] );
      } else if ( 0 == strcasecmp( pair[0] , "contents" )){
        contents = strMemCopy( pair[1] );
      }
      cur = cur->next;
    }
    free_copy_List( cpy );
  }
  {
    List * cur = parsed;
    while( cur != NULL ){
      char ** pair = cur->data;
      if(NULL !=  pair [0] ){
        free( pair[0] );
      }
      if(NULL !=  pair[1] ){
        free( pair[1] );
      }
      cur = cur->next;
    }
  }
  free_List( parsed );

  //write the file
  stripDotDotSlash( fname );
  FILE* f = fopen( fname , "wb" );
  fprintf(f , "%s",contents);
  fflush( f );
  fclose( f );

  free( fname );
  free( contents );
  fname=NULL;
  contents=NULL;
  
  MeathodReturn _ret = {0};
  _ret.simple = calloc( 4 , sizeof( char * ) );

  _ret.simple[0] = genPreHeader( status_OK );

  char * message = "<html><body>File creation finished<br></body></html>";
  _ret.simple[2] = strMemCopy( message ) ;

  return _ret;
}

MeathodReturn getPost( Request_header h , const int sock){
  assert( h.meathod == Meathod_Post );

  MeathodReturn out  = {0};
  if ( 0 == strncasecmp( h.uri , "createfile.html", strlen( "createfile.html" )  ) ) {
    out = makeFile( h, sock );
    return out;
  } 

  return internalError( h , status_Not_Found );
}

MeathodReturn internalError( Request_header h , int reason ){
  assert( reason > -1 );
  MeathodReturn _ret = {0};
  _ret.simple = calloc( 4 , sizeof( char * ) );

  assert( h.uri || 1 );
  switch ( reason ){
    case Client_Request_Error:
      _ret.simple[0] = genPreHeader( status_Bad_Request );
      _ret.simple[2] = genPreHeader( status_Bad_Request );
      break;

    case Not_Found_Error:
      _ret.simple[0] = genPreHeader( status_Not_Found );
      _ret.simple[2] = genPreHeader( status_Not_Found );
      break;
    case Malloc_Error:
      LOG("malloc error");
      break;

    default:
      LOG("bad call to function");
      break;
  }
  return _ret;
}

MeathodReturn getOutput( const Request_header h , const int sock ){
  MeathodReturn data= {0};
  char ** internal ;
  if( h.fail == 0 ){
    switch( h.meathod ){
      case Meathod_Get :
        data =  getGet(h);
        break;
      case Meathod_Head:
        data =  getHead(h);
        break;
      case Meathod_Post:
        data =  getPost(h, sock);
        break;
      default:
        data =  internalError(h, Client_Request_Error);
    }
  } else {
    data = internalError( h , Client_Request_Error );
  }

  internal = data.simple;

  assert( internal != NULL || data.rawLen > 0 );
  size_t len = strlenlist( internal+2 ) ;

  if( internal[1] == NULL ){
    char *buff = calloc(1024, 1 );
    sprintf( buff, "%i\r\n\r\n" , (int)len );
    internal[1] = buff;
  }

  if ( data.rawLen <= 0 ){
    char * str = concat_string_list( internal );
    {
      char ** cur  = internal;
      while( *cur  != NULL ){
        free( *cur );
        cur++;
      }
      //free( internal );
    }
    data.simple = NULL;
    data.rawLen  = strlen ( str );
    data.raw = str;
  }

     
  return data;
}

void * serve_client( void * argp ){
  assert( argp != NULL );
  threadArg *arg = argp;
  int sock = arg -> socket;

  char * header_string = getHeader( sock );
  if( header_string == NULL ){
    return NULL;
  }

  int b;
  Request_header rh = processHeader( header_string , &b );
  MeathodReturn mret = getOutput( rh, sock );

  if( b == 0 ){
    elog("header parsing failed"); 
  }

  write( sock , mret.raw  , mret.rawLen );
  free_MeathodReturn( mret );

  freeRequest_header( rh );
  
  close ( sock );
  free( arg -> threadStruct );
  free( argp );

  return NULL;
}
