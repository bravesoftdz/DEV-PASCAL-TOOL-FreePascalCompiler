{
    $Id: stringsi.inc,v 1.1 2000/07/13 06:30:49 michael Exp $
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2000 by the Free Pascal development team

    Processor independent part for strings and sysutils units

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

    function strcat(dest,source : pchar) : pchar;

      begin
        strcopy(strend(dest),source);
        strcat:=dest;
      end;

    function strlcat(dest,source : pchar;l : longint) : pchar;

      var
         destend : pchar;

      begin
         destend:=strend(dest);
         dec(l,destend-dest);
         if l>0 then
          strlcopy(destend,source,l);
         strlcat:=dest;
      end;

    function strmove(dest,source : pchar;l : longint) : pchar;

      begin
         move(source^,dest^,l);
         strmove:=dest;
      end;


    function strpos(str1,str2 : pchar) : pchar;

      var
         p : pchar;
         lstr2 : longint;

      begin
         strpos:=nil;
         p:=strscan(str1,str2^);
         if p=nil then
           exit;
         lstr2:=strlen(str2);
         while p<>nil do
           begin
              if strlcomp(p,str2,lstr2)=0 then
                begin
                   strpos:=p;
                   exit;
                end;
              inc(longint(p));
              p:=strscan(p,str2^);
           end;
      end;

{
  $Log: stringsi.inc,v $
  Revision 1.1  2000/07/13 06:30:49  michael
  + Initial import

  Revision 1.7  2000/02/09 16:59:31  peter
    * truncated log

  Revision 1.6  2000/01/07 16:41:36  daniel
    * copyright 2000

  Revision 1.5  1999/12/10 15:02:12  peter
    * strnew is ofcourse also different between sysutils and strings, just
      like stralloc/strdispose.

  Revision 1.4  1999/09/13 11:42:42  peter
    * fixed strlcat

  Revision 1.3  1999/09/01 09:25:10  peter
    * fixed return of strcat,strlcat

}
