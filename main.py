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

PORT = 12000

upload_file_html = """
<html><body><form enctype="multipart/form-data"
              method="post"
              action="upload.html">
  <input type="textbox" name="name"><br>
  <input type="file" name="contents"><br>
  <input type="submit" value="submit"><br>
</form></body></html>
"""
class myServer( BaseHTTPServer.BaseHTTPRequestHandler ):
    def evil( self ):
        if '..' in self.path : return True
        if '//' in self.path : return True
        if '~' in self.path : return True
        return False


    def do_GET( self ):
        if self.evil():
            self._404()
            return
        self.path = urllib.unquote(self.path).decode('utf8')

        while len(self.path) > 0 and self.path[0] == '/':
            self.path = self.path[1:]

        while len( self.path ) > 0 and self.path[-1] == '/':
            self.path = self.path[:-1]

        self.path = "./"+self.path

        print self.path
        indexes = ["./index.html","/index.html","index.html"]
        upload = ["./upload.html","/upload.html","upload.html"]
        if self.path in indexes :
            self.dumpIndex(".")
        elif self.path in upload :
            self.wfile.write( self.genPacket("200 OK", upload_file_html))

        elif os.path.isdir(self.path):
            self.dumpIndex(self.path)
        elif os.path.isfile(self.path):
            with file ( self.path , "rb" ) as f:
                payload = f.read()
                self.wfile.write( self.genPacket("200 OK", payload ) )
        else:
            self.dumpIndex(".")

    def do_POST( self ):
        """Begin serving a POST request. The request data is readable
        on a file-like object called self.rfile"""
        ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
        length = int(self.headers.getheader('content-length'))
        if ctype == 'multipart/form-data':
            body = cgi.parse_multipart(self.rfile, pdict)
        elif ctype == 'application/x-www-form-urlencoded':
            qs = self.rfile.read(length)
            body = cgi.parse_qs(qs, keep_blank_values=1)
        else:
            self.body = {}                   # Unknown content-type
        # some browsers send 2 more bytes...
        [ready_to_read,x,y] = select.select([self.connection],[],[],0)
        if ready_to_read:
            self.rfile.read(2)

        oname=None
        cont = None
        if 'name' not in body or not len(body['name'])>0:
            self.wfile.write( self.genPacket("300 Name Required", "Name Required in form"))
            return
        else:
            oname = body['name'][0]

        if 'contents' not in body :
            self.wfile.write( self.genPacket("300 File Required", "File Required in form"))
            return
        else:
            cont = body['contents'][0]

        extra = ""
        fails = 0

        while( os.path.isfile( oname+extra )):
            fails += 1
            extra = str( fails )

        oname = oname+extra

        with file( oname, 'wb' ) as f:
            f.write( cont )

        self.wfile.write( self.genPacket( "200 OK", "<html><body>Write Successfull\r\n<br>Filename is: "+oname+"</body></html>" ))
        return

    def do_HEAD( self ):
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

        self.wfile.write( self.genPacket( "200 OK", payload ) )



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
    allow_reuse_address=True

Handler = myServer

#httpd =  SocketServer.TCPServer (("", PORT), Handler)
httpd =  sserver (("", PORT), Handler)

print "serving at port", PORT

try:
    httpd.serve_forever()
except Exception as e:
    print e
    del httpd

