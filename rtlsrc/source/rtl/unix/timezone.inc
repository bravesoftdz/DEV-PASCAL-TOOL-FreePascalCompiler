{
  $Id: timezone.inc,v 1.1.2.1 2000/09/14 13:38:26 marco Exp $

  Support for timezone info in /usr/share/timezone
}

type
  plongint=^longint;
  pbyte=^byte;

  ttzhead=packed record
    tzh_reserved : array[0..19] of byte;
    tzh_ttisgmtcnt,
    tzh_ttisstdcnt,
    tzh_leapcnt,
    tzh_timecnt,
    tzh_typecnt,
    tzh_charcnt  : longint;
  end;

  pttinfo=^tttinfo;
  tttinfo=packed record
    offset : longint;
    isdst  : boolean;
    idx    : byte;
    isstd  : byte;
    isgmt  : byte;
  end;

  pleap=^tleap;
  tleap=record
    transition : longint;
    change     : longint;
  end;

var
  num_transitions,
  num_leaps,
  num_types    : longint;

  transitions  : plongint;
  type_idxs    : pbyte;
  types        : pttinfo;
  zone_names   : pchar;
  leaps        : pleap;

function find_transition(timer:longint):pttinfo;
var
  i : longint;
begin
  if (num_transitions=0) or (timer<transitions[0]) then
   begin
     i:=0;
     while (i<num_types) and (types[i].isdst) do
      inc(i);
     if (i=num_types) then
      i:=0;
   end
  else
   begin
     for i:=1 to num_transitions do
      if (timer<transitions[i]) then
       break;
     i:=type_idxs[i-1];
   end;
  find_transition:=@types[i];
end;


procedure GetLocalTimezone(timer:longint;var leap_correct,leap_hit:longint);
var
  info : pttinfo;
  i    : longint;
begin
{ reset }
  TZDaylight:=false;
  TZSeconds:=0;
  TZName[false]:=nil;
  TZName[true]:=nil;
  leap_correct:=0;
  leap_hit:=0;
{ get info }
  info:=find_transition(timer);
  if not assigned(info) then
   exit;
  TZDaylight:=info^.isdst;
  TZSeconds:=info^.offset;
  i:=0;
  while (i<num_types) do
   begin
     tzname[types[i].isdst]:=@zone_names[types[i].idx];
     inc(i);
   end;
  tzname[info^.isdst]:=@zone_names[info^.idx];
  i:=num_leaps;
  repeat
    if i=0 then
     exit;
    dec(i);
  until (timer>leaps[i].transition);
  leap_correct:=leaps[i].change;
  if (timer=leaps[i].transition) and
     (((i=0) and (leaps[i].change>0)) or
      (leaps[i].change>leaps[i-1].change)) then
   begin
     leap_hit:=1;
     while (i>0) and
           (leaps[i].transition=leaps[i-1].transition+1) and
           (leaps[i].change=leaps[i-1].change+1) do
      begin
        inc(leap_hit);
        dec(i);
      end;
   end;
end;


procedure GetLocalTimezone(timer:longint);
var
  lc,lh : longint;
begin
  GetLocalTimezone(timer,lc,lh);
end;


procedure ReadTimezoneFile(fn:string);

  procedure decode(var l:longint);
  var
    k : longint;
    p : pbyte;
  begin
    p:=pbyte(@l);
    if (p[0] and (1 shl 7))<>0 then
     k:=not 0
    else
     k:=0;
    k:=(k shl 8) or p[0];
    k:=(k shl 8) or p[1];
    k:=(k shl 8) or p[2];
    k:=(k shl 8) or p[3];
    l:=k;
  end;

var
  f      : longint;
  tzdir  : string;
  tzhead : ttzhead;
  i      : longint;
  chars  : longint;
  buf    : pbyte;
begin
  if fn='' then
   fn:='localtime';
  if fn[1]<>'/' then
   begin
     tzdir:=getenv('TZDIR');
     if tzdir='' then
      tzdir:='/usr/share/zoneinfo';
     if tzdir[length(tzdir)]<>'/' then
      tzdir:=tzdir+'/';
     fn:=tzdir+fn;
   end;
  f:=fdopen(fn,Open_RdOnly);
  if f<0 then
   exit;
  i:=fdread(f,tzhead,sizeof(tzhead));
  if i<>sizeof(tzhead) then
   exit;
  decode(tzhead.tzh_timecnt);
  decode(tzhead.tzh_typecnt);
  decode(tzhead.tzh_charcnt);
  decode(tzhead.tzh_leapcnt);
  decode(tzhead.tzh_ttisstdcnt);
  decode(tzhead.tzh_ttisgmtcnt);

  num_transitions:=tzhead.tzh_timecnt;
  num_types:=tzhead.tzh_typecnt;
  chars:=tzhead.tzh_charcnt;

  reallocmem(transitions,num_transitions*sizeof(longint));
  reallocmem(type_idxs,num_transitions);
  reallocmem(types,num_types*sizeof(tttinfo));
  reallocmem(zone_names,chars);
  reallocmem(leaps,num_leaps*sizeof(tleap));

  fdread(f,transitions^,num_transitions*4);
  fdread(f,type_idxs^,num_transitions);

  for i:=0 to num_transitions-1 do
   decode(transitions[i]);

  for i:=0 to num_types-1 do
   begin
     fdread(f,types[i].offset,4);
     fdread(f,types[i].isdst,1);
     fdread(f,types[i].idx,1);
     decode(types[i].offset);
     types[i].isstd:=0;
     types[i].isgmt:=0;
   end;

  fdread(f,zone_names^,chars);

  for i:=0 to num_leaps-1 do
   begin
     fdread(f,leaps[i].transition,4);
     fdread(f,leaps[i].change,4);
     decode(leaps[i].transition);
     decode(leaps[i].change);
   end;

  getmem(buf,tzhead.tzh_ttisstdcnt);
  fdread(f,buf^,tzhead.tzh_ttisstdcnt);
  for i:=0 to tzhead.tzh_ttisstdcnt-1 do
   types[i].isstd:=byte(buf[i]<>0);
  freemem(buf);

  getmem(buf,tzhead.tzh_ttisgmtcnt);
  fdread(f,buf^,tzhead.tzh_ttisgmtcnt);
  for i:=0 to tzhead.tzh_ttisgmtcnt-1 do
   types[i].isgmt:=byte(buf[i]<>0);
  freemem(buf);
  fdclose(f);
end;

Const
  // Debian system; contains location of timezone file.
  TimeZoneLocationFile = '/etc/timezone'; 
  // SuSE has link in /usr/lib/zoneinfo/localtime to /etc/localtime
  // RedHat uses /etc/localtime
  TimeZoneFile = '/usr/lib/zoneinfo/localtime'; 
  AltTimeZoneFile = '/etc/localtime'; 
  
function GetTimezoneFile:string;
var
  f,len : longint;
  s : string;
  info : stat;
  
begin
  GetTimezoneFile:='';
  f:=fdopen(TimeZoneLocationFile,Open_RdOnly);
  if f>0 then
    begin
    len:=fdread(f,s[1],high(s));
    s[0]:=chr(len);
    len:=pos(#10,s);
    if len<>0 then
     s[0]:=chr(len-1);
    fdclose(f);
    GetTimezoneFile:=s;
    end
  // Try SuSE  
  else if fstat(TimeZoneFile,info) then
    GetTimeZoneFile:=TimeZoneFile
  // Try RedHat  
  else If fstat(AltTimeZoneFile,Info) then
      GetTimeZoneFile:=AltTimeZoneFile;
end;


procedure InitLocalTime;
begin
  ReadTimezoneFile(GetTimezoneFile);
  GetLocalTimezone(GetTimeOfDay);
end;


procedure DoneLocalTime;
begin
  if assigned(transitions) then
   freemem(transitions);
  if assigned(type_idxs) then
   freemem(type_idxs);
  if assigned(types) then
   freemem(types);
  if assigned(zone_names) then
   freemem(zone_names);
  if assigned(leaps) then
   freemem(leaps);
  num_transitions:=0;
  num_leaps:=0;
  num_types:=0;
end;

{
  $Log: timezone.inc,v $
  Revision 1.1.2.1  2000/09/14 13:38:26  marco
   * Moved from Linux dir. now start of generic unix dir, from which the
      really exotic features should be moved to the target specific dirs.

  Revision 1.1  2000/07/13 06:30:54  michael
  + Initial import

  Revision 1.3  1999/12/04 13:50:43  michael
  + Now time zone also works on SuSE and RedHat

  Revision 1.2  1999/12/02 11:25:31  peter
    * timezone.inc dep for linux.ppu
    * fixed sigsegvs with wrong timezone settigns

  Revision 1.1  1999/12/01 22:46:59  peter
    + timezone support

}
