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

import  os
import sys
import base64
from ftplib import FTP
#chardet : http://chardet.feedparser.org/
import chardet


class Error(Exception): pass

TMP="tmp"
ADDRESS_FILE="ftp_address" 
TIME_OUT=20 #connect time limit
CONNECT_INTERVAL="2"
NEWCODE="GBK"  
MAXFAIL=3    #retry time
FAIL_FTPSITE="fail_ftp_site"  #ftp site that fail to connect

class Collect:   
    def __init__(self):
        self.fail_site=open(FAIL_FTPSITE,"w");
    
    def transform(self,str,old,new):
        try:
            unicode=str.decode(old)
            return unicode.encode(new)
        except:
            return str
    
    def recursive(self,curdir,ftp):
        print curdir
        stdout=sys.stdout
        self.file_count+=1
        dirfile_name="tmpfile"+str(self.file_count)
        try:
            sys.stdout=open(dirfile_name ,"w")
        except IOError:
            print "open write file %s error"%dirfile_name
            return "ioerror"

        try:
            ftp.retrlines("LIST")
        except:
            sys.stdout.close()
            sys.stdout=stdout
            print "debug:LIST error"
            for i in range(MAXFAIL):
                try:
                    ftp.cwd("..")
                    break
                except:
                    print "debug:LIST error"
                    continue
            return "recursion LIST error"
        sys.stdout.close()
        sys.stdout=stdout

        try:
            file_list=open(dirfile_name,"r").readlines()
            content=curdir
            for line in file_list:content+=line
            OLDCODE=chardet.detect(content)["encoding"]
        except:
            print "debug: open read file %s error"%dirfile_name
            ftp.cwd("..")
            return "open file error"

        try:            
            rewrite_file=open(dirfile_name,"w")
            rewrite_file.write(self.transform(curdir,OLDCODE,NEWCODE)+"\n")
        except:
            print "debug: rewrite file error"
            ftp.cwd("..")
            return "rewrite file error"
        
        for line in file_list:
            info=line.split(" ")
            if curdir=="/":add_slash=""
            else:add_slash="/"

            isdir=False
            next_dir=""
            # DOS format: 06-01-09  02:34PM       <DIR>          DSP     
            if info[0][0].isdigit():
                if info.count("<DIR>"):isdir=True
                count=0
                for item in info:
                    if item!="":
                        count+=1
                        if count>=4:
                            if next_dir=="":next_dir=item
                            else:next_dir=next_dir+" "+item
            #Unix format: drw-rw-r--    1 500      500           166 Jan 30 02:05 tmp
            else:
                if info[0][0]=='d':isdir=True
                count=0
                for item in info:
                    if item!="":
                        count+=1
                        if count>=9:
                            if next_dir=="":next_dir=item
                            else:next_dir=next_dir+" "+item
          
            next_dir=next_dir.split("\n")[0]
            try:                
                rewrite_file.write(next_dir.replace(" ","$")+" "\
                                   +self.transform(line,OLDCODE,NEWCODE))
            except:
                print "rewrite file  line error [%s]"%line
                ftp.cwd("..")
                return "error"
            
            next_dir.replace("$"," ")
            
            if isdir:
                if next_dir=="." or next_dir=="..":continue
                try:
                    ftp.cwd(next_dir)
                except:
                    print "debug: cwd %s error"% next_dir
                    continue
                try:
                    self.recursive(curdir+add_slash+next_dir,ftp)
                except:
                    print "debug: recursion %s error"%curdir+add_slash+next_dir
                    continue

        ftp.cwd("..")
        return "recursion success"
 
    def collect_site(self,addr):  
        self.file_count=0
        print "site:  ",addr[0]
        fail_count=0
        while fail_count<=MAXFAIL:
            try:
                try:
                    ftp=FTP(addr[0],timeout=TIME_OUT)
                except:
                    print "FTP error"
                    raise ftplib.Error                    
                login_res=ftp.login(user=addr[1],passwd=addr[2])
                if login_res.find("230")==-1:return "can not login"                
                print "debug: login success"

                dirname=""                
                if addr[1]=="anonymous" and addr[2]=="anonymous": dirname=addr[0]
                else: dirname=addr[1]+":"+addr[2]+"@"+addr[0]
                if not os.listdir(".").count(dirname):os.mkdir(dirname)
                
                os.chdir(dirname)         
                try:
                    recu_result=self.recursive("/",ftp)
                except:
                    os.chdir("..")
                    return "recursion error"
                
                os.chdir("..")
                return recu_result
            except:
                fail_count+=1                
                if fail_count>=MAXFAIL:
                    self.fail_site.write(":".join(addr)+"\n")
                    return "collect site: %s error"%addr[0]
                os.system("sleep "+CONNECT_INTERVAL)
    
    def collect(self):        
        try:
            f_addr=open(ADDRESS_FILE,"r")
        except IOError:
            return "debug: can not open %s"%ADDRESS_FILE
        list_addr=f_addr.readlines()
        os.system("rm  -rf "+TMP+"/*")
        os.chdir(TMP)
        for addr_name_pass in list_addr:
            if addr_name_pass.__len__()>4:
               error_message=self.collect_site(addr_name_pass.split(":"))
            print "debug: collect %s :"%addr_name_pass.split(":")[0],error_message
        os.chdir("..")
        return "debug: collect end"

if __name__=="__main__":
    spider=Collect()
    collect_result=spider.collect()
    print collect_result
