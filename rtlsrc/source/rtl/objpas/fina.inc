{
    *********************************************************************
    $Id: fina.inc,v 1.1.2.9 2003/01/27 19:48:35 carl Exp $
    Copyright (C) 1997, 1998 Gertjan Schouten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    *********************************************************************

    System Utilities For Free Pascal
}

function ChangeFileExt(const FileName, Extension: string): string;
var i: longint;
begin
  I := Length(FileName);
  while (I > 0) and not(FileName[I] in ['/', '.', '\', ':']) do 
    Dec(I);
  if (I = 0) or (FileName[I] <> '.') then 
    I := Length(FileName)+1;
  Result := Copy(FileName, 1, I - 1) + Extension;
end;

function ExtractFilePath(const FileName: string): string;
var i: longint;
begin
i := Length(FileName);
while (i > 0) and not (FileName[i] in ['/', '\', ':']) do Dec(i);
If I>0 then
  Result := Copy(FileName, 1, i)
else
  Result:='';
end;

function ExtractFileDir(const FileName: string): string;
var i: longint;
begin
I := Length(FileName);
while (I > 0) and not (FileName[I] in ['/', '\', ':']) do Dec(I);
if (I > 1) and (FileName[I] in ['\', '/']) and
   not (FileName[I - 1] in ['/', '\', ':']) then Dec(I);
Result := Copy(FileName, 1, I);
end;

function ExtractFileDrive(const FileName: string): string;
var i: longint;
begin
if (Length(FileName) >= 3) and (FileName[2] = ':') then
   result := Copy(FileName, 1, 2)
else if (Length(FileName) >= 2) and (FileName[1] in ['/', '\']) and
   (FileName[2] in ['/', '\']) then begin
   i := 2;
   While (i < Length(Filename)) do begin
           if Filename[i + 1] in ['/', '\'] then break;
      inc(i);
      end ;
   Result := Copy(FileName, 1, i);
   end
else Result := '';
end;

function ExtractFileName(const FileName: string): string;
var i: longint;
begin
I := Length(FileName);
while (I > 0) and not (FileName[I] in ['/', '\', ':']) do Dec(I);
Result := Copy(FileName, I + 1, 255);
end;

function ExtractFileExt(const FileName: string): string;
var i: longint;
begin
I := Length(FileName);
while (I > 0) and not (FileName[I] in ['.', '/', '\', ':']) do Dec(I);
if (I > 0) and (FileName[I] = '.') then
   Result := Copy(FileName, I, 255)
else Result := '';
end;

function ExpandFileName (Const FileName : string): String;

Var S : String;

Begin
 S:=FileName;
 DoDirSeparators(S);
{$ifdef hasunix}
  Result:=Linux.fexpand(S);
{$else}
  Result:=Dos.Fexpand(S);
{$endif}
end;

function ExpandUNCFileName (Const FileName : string): String;
begin
  Result:=ExpandFileName (FileName);
  //!! Here should follow code to replace the drive: part with UNC...
end;

Const MaxDirs = 129;

function ExtractRelativepath (Const BaseName,DestName : String): String;

Var Source, Dest : String;
    Sc,Dc,I,J    : Longint;
    SD,DD        : Array[1..MaxDirs] of PChar;

Const OneLevelBack = '..'+PathDelim;

begin
  If Upcase(ExtractFileDrive(BaseName))<>Upcase(ExtractFileDrive(DestName)) Then
    begin
    Result:=DestName;
    exit;
    end;
  Source:=ExtractFilePath(BaseName);
  Dest:=ExtractFilePath(DestName);
  SC:=GetDirs (Source,SD);
  DC:=GetDirs (Dest,DD);
  I:=1;
  While (I<DC) and (I<SC) do
    begin
    If StrIcomp(DD[i],SD[i])=0 then
      Inc(i)
    else
      Break;
    end;
  Result:='';
  For J:=I to SC-1 do Result:=Result+OneLevelBack;
  For J:=I to DC-1 do Result:=Result+DD[J]+PathDelim;
  Result:=Result+ExtractFileName(DestNAme);
end;

Procedure DoDirSeparators (Var FileName : String);

VAr I : longint;

begin
  For I:=1 to Length(FileName) do
    If FileName[I] in DirSeparators then
      FileName[i]:=PathDelim;
end;


Function SetDirSeparators (Const FileName : string) : String;

begin
  Result:=FileName;
  DoDirSeparators (Result);
end;

{
  DirName is split in a #0 separated list of directory names,
  Dirs is an array of pchars, pointing to these directory names.
  The function returns the number of directories found, or -1
  if none were found.
  DirName must contain only PathDelim as Directory separator chars.
}

Function GetDirs (Var DirName : String; Var Dirs : Array of pchar) : Longint;

Var I : Longint;

begin
  I:=1;
  Result:=-1;
  While I<=Length(DirName) do
    begin
    If DirName[i]=PathDelim then
      begin
      DirName[i]:=#0;
      Inc(Result);
      Dirs[Result]:=@DirName[I+1];
      end;
    Inc(I);
    end;
  If Result>-1 then inc(Result);
end;

function IncludeTrailingPathDelimiter(Const Path : String) : String;

Var
  l : Integer;

begin
  Result:=Path;
  l:=Length(Result);
  If (L=0) or (Result[l]<>PathDelim) then
    Result:=Result+PathDelim;
end;

function IncludeTrailingBackslash(Const Path : String) : String;

begin
  Result:=IncludeTrailingPathDelimiter(Path);
end;

function ExcludeTrailingBackslash(Const Path: string): string;

begin
  Result:=ExcludeTrailingPathDelimiter(Path);   
end;

function ExcludeTrailingPathDelimiter(Const Path: string): string;

Var
  L : Integer;

begin
  L:=Length(Path);
  If (L>0) and (Path[L]=PathDelim) then
    Dec(L);
  Result:=Copy(Path,1,L);
end;

function IsPathDelimiter(Const Path: string; Index: Integer): Boolean;

begin
  Result:=(Index>0) and (Index<=Length(Path)) and (Path[Index]=PathDelim);
end;

{
  $Log: fina.inc,v $
  Revision 1.1.2.9  2003/01/27 19:48:35  carl
    + Use HASUNIX to avoid compilation problems

  Revision 1.1.2.8  2002/11/17 20:37:38  marco
   * DONT USE IFDEF LINUX IF YOU MEAN IFDEF UNIX. ALSO IN FIXES!!!!

  Revision 1.1.2.7  2002/10/23 17:51:45  carl
    * linux unit only exists when linux is defined!

  Revision 1.1.2.6  2002/10/22 21:56:07  michael
  + Added some missing path functions

  Revision 1.1.2.5  2002/10/12 15:31:51  michael
  + Fixed changefileexit for long (>255) filenames

  Revision 1.1.2.4  2002/07/16 07:44:18  jonas
    * fixed compilation problems

  Revision 1.1.2.2  2002/05/31 11:18:21  marco
   * Rename fest for 1.0.x step one. Compiler and RTL

  Revision 1.1.2.1  2001/11/08 12:49:47  michael
  + Replaced OSDirSeparator with PathDelim

  Revision 1.1  2000/07/13 06:30:59  michael
  + Initial import

  Revision 1.7  2000/02/09 16:59:32  peter
    * truncated log

}
