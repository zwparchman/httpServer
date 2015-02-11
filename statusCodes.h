
#ifndef  statuscodes_INC
#define  statusCodes_INC

enum {
  status_OK ,
  status_Created,
  status_Accepted,
  status_No_Content,
  status_Multiple ,
  status_Moved_Permanent ,
  status_Moved_Temporary ,
  status_Not_Modified ,
  status_Bad_Request ,
  status_Unauthorized,
  status_Forbidden,
  status_Not_Found ,
  status_Internal ,
  status_Not_Implemented ,
  status_Bad_Gateway ,
  status_Service_Unavailable
};


extern char *  status_string [];



enum { Meathod_Get = 1 , Meathod_Head = 2 , Meathod_Post = 3 };
enum { Malloc_Error = 1 , Client_Request_Error = 2, Not_Found_Error = 3 };

#endif   /* ----- #ifndef statusCodes_INC  ----- */
