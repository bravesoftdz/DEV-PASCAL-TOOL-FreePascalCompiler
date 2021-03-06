{ Thread management routines }

const
  CM_EXECPROC = $8FFF;
  CM_DESTROYWINDOW = $8FFE;

type
  PRaiseFrame = ^TRaiseFrame;
  TRaiseFrame = record
    NextRaise: PRaiseFrame;
    ExceptAddr: Pointer;
    ExceptObject: TObject;
    ExceptionRecord: pointer; {PExceptionRecord}
  end;

var
  ThreadWindow: HWND;
  ThreadCount: Integer;

function ThreadWndProc(Window: HWnd; AMessage:UInt; WParam : WParam; LParam: LParam): Longint; stdcall;

begin
  case AMessage of
    CM_EXECPROC:
      with TThread(lParam) do
      begin
        Result := 0;
        try
          FSynchronizeException := nil;
          FMethod;
        except
{          if RaiseList <> nil then
          begin
            FSynchronizeException := PRaiseFrame(RaiseList)^.ExceptObject;
            PRaiseFrame(RaiseList)^.ExceptObject := nil;
          end; }
        end;
      end;
    CM_DESTROYWINDOW:
      begin
        DestroyWindow(Window);
        Result := 0;
      end;
  else
    Result := DefWindowProc(Window, AMessage, wParam, lParam);
  end;
end;

const
  ThreadWindowClass: TWndClass = (
    style: 0;
    lpfnWndProc: nil;
    cbClsExtra: 0;
    cbWndExtra: 0;
    hInstance: 0;
    hIcon: 0;
    hCursor: 0;
    hbrBackground: 0;
    lpszMenuName: nil;
    lpszClassName: 'TThreadWindow');

procedure AddThread;

  function AllocateWindow: HWND;
  var
    TempClass: TWndClass;
    ClassRegistered: Boolean;
  begin
    ThreadWindowClass.hInstance := HInstance;
    ThreadWindowClass.lpfnWndProc:=WndProc(@ThreadWndProc);
    ClassRegistered := GetClassInfo(HInstance, ThreadWindowClass.lpszClassName,
      @TempClass);
    if not ClassRegistered or (TempClass.lpfnWndProc <> WndProc(@ThreadWndProc)) then
    begin
      if ClassRegistered then
        Windows.UnregisterClass(ThreadWindowClass.lpszClassName, HInstance);
      Windows.RegisterClass(ThreadWindowClass);
    end;
    Result := CreateWindow(ThreadWindowClass.lpszClassName, '', 0,
      0, 0, 0, 0, 0, 0, HInstance, nil);
  end;

begin
  if ThreadCount = 0 then
    ThreadWindow := AllocateWindow;
  Inc(ThreadCount);
end;

procedure RemoveThread;
begin
  Dec(ThreadCount);
  if ThreadCount = 0 then
    PostMessage(ThreadWindow, CM_DESTROYWINDOW, 0, 0);
end;

{ TThread }

function ThreadProc(Thread: TThread): Integer;
var
  FreeThread: Boolean;
begin
  Thread.Execute;
  FreeThread := Thread.FFreeOnTerminate;
  Result := Thread.FReturnValue;
  Thread.FFinished := True;
  Thread.DoTerminate;
  if FreeThread then Thread.Free;
  ExitThread(Result);
end;

constructor TThread.Create(CreateSuspended: Boolean);
var
  Flags: Integer;
begin
  inherited Create;
  AddThread;
  FSuspended := CreateSuspended;
  Flags := 0;
  if CreateSuspended then Flags := CREATE_SUSPENDED;
  IsMultiThread := TRUE;
  FHandle := CreateThread(nil, 0, @ThreadProc, Pointer(self), Flags, DWord(FThreadID));
end;


destructor TThread.Destroy;
begin
  if not FFinished and not Suspended then
  begin
    Terminate;
    WaitFor;
  end;
  if FHandle <> 0 then CloseHandle(FHandle);
  inherited Destroy;
  RemoveThread;
end;

procedure TThread.CallOnTerminate;
begin
  FOnTerminate(Self);
end;

procedure TThread.DoTerminate;
begin
  if Assigned(FOnTerminate) then
    Synchronize(@CallOnTerminate);
end;

const
  Priorities: array [TThreadPriority] of Integer =
   (THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL,
    THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
    THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_TIME_CRITICAL);

function TThread.GetPriority: TThreadPriority;
var
  P: Integer;
  I: TThreadPriority;
begin
  P := GetThreadPriority(FHandle);
  Result := tpNormal;
  for I := Low(TThreadPriority) to High(TThreadPriority) do
    if Priorities[I] = P then Result := I;
end;

procedure TThread.SetPriority(Value: TThreadPriority);
begin
  SetThreadPriority(FHandle, Priorities[Value]);
end;

procedure TThread.Synchronize(Method: TThreadMethod);
begin
  FSynchronizeException := nil;
  FMethod := Method;
  SendMessage(ThreadWindow, CM_EXECPROC, 0, Longint(Self));
  if Assigned(FSynchronizeException) then raise FSynchronizeException;
end;

procedure TThread.SetSuspended(Value: Boolean);
begin
  if Value <> FSuspended then
    if Value then
      Suspend else
      Resume;
end;

procedure TThread.Suspend;
begin
  FSuspended := True;
  SuspendThread(FHandle);
end;

procedure TThread.Resume;
begin
  if ResumeThread(FHandle) = 1 then FSuspended := False;
end;

procedure TThread.Terminate;
begin
  FTerminated := True;
end;

function TThread.WaitFor: Integer;
var
  Msg: TMsg;
begin
  if GetCurrentThreadID = MainThreadID then
    while MsgWaitForMultipleObjects(1, FHandle, False, INFINITE, QS_SENDMESSAGE) = WAIT_OBJECT_0 + 1 do
      PeekMessage(Msg, 0, 0, 0, PM_NOREMOVE)
  else
    WaitForSingleObject(ulong(FHandle), INFINITE);
  GetExitCodeThread(FHandle, DWord(Result));
end;
{
  $Log: thread.inc,v $
  Revision 1.7  2003/04/23 11:35:30  peter
    * wndproc definition fix

  Revision 1.6  2002/09/07 15:15:29  peter
    * old logs removed and tabs fixed

}
