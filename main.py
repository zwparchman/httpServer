#!/usr/bin/python
import sys
import string
import cStringIO
import cgi
import select
import Cookie

import BaseHTTPServer
import SocketServer
import os
import StringIO
import urllib
import cgi
import datetime

PORT = 12000

upload_file_html = """
<html><body><form enctype="multipart/form-data"
              method="post"
              action="upload.html">
              <!-- I know "tables for layout", but it's easy -->
              <table>
                  <tr>
                      <td>Save as (not required)</td>
                      <td><input type="text" name="name"></td>
                  </tr>
                  <tr>
                      <td>File to upload</td>
                      <td><input type="file" name="contents"></td>
                  </tr>
              </table>
  <input type="submit" value="submit"><br>
</form></body></html>
"""

def cacheDumpIndex( di ):
    '''cache dumpIndex using 2 state LRU algo'''

    #dict of type {string:(return_value,LRU_value),...}
    args = {}

    def new( self, s ):
        if s in args:
            val = args[s]
            args[s] = ( val[0] , 1 )
            return val[0]
        else:
            ret = di( self, s )
            args[s]= ( ret, 1 )
            return ret

    return new


home = os.getenv("HOME")
if home is None : home = "/tmp"


class myServer( BaseHTTPServer.BaseHTTPRequestHandler ):
    def log( self , s=None ):
        all = ""
        today = datetime.date.today().isoformat()
        today = "DateOfRequest:"+str( today )
        path = " requestedPath:"+self.path
        headers = " Headers:"+str( self.headers.dict )
        all = today + path + headers

        if s is not None:
            all = all+s

        all = all.replace('\n','')
        all = all.replace('\r','')

        def doWrite(f):
            f.write( all+"\n" )
            f.flush()
        try:
            with file( os.path.join(home,"serverlog") ,"a" ) as f:
                doWrite(f)
        except IOError:
            try:
                doWrite(sys.stdout)
            except IOError:
                pass

    def evil( self , s ):
        if '..' in s : return True
        if '//' in s : return True
        if '~' in s : return True
        if '\\'in s : return True
        return False


    def do_GET( self ):
        self.log()
        if self.evil( self.path ):
            self._404()
            return
        self.path = urllib.unquote(self.path).decode('utf8')

        while len(self.path) > 0 and self.path[0] == '/':
            self.path = self.path[1:]

        while len( self.path ) > 0 and self.path[-1] == '/':
            self.path = self.path[:-1]

        self.path = "./"+self.path

        indexes = ["./index.html","/index.html","index.html"]
        upload = ["./upload.html","/upload.html","upload.html"]
        if self.path in indexes :
            self.write( self.dumpIndex(".") )
        elif self.path in upload :
            self.write( self.genPacket("200 OK", upload_file_html))

        elif os.path.isdir(self.path):
            self.write( self.dumpIndex(self.path) )
        elif os.path.isfile(self.path):
            with file ( self.path , "rb" ) as f:
                payload = f.read()
                self.wfile.write( self.genPacket("200 OK", payload ) )
        else:
            self.write( self.dumpIndex(".") )

    def do_POST( self ):
        """Begin serving a POST request. The request data is readable
        on a file-like object called self.rfile"""
        self.log()
        ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
        length = int(self.headers.getheader('content-length'))
        fs=None


        if ctype == 'multipart/form-data':
            fs = cgi.FieldStorage( fp = self.rfile,
                headers = self.headers,
                environ={ 'REQUEST_METHOD':'POST' } #let cgi know this is a POST meathod
                )
        else:
            fs = cgi.FieldStorage()

        # some browsers send 2 more bytes...
        [ready_to_read,x,y] = select.select([self.connection],[],[],0)
        if ready_to_read:
            self.rfile.read(2)

        self.log( "PostBody: "+str(fs) )

        oname=None
        fileitem = None

        file_field = fs['contents']
        filedata = file_field.file

        if( filedata is None ) :
            self.wfile.write( self.genPacket(
                "300 File Required",
                "File Required in form"))
            return

        oname = file_field.filename

        override_name = fs.getfirst('name')
        if override_name is not None and override_name != "":
            oname = override_name

        if oname == "":
            self.wfile.write( self.genPacket(
                "300 Name Required",
                "Name Required in form"))
            return

        extra = ""
        fails = 0

        while( os.path.isdir( oname+extra ) or os.path.isfile( oname+extra )):
            fails += 1
            extra = str( fails )

        oname = oname+extra

        if self.evil( oname ):
            self.write( self.genPacket("300 I didn't like that file name","I didn't like that file name" ) )

        try:
            with file( oname, 'wb' ) as f:
                f.write( filedata.read() )
        except IOError:
            self.write( self.genPacket("500 server IO error.", "There was an error writing the file"))
            return

        self.write( self.genPacket( "200 OK", "<html><body>Write Successfull\r\n<br>Filename is: "+oname+"</body></html>" ))
        return

    def do_HEAD( self ):
        self.log()
        if self.evil():
            self.wfile( self.genPacket("404 Not Found","") )
        else:
            self.wfile( self.genPacket("200 OK", "" ))

    def genPacket( self, stat , payload ):
        return "HTTP/1.0 "+stat+"\r\n"+\
                "host: Zacks Server\r\n" +\
                "ContentLength: " + str( len(payload))+"\r\n" +\
                "\r\n"+ \
                payload

    def _404( self ):
        payload = "File not found"
        headerstr = self.genPacket( "404 Not Found" , payload )
        self.wfile.write( headerstr + payload )

    def write(self, s ):
        self.wfile.write( s )

    #@cacheDumpIndex
    def dumpIndex( self, path ):
        dname = []
        fname = []
        for dirpath, dirnames, fnames in os.walk(path):
            fname = fnames
            dname = dirnames
            break

        payload = ""
        if path != "./":
            less = path
            less = os.path.split( less )[0]
            less = os.path.normpath( less )
            payload += wrapInLink( less, "Up one directory" )+"<br>"

        for f in fname:
            payload+= wrapInLink( path+"/"+f ) + "<br>"

        payload += "<h2>Directories</h2>"
        for d in dname:
            payload += "DIR "+wrapInLink( path+"/"+d )+"<br>"

        payload = "<html><body><h2>Listing</h2><br>"+payload+"</body></html>"

        return self.genPacket( "200 OK", payload )


def wrapInLink( path , writeAs=None ):
    opath = path
    while len( opath ) >  0 and opath[0] == '.':
        opath = opath[1:]

    while len( opath ) >  0 and opath[0] == '/':
        opath = opath[1:]

    if len( opath ) == 0 :
        opath = "/index.html"
    else:
        opath = "/"+opath

    if writeAs == None:
        writeAs = os.path.split(opath)[1]
    return "<a href="+opath+"> "+writeAs+"</a>"

class sserver( SocketServer.ThreadingMixIn, SocketServer.TCPServer ):
#class sserver( SocketServer.TCPServer ):
    allow_reuse_address=True
    timeout = 1
    request_queue_size=16

Handler = myServer

httpd =  sserver (("", PORT), Handler)


try:
    print "serving at port", PORT
except IOError:
    pass

try:
    httpd.serve_forever()
except Exception as e:
    print e
    del httpd

