#!/usr/bin/python

 #   Copyright (C) 2011 Bolin Huang <bolin.huang@gmail.com>
 #   I made it just for fun -_-". And I hope it works as desired.
 #
 #    This program is free software; you can redistribute it and/or modify
 #    it under the terms of the GNU General Public License as published by
 #    the Free Software Foundation; either version 2 of the License, or
 #    (at your option) any later version.
 #
 #    This program is distributed in the hope that it will be useful,
 #    but WITHOUT ANY WARRANTY; without even the implied warranty of
 #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #    GNU General Public License for more details.
 
import sys,os
from ftplib import FTP
import cgi

class Addsuggest:
    def getsite(self):
        try:
            form=cgi.FieldStorage()           
            remote_addr=os.environ["REMOTE_ADDR"]
            file=open("suggestion","a")
            file.write(remote_addr+":\n")
            file.write("\t"+form["suggest"].value+"\n\n");
        except:
            print "<p>An error occured.</p>"
            
    def puthead(self):
        print "Content-Type: text/html; charset=GBK\r\n\r\n"
        print "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=GBK\">\r\n"
        print "<TITLE>%s</TITLE></head>\n"%"Simple FTP Search Engine"
        print """
        <H1>Simple Ftp Search Engine</H1>
        <form action="/cgi-bin/search">
        <input type="text"  name="q"/>
        <input type="submit" />
        </form>   """
        print "<hr/>"                

add=Addsuggest()
add.puthead()
add.getsite()
print "<h3>Thanks for your suggestion.</h3>\r\n"
 
    

