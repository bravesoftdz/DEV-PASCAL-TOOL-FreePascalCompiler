{
    $Id: astrings.inc,v 1.1.2.19 2003/02/26 19:21:50 jonas Exp $
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2000 by Michael Van Canneyt,
    member of the Free Pascal development team.

    This file implements AnsiStrings for FPC

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{ This will release some functions for special shortstring support }
{ define EXTRAANSISHORT}

{
  This file contains the implementation of the AnsiString type,
  and all things that are needed for it.
  AnsiString is defined as a 'silent' pchar :
  a pchar that points to :

  @-12 : Longint for maximum size;
  @-8  : Longint for size;
  @-4  : Longint for reference count;
  @    : String + Terminating #0;
  Pchar(Ansistring) is a valid typecast.
  So AS[i] is converted to the address @AS+i-1.

  Constants should be assigned a reference count of -1
  Meaning that they can't be disposed of.
}

Type
  PAnsiRec = ^TAnsiRec;
  TAnsiRec = Packed Record
    Maxlen,
    len,
    ref   : Longint;
    First : Char;
  end;

Const
  AnsiRecLen = SizeOf(TAnsiRec);
  FirstOff   = SizeOf(TAnsiRec)-1;


{****************************************************************************
                    Internal functions, not in interface.
****************************************************************************}

{$ifdef AnsiStrDebug}
Procedure DumpAnsiRec(S : Pointer);
begin
  If S=Nil then
    Writeln ('String is nil')
  Else
    Begin
      With PAnsiRec(S-Firstoff)^ do
       begin
         Write   ('(Maxlen: ',maxlen);
         Write   (' Len:',len);
         Writeln (' Ref: ',ref,')');
       end;
    end;
end;
{$endif}


Function NewAnsiString(Len : Longint) : Pointer;
{
  Allocate a new AnsiString on the heap.
  initialize it to zero length and reference count 1.
}
Var
  P : Pointer;
begin
  { Also add +1 for a terminating zero }
  GetMem(P,Len+AnsiRecLen);
  If P<>Nil then
   begin
     PAnsiRec(P)^.Maxlen:=Len;    { Maximal length }
     PAnsiRec(P)^.Len:=0;         { Initial length }
     PAnsiRec(P)^.Ref:=1;         { Set reference count }
     PAnsiRec(P)^.First:=#0;      { Terminating #0 }
     P:=P+FirstOff;               { Points to string now }
   end;
  NewAnsiString:=P;
end;


Procedure DisposeAnsiString(Var S : Pointer);
{
  Deallocates a AnsiString From the heap.
}
begin
  If S=Nil then
    exit;
  Dec (Longint(S),FirstOff);
  FreeMem (S);
  S:=Nil;
end;


Procedure AnsiStr_Decr_Ref (Var S : Pointer);[Public,Alias:'FPC_ANSISTR_DECR_REF'];
{
  Decreases the ReferenceCount of a non constant ansistring;
  If the reference count is zero, deallocate the string;
}
Type
  plongint = ^longint;
Var
  l : plongint;
Begin
  { Zero string }
  If S=Nil then exit;
  { check for constant strings ...}
  l:=@PANSIREC(S-FirstOff)^.Ref;
  If l^<0 then exit;
  If declocked(l^) then
    { Ref count dropped to zero }
    DisposeAnsiString (S);        { Remove...}
  { this pointer is not valid anymore, so set it to zero }
  S:=nil;
end;


Procedure AnsiStr_Incr_Ref (var S : Pointer);[Public,Alias:'FPC_ANSISTR_INCR_REF'];
Begin
  If S=Nil then
    exit;
  { Let's be paranoid : Constant string ??}
  If PAnsiRec(S-FirstOff)^.Ref<0 then exit;
  inclocked(PAnsiRec(S-FirstOff)^.Ref);
end;


Procedure AnsiStr_Assign (Var S1 : Pointer;S2 : Pointer);[Public,Alias:'FPC_ANSISTR_ASSIGN'];
{
  Assigns S2 to S1 (S1:=S2), taking in account reference counts.
}
begin
  If S2<>nil then
    If PAnsiRec(S2-FirstOff)^.Ref>0 then
      inclocked(PAnsiRec(S2-FirstOff)^.ref);
  { Decrease the reference count on the old S1 }
  ansistr_decr_ref (S1);
  { And finally, have S1 pointing to S2 (or its copy) }
  S1:=S2;
end;


Procedure AnsiStr_Concat (S1,S2 : Pointer;var S3 : Pointer);[Public, alias: 'FPC_ANSISTR_CONCAT'];
{
  Concatenates 2 AnsiStrings : S1+S2.
  Result Goes to S3;
}
Var
  Size,Location : Longint;
begin
{ only assign if s1 or s2 is empty }
  if (S1=Nil) then
    AnsiStr_Assign(S3,S2)
  else
    if (S2=Nil) then
      AnsiStr_Assign(S3,S1)
  else
    begin
       { create new result }
       if S3<>nil then
         AnsiStr_Decr_Ref(S3);
       Size:=PAnsiRec(S2-FirstOff)^.Len;
       Location:=Length(AnsiString(S1));
       SetLength (AnsiString(S3),Size+Location);
       Move (S1^,S3^,Location);
       Move (S2^,(S3+location)^,Size+1);
    end;
end;


{$ifdef EXTRAANSISHORT}
Procedure AnsiStr_ShortStr_Concat (Var S1: AnsiString; Var S2 : ShortString);
{
  Concatenates a Ansi with a short string; : S2 + S2
}
Var
  Size,Location : Longint;
begin
  Size:=Length(S2);
  Location:=Length(S1);
  If Size=0 then
    exit;
  { Setlength takes case of uniqueness
    and alllocated memory. We need to use length,
    to take into account possibility of S1=Nil }
  SetLength (S1,Size+Length(S1));
  Move (S2[1],Pointer(Pointer(S1)+Location)^,Size);
  PByte( Pointer(S1)+length(S1) )^:=0; { Terminating Zero }
end;
{$endif EXTRAANSISHORT}


Procedure AnsiStr_To_ShortStr (Var S1 : ShortString;S2 : Pointer);[Public, alias: 'FPC_ANSISTR_TO_SHORTSTR'];
{
  Converts a AnsiString to a ShortString;
}
Var
  Size : Longint;
begin
  if S2=nil then
   S1:=''
  else
   begin
     Size:=PAnsiRec(S2-FirstOff)^.Len;
     If Size>high(S1) then
      Size:=high(S1);
     Move (S2^,S1[1],Size);
     byte(S1[0]):=Size;
   end;
end;


Procedure ShortStr_To_AnsiStr (Var S1 : Pointer; Const S2 : ShortString);[Public, alias: 'FPC_SHORTSTR_TO_ANSISTR'];
{
  Converts a ShortString to a AnsiString;
}
Var
  Size : Longint;
begin
  Size:=Length(S2);
  Setlength (AnsiString(S1),Size);
  if Size>0 then
   begin
     Move (S2[1],Pointer(S1)^,Size);
     { Terminating Zero }
     PByte(Pointer(S1)+Size)^:=0;
   end;
end;


Procedure Char_To_AnsiStr(var S1 : Pointer; c : Char);[Public, alias: 'FPC_CHAR_TO_ANSISTR'];
{
  Converts a ShortString to a AnsiString;
}
begin
  Setlength (AnsiString(S1),1);
  PByte(Pointer(S1))^:=byte(c);
  { Terminating Zero }
  PByte(Pointer(S1)+1)^:=0;
end;


Procedure PChar_To_AnsiStr(var a : ansistring;p : pchar);[Public,Alias : 'FPC_PCHAR_TO_ANSISTR'];
Var
  L : Longint;
begin
  if pointer(a)<>nil then
    begin
       AnsiStr_Decr_Ref(Pointer(a));
       pointer(a):=nil;
    end;
  if (not assigned(p)) or (p[0]=#0) Then
    Pointer(a):=nil
  else
    begin
      //!! Horribly inneficient, but I see no other way...
      L:=1;
      While P[l]<>#0 do
        inc (l);
      Pointer(a):=NewAnsistring(L);
      SetLength(A,L);
      Move (P[0],Pointer(A)^,L)
    end;
end;


Procedure CharArray_To_AnsiStr(var a : ansistring;p : pchar;l:longint);[Public,Alias : 'FPC_CHARARRAY_TO_ANSISTR'];
var
  i  : longint;
  hp : pchar;
begin
  if p[0]=#0 Then
    Pointer(a):=nil
  else
    begin
      { p[0] <> #0, checked above (JM) }
      hp:=p+1;
      i:=1;
      while (i<l) and (hp^<>#0) do
       begin
         inc(hp);
         inc(i);
       end;
      Pointer(a):=NewAnsistring(i);
      SetLength(A,i);
      Move (P[0],Pointer(A)^,i);
    end;
end;


Function AnsiStr_Compare(S1,S2 : Pointer): Longint;[Public,Alias : 'FPC_ANSISTR_COMPARE'];
{
  Compares 2 AnsiStrings;
  The result is
   <0 if S1<S2
   0 if S1=S2
   >0 if S1>S2
}
Var
  i,MaxI,Temp : Longint;
begin
  i:=0;
  Maxi:=Length(AnsiString(S1));
  temp:=Length(AnsiString(S2));
  If MaxI>Temp then
   MaxI:=Temp;
  Temp:=0;
  While (i<MaxI) and (Temp=0) do
   begin
     Temp:= PByte(S1+I)^ - PByte(S2+i)^;
     inc(i);
   end;
  if temp=0 then
   temp:=Length(AnsiString(S1))-Length(AnsiString(S2));
  AnsiStr_Compare:=Temp;
end;


Procedure AnsiStr_CheckZero(p : pointer);[Public,Alias : 'FPC_ANSISTR_CHECKZERO'];
begin
  if p=nil then
    HandleErrorFrame(201,get_frame);
end;


Procedure AnsiStr_CheckRange(len,index : longint);[Public,Alias : 'FPC_ANSISTR_RANGECHECK'];
begin
  if (index>len) or (Index<1) then
    HandleErrorFrame(201,get_frame);
end;


{$ifdef EXTRAANSISHORT}
Function AnsiStr_ShortStr_Compare (Var S1 : Pointer; Var S2 : ShortString): Longint;
{
  Compares a AnsiString with a ShortString;
  The result is
   <0 if S1<S2
   0 if S1=S2
   >0 if S1>S2
}
Var
  i,MaxI,Temp : Longint;
begin
  Temp:=0;
  i:=0;
  MaxI:=Length(AnsiString(S1));
  if MaxI>byte(S2[0]) then
    MaxI:=Byte(S2[0]);
  While (i<MaxI) and (Temp=0) do
   begin
     Temp:= PByte(S1+I)^ - Byte(S2[i+1]);
     inc(i);
   end;
  AnsiStr_ShortStr_Compare:=Temp;
end;
{$endif EXTRAANSISHORT}


{*****************************************************************************
                     Public functions, In interface.
*****************************************************************************}

Function Length (Const S : AnsiString) : Longint;
{
  Returns the length of an AnsiString.
  Takes in acount that zero strings are NIL;
}
begin
  If Pointer(S)=Nil then
    Length:=0
  else
    Length:=PAnsiRec(Pointer(S)-FirstOff)^.Len;
end;


Procedure SetLength (Var S : AnsiString; l : Longint);
{
  Sets The length of string S to L.
  Makes sure S is unique, and contains enough room.
}
Var
  Temp : Pointer;
  movelen: longint;
begin
   if (l>0) then
    begin
      if Pointer(S)=nil then
       begin
         { Need a complete new string...}
         Pointer(s):=NewAnsiString(l);
       end
      else
       If (PAnsiRec(Pointer(S)-FirstOff)^.Maxlen < L) or
          (PAnsiRec(Pointer(S)-FirstOff)^.Ref <> 1) then
        begin
          { Reallocation is needed... }
          Temp:=Pointer(NewAnsiString(L));
          if Length(S)>0 then
            begin
              if l < succ(length(s)) then
                movelen := l
              { also move terminating null }
              else movelen := succ(length(s));
              Move(Pointer(S)^,Temp^,movelen);
            end;
          ansistr_decr_ref(Pointer(S));
          Pointer(S):=Temp;
       end;
      { Force nil termination in case it gets shorter }
      PByte(Pointer(S)+l)^:=0;
      PAnsiRec(Pointer(S)-FirstOff)^.Len:=l;
    end
  else
    begin
      { Length=0 }
      if Pointer(S)<>nil then
       ansistr_decr_ref (Pointer(S));
      Pointer(S):=Nil;
    end;
end;


Procedure UniqueString(Var S : AnsiString); [Public,Alias : 'FPC_ANSISTR_UNIQUE'];
{
  Make sure reference count of S is 1,
  using copy-on-write semantics.
}
Var
  SNew : Pointer;
begin
  If Pointer(S)=Nil then
    exit;
  if PAnsiRec(Pointer(S)-Firstoff)^.Ref<>1 then
   begin
     SNew:=NewAnsiString (PAnsiRec(Pointer(S)-FirstOff)^.len);
     Move (Pointer(S)^,SNew^,PAnsiRec(Pointer(S)-FirstOff)^.len+1);
     PAnsiRec(SNew-FirstOff)^.len:=PAnsiRec(Pointer(S)-FirstOff)^.len;
     ansistr_decr_ref (Pointer(S));  { Thread safe }
     Pointer(S):=SNew;
   end;
end;


Function Copy (Const S : AnsiString; Index,Size : Longint) : AnsiString;
var
  ResultAddress : Pointer;
begin
  ResultAddress:=Nil;
  dec(index);
  if Index < 0 then
    Index := 0;
  { Check Size. Accounts for Zero-length S, the double check is needed because
    Size can be maxint and will get <0 when adding index }
  if (Size>Length(S)) or
     (Index+Size>Length(S)) then
   Size:=Length(S)-Index;
  If Size>0 then
   begin
     If Index<0 Then
      Index:=0;
     ResultAddress:=Pointer(NewAnsiString (Size));
     if ResultAddress<>Nil then
      begin
        Move (Pointer(Pointer(S)+index)^,ResultAddress^,Size);
        PAnsiRec(ResultAddress-FirstOff)^.Len:=Size;
        PByte(ResultAddress+Size)^:=0;
      end;
   end;
  Pointer(Copy):=ResultAddress;
end;


Function Pos (Const Substr : AnsiString; Const Source : AnsiString) : Longint;
var
  substrlen,
  maxi,
  i,j : longint;
  e   : boolean;
  S   : AnsiString;
  se  : Pointer;
begin
  i := 0;
  j := 0;
  substrlen:=Length(SubStr);
  maxi:=length(source)-substrlen;
  e:=(substrlen>0);
  while (e) and (i <= maxi) do
   begin
     inc (i);
     if Source[i]=SubStr[1] then
      begin
        S:=copy(Source,i,substrlen);
        Se:=pointer(SubStr);
        if AnsiStr_Compare(se,Pointer(S))=0 then
         begin
           j := i;
           break;
         end;
      end;
   end;
  pos := j;
end;


{ Faster version for a char alone. Must be implemented because   }
{ pos(c: char; const s: shortstring) also exists, so otherwise   }
{ using pos(char,pchar) will always call the shortstring version }
{ (exact match for first argument), also with $h+ (JM)           }
Function Pos (c : Char; Const s : AnsiString) : Longint;
var
  i: longint;
begin
  for i:=1 to length(s) do
   if s[i]=c then
    begin
      pos:=i;
      exit;
    end;
  pos:=0;
end;


Function ValAnsiFloat(Const S : AnsiString; Var Code : ValSInt): ValReal; [public, alias:'FPC_VAL_REAL_ANSISTR'];
Var
  SS : String;
begin
  AnsiStr_To_ShortStr(SS,Pointer(S));
  ValAnsiFloat := ValFloat(SS,Code);
end;


Function ValAnsiUnsignedInt (Const S : AnsiString; Var Code : ValSInt): ValUInt; [public, alias:'FPC_VAL_UINT_ANSISTR'];
Var
  SS : ShortString;
begin
  AnsiStr_To_ShortStr(SS,Pointer(S));
  ValAnsiUnsignedInt := ValUnsignedInt(SS,Code);
end;


Function ValAnsiSignedInt (DestSize: longint; Const S : AnsiString; Var Code : ValSInt): ValSInt; [public, alias:'FPC_VAL_SINT_ANSISTR'];
Var
  SS : ShortString;
begin
  AnsiStr_To_ShortStr (SS,Pointer(S));
  ValAnsiSignedInt := ValSignedInt(DestSize,SS,Code);
end;

Function ValAnsiUnsignedint64 (Const S : AnsiString; Var Code : ValSInt): qword; [public,alias:'FPC_VAL_QWORD_ANSISTR'];
Var
  SS : ShortString;
begin
  if length(S)>255 then
    code:=256
  else
    begin
       AnsiStr_To_ShortStr(SS,Pointer(S));
       ValAnsiUnsignedInt64 := ValQWord(SS,Code);
    end;
end;


Function ValAnsiSignedInt64 (Const S : AnsiString; Var Code : ValSInt): Int64; [public, alias:'FPC_VAL_INT64_ANSISTR'];
Var
  SS : ShortString;
begin
  if Length(S)>255 then
    Code:=0
  else
    begin
       AnsiStr_To_ShortStr (SS,Pointer(S));
       ValAnsiSignedInt64 := valInt64(SS,Code);
    end;
end;

{$IfDef SUPPORT_FIXED}
Function ValAnsiFixed(Const S : AnsiString; Var Code : ValSint): ValReal; [public, alias:'FPC_VAL_FIXED_ANSISTR'];
Var
  SS : String;
begin
  AnsiStr_To_ShortStr (SS,Pointer(S));
  ValAnsiFixed := Fixed(ValFloat(SS,Code));
end;
{$EndIf SUPPORT_FIXED}


procedure AnsiStr_Float(d : ValReal;len,fr,rt : longint;var s : ansistring);[public,alias:'FPC_ANSISTR_FLOAT'];
var
  ss : shortstring;
begin
  str_real(len,fr,d,treal_type(rt),ss);
  s:=ss;
end;


Procedure AnsiStr_Cardinal(C : Cardinal;Len : Longint; Var S : AnsiString);[Public,Alias : 'FPC_ANSISTR_CARDINAL'];
Var
  SS : ShortString;
begin
  int_str_cardinal(C,Len,SS);
  S:=SS;
end;



Procedure AnsiStr_Longint(L : Longint; Len : Longint; Var S : AnsiString);[Public,Alias : 'FPC_ANSISTR_LONGINT'];
Var
  SS : ShortString;
begin
  int_Str_Longint (L,Len,SS);
  S:=SS;
end;


Procedure Delete (Var S : AnsiString; Index,Size: Longint);

Var
  LS : Longint;

begin
  ls:=Length(S);
  If (Index>LS) or (Index<=0) or (Size<=0) then
    exit;
  UniqueString (S);
  If (Size>LS-Index) then   // Size+Index gives overflow ??
     Size:=LS-Index+1;
  If (Size<=LS-Index) then
    begin
    Dec(Index);
    Move(PByte(Pointer(S))[Index+Size],PByte(Pointer(S))[Index],LS-Index-Size+1);
    end;
  Setlength(S,LS-Size);
end;


Procedure Insert (Const Source : AnsiString; Var S : AnsiString; Index : Longint);
var
  Temp : AnsiString;
  LS : Longint;
begin
  If Length(Source)=0 then
   exit;
  if index <= 0 then
   index := 1;
  Ls:=Length(S);
  if index > LS then
   index := LS+1;
  Dec(Index);
  Pointer(Temp) := NewAnsiString(Length(Source)+LS);
  SetLength(Temp,Length(Source)+LS);
  If Index>0 then
    move (Pointer(S)^,Pointer(Temp)^,Index);
  Move (Pointer(Source)^,PByte(Temp)[Index],Length(Source));
  If (LS-Index)>0 then
    Move(PByte(Pointer(S))[Index],PByte(temp)[Length(Source)+index],LS-Index);
  S:=Temp;
end;


Function StringOfChar(c : char;l : longint) : AnsiString;
begin
  SetLength(StringOfChar,l);
  FillChar(Pointer(StringOfChar)^,Length(StringOfChar),c);
end;

Procedure SetString (Var S : AnsiString; Buf : PChar; Len : Longint);
begin
  SetLength(S,Len);
  If (Buf<>Nil) then
    begin
      Move (Buf[0],S[1],Len);
    end;
end;

function upcase(const s : ansistring) : ansistring;
var
  i : longint;
begin
  Setlength(result,length(s));
  for i := 1 to length (s) do
    result[i] := upcase(s[i]);
end;


function lowercase(const s : ansistring) : ansistring;
var
  i : longint;
begin
  Setlength(result,length(s));
  for i := 1 to length (s) do
    result[i] := lowercase(s[i]);
end;

{
  $Log: astrings.inc,v $
  Revision 1.1.2.19  2003/02/26 19:21:50  jonas
    * removed unused variable in ansistring version of setstring
    * fixed shortstring version of setstring

  Revision 1.1.2.18  2003/02/26 19:15:40  jonas
    * fixed setstring (+- like suggested by Dimitry Sibiryakov)

  Revision 1.1.2.17  2002/12/09 08:32:34  michael
  + Fixed range check error and others in Delete

  Revision 1.1.2.16  2002/10/21 19:30:57  jonas
    * fixed some buffer overflow errors in SetString (both short and
      ansistring versions)

  Revision 1.1.2.15  2002/10/19 17:05:19  michael
  + Added check for nil buffer to setstring

  Revision 1.1.2.14  2002/10/08 16:15:29  florian
    * ansistrings should be threadsafe now

  Revision 1.1.2.13  2002/09/14 11:50:12  carl
    * Delphi compatibility fix (with string routines)

  Revision 1.1.2.12  2001/07/09 12:17:26  jonas
    * refcount of destination in concat_ansistring was decreased twice if
      s1 or s2 was nil

  Revision 1.1.2.11  2001/07/04 12:07:09  jonas
    * removed DestSize parameter from declaration of ValAnsiSignedInt64

  Revision 1.1.2.10  2001/06/28 08:00:10  jonas
    * fixed ansistr_incr_ref() and the generic addref()

  Revision 1.1.2.9  2001/06/16 03:51:50  carl
  * bugfix of parameter ansistr_incr_ref which did not concord with x86 asm version

  Revision 1.1.2.8  2001/06/05 16:01:17  jonas
    * backmerged lowercase/uppercase for ansistrings from 1.1 branch

  Revision 1.1.2.7  2000/12/09 22:54:06  florian
    * helper name for qword-val fixed

  Revision 1.1.2.6  2000/12/09 22:37:31  florian
    * fixed last commit, under go32v2 it worked

  Revision 1.1.2.5  2000/12/08 13:46:36  jonas
    + added pos(char,ansistring), because there is also a pos(char,shortstring)
      and without the ansistring version, the shortstring version is always
      called when calling pos(char,pchar), even when using $h+ (because the
      first parameter matches exactly)

  Revision 1.1.2.4  2000/08/29 18:37:38  peter
    * fixed chararray to ansistring

  Revision 1.1.2.3  2000/08/24 07:13:46  jonas
    * fixed bug in setlength (it sometimes read after the end of the heap)

  Revision 1.1.2.1  2000/08/10 18:53:35  peter
    * int64 ansistring val support

  Revision 1.1  2000/07/13 06:30:44  michael
  + Initial import

  Revision 1.43  2000/07/04 07:57:46  pierre
   * Change Code to var param in ValAnsiUnsignedInt function

  Revision 1.42  2000/06/11 07:02:30  peter
    * UniqueAnsiString -> UniqueString for Delphi compatibility

  Revision 1.41  2000/05/18 17:04:48  peter
    * use freemem without size

  Revision 1.40  2000/02/09 16:59:29  peter
    * truncated log

  Revision 1.39  2000/01/07 16:41:33  daniel
    * copyright 2000

  Revision 1.38  2000/01/07 16:32:24  daniel
    * copyright 2000 added

  Revision 1.37  1999/11/28 11:24:04  sg
  * Fixed bug 722: If the start position of AnsiString Copy is less than 1,
    it will be set to 1 (same behaviour as in Delphi)

  Revision 1.36  1999/11/25 13:34:57  michael
  + Added Ansistring setstring call

  Revision 1.35  1999/11/06 14:35:38  peter
    * truncated log

  Revision 1.34  1999/11/02 23:57:54  peter
    * fixed copy where size+index could be < 0

  Revision 1.33  1999/10/27 14:27:49  florian
    * StringOfChar fixed, how can be a bug in two lines of code ?????

  Revision 1.32  1999/10/27 14:17:20  florian
    + StringOfChar

  Revision 1.31  1999/10/04 20:48:18  peter
    * pos function speed up by a factor 40 :)

}
