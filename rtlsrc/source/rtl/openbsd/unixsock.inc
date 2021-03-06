{$Id: unixsock.inc,v 1.1.2.3 2003/03/05 13:14:42 marco Exp $}

{******************************************************************************
                          Basic Socket Functions
******************************************************************************}

Function socket(Domain,SocketType,Protocol:Longint):Longint;
begin
  Socket:=Do_Syscall(syscall_nr_socket,Domain,SocketType,Protocol);
end;

Function Send(Sock:Longint;Const Buf;BufLen,Flags:Longint):Longint;
begin
  Send:=do_syscall(syscall_nr_sendto,Sock,Longint(@Buf),BufLen,Flags,0,0);
end;

Function SendTo(Sock:Longint;Const Buf;BufLen,Flags:Longint;Var Addr; AddrLen : Longint):Longint;
begin
  Sendto:=do_syscall(syscall_nr_Sendto,Sock,Longint(@Buf),BufLen,Flags,Longint(@Addr),AddrLen);
end;

Function Recv(Sock:Longint;Var Buf;BufLen,Flags:Longint):Longint;
begin
  Recv:=do_syscall(syscall_nr_Recvfrom,Sock,Longint(@Buf),BufLen,Flags,0,0);
end;

Function RecvFrom(Sock : Longint; Var Buf; Buflen,Flags : Longint; Var Addr ; var AddrLen : Longint) : longint;

begin
  RecvFrom:=do_syscall(syscall_nr_Recvfrom,Sock,Longint(@buf),buflen,flags,Longint(@Addr),AddrLen);
end;

Function Bind(Sock:Longint;Const Addr;AddrLen:Longint):Boolean;
begin
  Bind:=(do_syscall(syscall_nr_Bind,Sock,Longint(@Addr),AddrLen)=0);
end;

Function Listen(Sock,MaxConnect:Longint):Boolean;
begin
  Listen:=(do_syscall(syscall_nr_Listen,Sock,MaxConnect,0)=0);
end;

Function Accept(Sock:Longint;Var Addr;Var Addrlen:Longint):Longint;
begin
  Accept:=do_syscall(syscall_nr_accept,Sock,longint(@Addr),longint(@AddrLen));
  If Accept<0 Then
    Accept:=-1;
end;

Function Connect(Sock:Longint;Const Addr;Addrlen:Longint): boolean;

begin
  Connect:=do_syscall(syscall_nr_connect,Sock,longint(@Addr),AddrLen)=0;
end;


Function Shutdown(Sock:Longint;How:Longint):Longint;
begin
  ShutDown:=do_syscall(syscall_nr_shutdown,Sock,How);
end;


Function GetSocketName(Sock:Longint;Var Addr;Var Addrlen:Longint):Longint;
begin
  GetSocketName:=do_syscall(syscall_nr_GetSockName,Sock,longint(@Addr),longint(@AddrLen));
end;



Function GetPeerName(Sock:Longint;Var Addr;Var Addrlen:Longint):Longint;
begin
  GetPeerName:=do_syscall(syscall_nr_GetPeerName,Sock,longint(@Addr),longint(@AddrLen));
end;



Function SetSocketOptions(Sock,Level,OptName:Longint;Const OptVal;optlen:longint):Longint;
begin
  SetSocketOptions:=do_syscall(syscall_nr_SetSockOpt,Sock,Level,OptName,Longint(@OptVal),OptLen,0);
end;



Function GetSocketOptions(Sock,Level,OptName:Longint;Var OptVal;Var optlen:longint):Longint;
begin
  GetSocketOptions:=do_syscall(syscall_nr_GetSockOpt,Sock,Level,OptName,Longint(@OptVal),OptLen,0);
end;



Function SocketPair(Domain,SocketType,Protocol:Longint;var Pair:TSockArray):Longint;
begin
  SocketPair:=do_syscall(syscall_nr_SocketPair,Domain,SocketType,Protocol,longint(@Pair),0,0);
end;

{******************************************************************************
                               UnixSock
******************************************************************************}

Procedure Str2UnixSockAddr(const addr:string;var t:TUnixSockAddr;var len:longint);
begin
  Move(Addr[1],t.Path,length(Addr));
  t.Family:=AF_UNIX;
  t.Path[length(Addr)]:=#0;
  Len:=Length(Addr)+3;
end;


Function Bind(Sock:longint;const addr:string):boolean;
var
  UnixAddr : TUnixSockAddr;
  AddrLen  : longint;
begin
  Str2UnixSockAddr(addr,UnixAddr,AddrLen);
  Bind(Sock,UnixAddr,AddrLen);
  Bind:=(SocketError=0);
end;



Function DoAccept(Sock:longint;var addr:string):longint;
var
  UnixAddr : TUnixSockAddr;
  AddrLen  : longint;
begin
  AddrLen:=length(addr)+3;
  DoAccept:=Accept(Sock,UnixAddr,AddrLen);
  Move(UnixAddr.Path,Addr[1],AddrLen);
  SetLength(Addr,AddrLen);
end;



Function DoConnect(Sock:longint;const addr:string):Boolean;
var
  UnixAddr : TUnixSockAddr;
  AddrLen  : longint;
begin
  Str2UnixSockAddr(addr,UnixAddr,AddrLen);
  DoConnect:=Connect(Sock,UnixAddr,AddrLen);
end;

Function Accept(Sock:longint;var addr:string;var SockIn,SockOut:text):Boolean;
var
  s : longint;
begin
  S:=DoAccept(Sock,addr);
  if S>0 then
   begin
     Sock2Text(S,SockIn,SockOut);
     Accept:=true;
   end
  else
   Accept:=false;
end;



Function Accept(Sock:longint;var addr:string;var SockIn,SockOut:File):Boolean;
var
  s : longint;
begin
  S:=DoAccept(Sock,addr);
  if S>0 then
   begin
     Sock2File(S,SockIn,SockOut);
     Accept:=true;
   end
  else
   Accept:=false;
end;



Function Connect(Sock:longint;const addr:string;var SockIn,SockOut:text):Boolean;
begin
  Connect:=DoConnect(Sock,addr);
  If Connect then
     Sock2Text(Sock,SockIn,SockOut);
end;



Function Connect(Sock:longint;const addr:string;var SockIn,SockOut:file):Boolean;
begin
  Connect:=DoConnect(Sock,addr);
  if Connect then
     Sock2File(Sock,SockIn,SockOut);
end;

{
 $Log: unixsock.inc,v $
 Revision 1.1.2.3  2003/03/05 13:14:42  marco
  * Fixed last param recvfrom from 16-bit to 32-bit

 Revision 1.1.2.2  2003/02/09 11:51:16  marco
  * a days work...

 Revision 1.1.2.1  2002/07/29 17:15:59  marco
  * Initial OpenBSD port. One days work. Still problems with initcode

 Revision 1.1.2.2  2002/03/04 07:13:49  marco
  * Something that was forgotten.

 Revision 1.1.2.4  2002/02/05 07:47:46  marco
  * Fixed typo


}