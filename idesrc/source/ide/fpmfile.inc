{
    $Id: fpmfile.inc,v 1.7 2002/10/12 19:43:07 hajny Exp $
    This file is part of the Free Pascal Integrated Development Environment
    Copyright (c) 1998 by Berczi Gabor

    File menu entries

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

procedure TIDEApp.NewEditor;
begin
  OpenEditorWindow(nil,'',0,0);
end;


procedure TIDEApp.NewFromTemplate;
var D: PCenterDialog;
    R,R2: TRect;
    SB: PScrollBar;
    LB: PAdvancedListBox;
    I: integer;
    C: PUnsortedStringCollection;
    TE: PSourceWindow;
begin
  if GetTemplateCount=0 then
     begin InformationBox(msg_notemplatesavailable,nil); Exit; end;
  New(C, Init(10,10));
  R.Assign(0,0,40,14);
  New(D, Init(R, dialog_newfromtemplate));
  with D^ do
  begin
    HelpCtx:=hcNewFromTemplate;
    GetExtent(R); R.Grow(-2,-2); Inc(R.A.Y); Dec(R.B.X,12);
    R2.Copy(R); R2.Move(1,0); R2.A.X:=R2.B.X-1;
    New(SB, Init(R2)); Insert(SB);

    New(LB, Init(R,1,SB));
    LB^.Default:=true;
    for I:=0 to GetTemplateCount-1 do
      C^.Insert(NewStr(GetTemplateName(I)));
    LB^.NewList(C);
    Insert(LB);
    Dec(R.A.Y); R.B.Y:=R.A.Y+1;
    Insert(New(PLabel, Init(R, label_availabletemplates, LB)));

    GetExtent(R2); R2.Grow(-2,-3); R2.A.X:=R.B.X+2; R2.B.Y:=R2.A.Y+2;
    Insert(New(PButton, Init(R2, button_OK, cmOK, bfDefault)));
    R2.Move(0,2);
    Insert(New(PButton, Init(R2, button_Cancel, cmCancel, bfNormal)));
  end;
  LB^.Select;
  if Desktop^.ExecView(D)=cmOK then
  begin
{    Desktop^.Lock;}
    TE:=OpenEditorWindow(nil,'',0,0);
    if TE<>nil then
    begin
      TE^.Editor^.SetModified(false); { if nothing changes, we don't need to save it }
      StartTemplate(LB^.Focused,TE^.Editor);
(*      TE^.Hide; { we need this trick to get the editor updated }
      TE^.Show;*)
    end;
{    Desktop^.UnLock;}
  end;
  Dispose(D, Done);
  Dispose(C, Done);
end;


procedure TIDEApp.Open(FileName: string);
var D: PFileDialog;
    OpenIt: boolean;
    DriveNumber : byte;
    StoreDir,StoreDir2 : DirStr;
    NewPSW : PSourceWindow;
begin
  OpenIt:=FileName<>'';
  DriveNumber:=0;
  if not OpenIt then
   begin
     GetDir(0,StoreDir);
     if (Length(FileDir)>1) and (FileDir[2]=':') then
       begin
         { does not assume that lowercase are greater then uppercase ! }
         if (FileDir[1]>='a') and (FileDir[1]<='z') then
           DriveNumber:=Ord(FileDir[1])-ord('a')+1
         else
           DriveNumber:=Ord(FileDir[1])-ord('A')+1;
         GetDir(DriveNumber,StoreDir2);
{$ifndef FPC}
         ChDir(Copy(FileDir,1,2));
           { sets InOutRes in win32 PM }
{$endif not FPC}
       end;
     if (FileDir<>'') and ExistsDir(FileDir) then
       ChDir(TrimEndSlash(FileDir));
     New(D, Init(OpenExts,dialog_openafile,label_filetoopen,fdOpenButton,hidOpenSourceFile));
     D^.HelpCtx:=hcOpen;
     OpenIt:=Desktop^.ExecView(D)<>cmCancel;
     { if I go to root under go32v2 and there is no
       floppy I get a InOutRes = 152
       get rid of it ! }
     EatIO;
     if OpenIt then
       Begin
         D^.GetFileName(FileName);
         OpenExts:=D^.WildCard;
         if ExistsDir(DirOf(FExpand(FileName))) then
           FileDir:=DirOf(FExpand(FileName));
       End;
     Dispose(D, Done);
     if DriveNumber<>0 then
       ChDir(TrimEndSlash(StoreDir2));
{$ifndef FPC}
     if (Length(StoreDir)>1) and (StoreDir[2]=':') then
       ChDir(Copy(StoreDir,1,2));
{$endif not FPC}
     ChDir(TrimEndSlash(StoreDir));
   end;
  if OpenIt then
   begin
     FileName:=FExpand(LocatePasFile(FileName));
     if ExistsFile(FileName) then
     { like for BP unexistant files should  be created PM }
       OpenEditorWindow(nil,FileName,0,0)
     else
       {ErrorBox(FormatStrStr(msg_cantfindfile,FileName),nil);}
       begin
         NewPSW:=OpenEditorWindow(nil,'',0,0);
         NewPSW^.Editor^.FileName:=FileName;
         NewPSW^.SetTitle(FileName);
         Message(Application,evBroadcast,cmFileNameChanged,NewPSW^.Editor);
       end;
   end;
end;

function TIDEApp.OpenSearch(FileName: string) : boolean;
var D: PFileDialog;
    OpenIt: boolean;
    P : PString;
    Dir,S : String;
begin
  OpenIt:=False;
  if not OpenIt then
   begin
     ClearFormatParams; AddFormatParamStr(FileName);
     FormatStr(S,label_lookingfor,FormatParams);
     New(D, Init(FileName,dialog_openafile,S,fdOpenButton,hidOpenSourceFile));
     D^.HelpCtx:=hcOpen;
     OpenIt:=Desktop^.ExecView(D)<>cmCancel;
     if OpenIt then
       Begin
         D^.GetFileName(FileName);
       End;
     Dispose(D, Done);
   end;
  if OpenIt then
   begin
     FileName:=FExpand(LocatePasFile(FileName));
     Dir:=DirOf(FileName);
     P:=@Dir;
     If Pos(Dir+';',GetSourceDirectories)=0 then
       if ConfirmBox(msg_confirmsourcediradd,@P,false)=cmYes then
         begin
           SourceDirs:=SourceDirs+';'+Dir;
           {$IFNDEF NODEBUG}
           if assigned(Debugger) then
             Debugger^.SetDirectories;
           {$ENDIF}
         end;
     OpenEditorWindow(nil,FileName,0,0);
   end;
  OpenSearch:=OpenIt;
end;


procedure TIDEApp.OpenRecentFile(RecentIndex: integer);
begin
  with RecentFiles[RecentIndex] do
  if OpenEditorWindow(nil,FileName,LastPos.X,LastPos.Y)<>nil then
     RemoveRecentFile(RecentIndex);
end;

function TIDEApp.AskSaveAll: boolean;
function CanClose(P: PView): boolean; {$ifndef FPC}far;{$endif}
begin
  CanClose:=not P^.Valid(cmAskSaveAll);
end;
begin
  AskSaveAll:=Desktop^.FirstThat(@CanClose)=nil;
end;

function TIDEApp.SaveAll: boolean;

  procedure SendSave(P: PView); {$ifndef FPC}far;{$endif}
  begin
    Message(P,evCommand,cmSave,nil);
  end;

begin
  SaveCancelled:=false;
  Desktop^.ForEach(@SendSave);
  SaveAll:=not SaveCancelled;
end;


procedure TIDEApp.ChangeDir;
var
  D : PChDirDialog;
begin
  New(D, Init(cdNormal, hisChDirDialog));
  D^.HelpCtx:=hcChangeDir;
  ExecuteDialog(D,nil);
  CurDirChanged;
end;

{
  $Log: fpmfile.inc,v $
  Revision 1.7  2002/10/12 19:43:07  hajny
    * missing HasSignal conditionals added (needed for FPC/2)

  Revision 1.6  2002/09/07 15:40:43  peter
    * old logs removed and tabs fixed

  Revision 1.5  2002/08/29 09:58:04  pierre
   * remove wrong log line

  Revision 1.4  2002/08/29 09:31:53  pierre
   * set modified flag to false before prompt
     dialog are executed in NewFromTemplate

  Revision 1.3  2002/04/10 22:38:11  pierre
   * allow to open non existant file

}
