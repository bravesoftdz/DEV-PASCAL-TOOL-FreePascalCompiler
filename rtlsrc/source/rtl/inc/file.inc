{
    $Id: file.inc,v 1.1.2.4 2002/04/10 08:03:45 pierre Exp $
    This file is part of the Free Pascal Run time library.
    Copyright (c) 1999-2000 by the Free Pascal development team

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WithOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{****************************************************************************
                    subroutines For UnTyped File handling
****************************************************************************}

type
  UnTypedFile=File;

Procedure Assign(var f:File;const Name:string);
{
  Assign Name to file f so it can be used with the file routines
}
Begin
  FillChar(f,SizeOf(FileRec),0);
  FileRec(f).Handle:=UnusedHandle;
  FileRec(f).mode:=fmClosed;
  Move(Name[1],FileRec(f).Name,Length(Name));
End;


Procedure assign(var f:File;p:pchar);
{
  Assign Name to file f so it can be used with the file routines
}
begin
  Assign(f,StrPas(p));
end;


Procedure assign(var f:File;c:char);
{
  Assign Name to file f so it can be used with the file routines
}
begin
  Assign(f,string(c));
end;


Procedure Rewrite(var f:File;l:Longint);[IOCheck];
{
  Create file f with recordsize of l
}
Begin
  If InOutRes <> 0 then
   exit;
  Case FileRec(f).mode Of
   fmInOut,fmInput,fmOutput : Close(f);
   fmClosed : ;
  else
   Begin
     InOutRes:=102;
     exit;
   End;
  End;
  If l=0 Then
   InOutRes:=2
  else
   Begin
     { Reopen with filemode 2, to be Tp compatible (PFV) }
     Do_Open(f,PChar(@FileRec(f).Name),$1002);
     FileRec(f).RecSize:=l;
   End;
End;


Procedure Reset(var f:File;l:Longint);[IOCheck];
{
  Open file f with recordsize of l and filemode
}
Begin
  If InOutRes <> 0 then
   Exit;
  Case FileRec(f).mode Of
   fmInOut,fmInput,fmOutput : Close(f);
   fmClosed : ;
  else
   Begin
     InOutRes:=102;
     exit;
   End;
  End;
  If l=0 Then
   InOutRes:=2
  else
   Begin
     if FileRec(F).Name[0]=#0 then
       Do_open(f,PChar(@FileRec(f).Name),0)
     else
       Do_Open(f,PChar(@FileRec(f).Name),Filemode);
     FileRec(f).RecSize:=l;
   End;
End;


Procedure Rewrite(Var f:File);[IOCheck];
{
  Create file with (default) 128 byte records
}
Begin
  If InOutRes <> 0 then
   exit;
  Rewrite(f,128);
End;


Procedure Reset(Var f:File);[IOCheck];
{
  Open file with (default) 128 byte records
}
Begin
  If InOutRes <> 0 then
   exit;
  Reset(f,128);
End;


Procedure BlockWrite(Var f:File;Const Buf;Count:Longint;var Result:Longint);[IOCheck];
{
  Write Count records from Buf to file f, return written records in result
}
Begin
  Result:=0;
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmOutput :
      Result:=Do_Write(FileRec(f).Handle,Longint(@Buf),Count*FileRec(f).RecSize)
        div FileRec(f).RecSize;
    fmInPut: inOutRes := 105;
    else InOutRes:=103;
  end;
End;


Procedure BlockWrite(Var f:File;Const Buf;Count:Word;var Result:Word);[IOCheck];
{
  Write Count records from Buf to file f, return written records in Result
}
var
  l : longint;
Begin
  BlockWrite(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockWrite(Var f:File;Const Buf;Count:Cardinal;var Result:Cardinal);[IOCheck];
{
  Write Count records from Buf to file f, return written records in Result
}
var
  l : longint;
Begin
  BlockWrite(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockWrite(Var f:File;Const Buf;Count:Word;var Result:Integer);[IOCheck];
{
  Write Count records from Buf to file f, return written records in Result
}
var
  l : longint;
Begin
  BlockWrite(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockWrite(Var f:File;Const Buf;Count:Longint);[IOCheck];
{
  Write Count records from Buf to file f, if none a Read and Count>0 then
  InOutRes is set
}
var
  Result : Longint;
Begin
  BlockWrite(f,Buf,Count,Result);
  If (Result<Count) and (Count>0) Then
   InOutRes:=101;
End;


Procedure BlockRead(var f:File;var Buf;Count:Longint;var Result:Longint);[IOCheck];
{
  Read Count records from file f ro Buf, return number of read records in
  Result
}
Begin
  Result:=0;
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmInput :
      Result:=Do_Read(FileRec(f).Handle,Longint(@Buf),count*FileRec(f).RecSize)
        div FileRec(f).RecSize;
    fmOutput: inOutRes := 104;
    else InOutRes:=103;
  end;
End;


Procedure BlockRead(var f:File;var Buf;count:Word;var Result:Word);[IOCheck];
{
  Read Count records from file f to Buf, return number of read records in
  Result
}
var
  l : longint;
Begin
  BlockRead(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockRead(var f:File;var Buf;count:Cardinal;var Result:Cardinal);[IOCheck];
{
  Read Count records from file f to Buf, return number of read records in
  Result
}
var
  l : longint;
Begin
  BlockRead(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockRead(var f:File;var Buf;count:Word;var Result:Integer);[IOCheck];
{
  Read Count records from file f to Buf, return number of read records in
  Result
}
var
  l : longint;
Begin
  BlockRead(f,Buf,Count,l);
  Result:=l;
End;


Procedure BlockRead(Var f:File;Var Buf;Count:Longint);[IOCheck];
{
  Read Count records from file f to Buf, if none are read and Count>0 then
  InOutRes is set
}
var
  Result : Longint;
Begin
  BlockRead(f,Buf,Count,Result);
  If (Result<Count) and (Count>0) Then
   InOutRes:=100;
End;


Function FilePos(var f:File):Longint;[IOCheck];
{
  Return current Position In file f in records
}
Begin
  FilePos:=0;
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmInput,fmOutput :
      FilePos:=Do_FilePos(FileRec(f).Handle) div FileRec(f).RecSize;
    else
      InOutRes:=103;
  end;
End;


Function FileSize(var f:File):Longint;[IOCheck];
{
  Return the size of file f in records
}
Begin
  FileSize:=0;
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmInput,fmOutput :
      begin
        if (FileRec(f).RecSize>0) then
          FileSize:=Do_FileSize(FileRec(f).Handle) div FileRec(f).RecSize;
      end;
    else InOutRes:=103;
  end;
End;


Function Eof(var f:File):Boolean;[IOCheck];
{
  Return True if we're at the end of the file f, else False is returned
}
Begin
  Eof:=false;
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    {Can't use do_ routines because we need record support}
    fmInOut,fmInput,fmOutput : Eof:=(FileSize(f)<=FilePos(f));
    else InOutRes:=103;
  end;
End;


Procedure Seek(var f:File;Pos:Longint);[IOCheck];
{
  Goto record Pos in file f
}
Begin
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmInput,fmOutput :
      Do_Seek(FileRec(f).Handle,Pos*FileRec(f).RecSize);
    else InOutRes:=103;
  end;
End;


Procedure Truncate(Var f:File);[IOCheck];
{
  Truncate/Cut file f at the current record Position
}
Begin
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmOutput :
      Do_Truncate(FileRec(f).Handle,FilePos(f)*FileRec(f).RecSize);
    else InOutRes:=103;
  end;
End;


Procedure Close(var f:File);[IOCheck];
{
  Close file f
}
Begin
  If InOutRes <> 0 then
   exit;
  case FileRec(f).Mode of
    fmInOut,fmInput,fmOutput :
      begin
        Do_Close(FileRec(f).Handle);
        FileRec(f).mode:=fmClosed;
      end
    else InOutRes:=103;
  end;
End;


Procedure Erase(var f : File);[IOCheck];
Begin
  If InOutRes <> 0 then
   exit;
  If FileRec(f).mode=fmClosed Then
   Do_Erase(PChar(@FileRec(f).Name));
End;


Procedure Rename(var f : File;p:pchar);[IOCheck];
Begin
  If InOutRes <> 0 then
   exit;
  If FileRec(f).mode=fmClosed Then
   Begin
     Do_Rename(PChar(@FileRec(f).Name),p);
     { check error code of do_rename }
     If InOutRes = 0 then
        Move(p^,FileRec(f).Name,StrLen(p)+1);
   End;
End;


Procedure Rename(var f : File;const s : string);[IOCheck];
var
  p : array[0..255] Of Char;
Begin
  If InOutRes <> 0 then
   exit;
  Move(s[1],p,Length(s));
  p[Length(s)]:=#0;
  Rename(f,Pchar(@p));
End;


Procedure Rename(var f : File;c : char);[IOCheck];
var
  p : array[0..1] Of Char;
Begin
  If InOutRes <> 0 then
   exit;
  p[0]:=c;
  p[1]:=#0;
  Rename(f,Pchar(@p));
End;

{
  $Log: file.inc,v $
  Revision 1.1.2.4  2002/04/10 08:03:45  pierre
   * fix for bug 1914

  Revision 1.1.2.3  2001/06/27 02:22:12  carl
  * bugfix of rename from Paul Yanchenko

  Revision 1.1.2.2  2001/06/04 11:43:18  peter
    * Formal const to var fixes
    * Hexstr(int64) added

  Revision 1.1.2.1  2001/02/12 12:10:37  pierre
   + cardinal versions of blockread/blockwrite

  Revision 1.1  2000/07/13 06:30:44  michael
  + Initial import

  Revision 1.20  2000/03/24 10:26:18  jonas
    * changed a lot of "if fm.mode = fmClosed then" to case statements,
      because if f is not yet initialized, the mode is invalid and can
      contain another value even though the file is closed
    + check if a file is open in writeln_end (caused crash if used on
      not opened files)

  Revision 1.19  2000/02/09 16:59:29  peter
    * truncated log

  Revision 1.18  2000/01/17 20:02:30  peter
    * open with mode 2 in rewrite

  Revision 1.17  2000/01/16 22:25:38  peter
    * check handle for file closing

  Revision 1.16  2000/01/07 16:41:33  daniel
    * copyright 2000

  Revision 1.15  2000/01/07 16:32:24  daniel
    * copyright 2000 added

  Revision 1.14  1999/10/28 09:52:50  peter
    * use filemode for rewrite instead of mode 1

  Revision 1.13  1999/09/10 15:40:33  peter
    * fixed do_open flags to be > $100, becuase filemode can be upto 255

  Revision 1.12  1999/09/08 16:12:24  peter
    * fixed inoutres for diskfull

  Revision 1.11  1999/09/07 15:54:18  hajny
    * fixed problem with Close under OS/2

}
