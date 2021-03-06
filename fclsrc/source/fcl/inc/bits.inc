{
    $Id: bits.inc,v 1.9 2003/05/25 16:05:18 jonas Exp $
    This file is part of the Free Component Library (FCL)
    Copyright (c) 1999-2000 by the Free Pascal development team

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}
{****************************************************************************}
{*                               TBits                                      *}
{****************************************************************************}

ResourceString
  SErrInvalidBitIndex = 'Invalid bit index : %d';
  SErrindexTooLarge   = 'Bit index exceeds array limit: %d';
  SErrOutOfMemory     = 'Out of memory';

Procedure BitsError (Msg : string);

begin
{$ifdef VER1_0}
  Raise EBitsError.Create(Msg) at get_caller_addr(get_frame);
{$else VER1_0}
  Raise EBitsError.Create(Msg) at pointer(get_caller_addr(get_frame));
{$endif VER1_0}
end;

Procedure BitsErrorFmt (Msg : string; const Args : array of const);

begin
{$ifdef VER1_0}
  Raise EBitsError.CreateFmt(Msg,args) at get_caller_addr(get_frame);
{$else VER1_0}
  Raise EBitsError.CreateFmt(Msg,args) at pointer(get_caller_addr(get_frame));
{$endif VER1_0}
end;

procedure TBits.CheckBitIndex (Bit : longint;CurrentSize : Boolean);

begin
 if (bit<0) or (CurrentSize and (Bit>Size)) then
   BitsErrorFmt(SErrInvalidBitIndex,[bit]);
 if (bit>=MaxBitFlags) then
   BitsErrorFmt(SErrIndexTooLarge,[bit])

end;

{ ************* functions to match TBits class ************* }

function TBits.getSize : longint;
begin
   result := (FSize shl BITSHIFT) - 1;
end;

procedure TBits.setSize(value : longint);
begin
   grow(value - 1);
end;

procedure TBits.SetBit(bit : longint; value : Boolean);
begin
   if value = True then
      seton(bit)
   else
      clear(bit);
end;

function TBits.OpenBit : longint;
var
   loop : longint;
   loop2 : longint;
   startIndex : longint;
begin
   result := -1; {should only occur if the whole array is set}
   for loop := 0 to FSize - 1 do
   begin
      if FBits^[loop] <> $FFFFFFFF then
      begin
         startIndex := loop * 32;
         for loop2 := startIndex to startIndex + 31 do
         begin
            if get(loop2) = False then
            begin
               result := loop2;
               break; { use this as the index to return }
            end;
         end;
         break;  {stop looking for empty bit in records }
      end;
   end;

   if result = -1 then
      if FSize < MaxBitRec then
          result := FSize * 32;  {first bit of next record}
end;

{ ******************** TBits ***************************** }

constructor TBits.Create(theSize : longint);
begin
   FSize := 0;
   FBits := nil;
   findIndex := -1;
   findState := True;  { no reason just setting it to something }
   grow(theSize);
end;

destructor TBits.Destroy;
begin
   if FBits <> nil then
      FreeMem(FBits, FSize * SizeOf(longint));
   FBits := nil;

   inherited Destroy;
end;

procedure TBits.grow(nbit : longint);
var
   newSize : longint;
   loop : longint;
begin
   CheckBitindex(nbit,false);

   newSize :=  (nbit shr BITSHIFT) + 1;

   if newSize > FSize then
   begin
      ReAllocMem(FBits, newSize * SizeOf(longint));
      if FBits <> nil then
        begin
         if newSize > FSize then
            for loop := FSize to newSize - 1 do
               FBits^[loop] := 0;
         FSize := newSize;
       end
      else
        BitsError(SErrOutOfMemory);
   end;
end;

function TBits.getFSize : longint;
begin
   result := FSize;
end;

procedure TBits.seton(bit : longint);
var
   n : longint;
begin
   n := bit shr BITSHIFT;
   grow(bit);
   FBits^[n] := FBits^[n] or (longint(1) shl (bit and MASK));
end;

procedure TBits.clear(bit : longint);
var
   n : longint;
begin
   CheckBitIndex(bit,false);
   n := bit shr BITSHIFT;
   grow(bit);
   FBits^[n] := FBits^[n] and not(longint(1) shl (bit and MASK));
end;

procedure TBits.clearall;
var
   loop : longint;
begin
   for loop := 0 to FSize - 1 do
      FBits^[loop] := 0;
end;

function TBits.get(bit : longint) : Boolean;
var
   n : longint;
begin
   CheckBitIndex(bit,true);
   result := False;
   n := bit shr BITSHIFT;
   if (n < FSize) then
      result := (FBits^[n] and (longint(1) shl (bit and MASK))) <> 0;
end;

procedure TBits.andbits(bitset : TBits);
var
   n : longint;
   loop : longint;
begin
   if FSize < bitset.getFSize then
      n := FSize - 1
   else
      n := bitset.getFSize - 1;

   for loop := 0 to n do
      FBits^[loop] := FBits^[loop] and bitset.FBits^[loop];

   for loop := n + 1 to FSize - 1 do
      FBits^[loop] := 0;
end;

procedure TBits.notbits(bitset : TBits);
var
   n : longint;
   jj : longint;
   loop : longint;
begin
   if FSize < bitset.getFSize then
      n := FSize - 1
   else
      n := bitset.getFSize - 1;

   for loop := 0 to n do
   begin
      jj := FBits^[loop];
      FBits^[loop] := FBits^[loop] and (jj xor bitset.FBits^[loop]);
   end;
end;

procedure TBits.orbits(bitset : TBits);
var
   n : longint;
   loop : longint;
begin
   if FSize < bitset.getFSize then
      n := bitset.getFSize - 1
   else
      n := FSize - 1;

   grow(n shl BITSHIFT);

   for loop := 0 to n do
      FBits^[loop] := FBits^[loop] or bitset.FBits^[loop];
end;

procedure TBits.xorbits(bitset : TBits);
var
   n : longint;
   loop : longint;
begin
   if FSize < bitset.getFSize then
      n := bitset.getFSize - 1
   else
      n := FSize - 1;

   grow(n shl BITSHIFT);

   for loop := 0 to n do
      FBits^[loop] := FBits^[loop] xor bitset.FBits^[loop];
end;

function TBits.equals(bitset : TBits) : Boolean;
var
   n : longint;
   loop : longint;
begin
   result := False;

   if FSize < bitset.getFSize then
      n := FSize - 1
   else
      n := bitset.getFSize - 1;

   for loop := 0 to n do
      if FBits^[loop] <> bitset.FBits^[loop] then exit;

   if FSize - 1 > n then
   begin
      for loop := n to FSize - 1 do
         if FBits^[loop] <> 0 then exit;
   end
   else if bitset.getFSize - 1 > n then
      for loop := n to bitset.getFSize - 1 do
         if bitset.FBits^[loop] <> 0 then exit;

   result := True;  {passed all tests}
end;


{ us this in place of calling FindFirstBit. It sets the current }
{ index used by FindNextBit and FindPrevBit                     }

procedure TBits.SetIndex(index : longint);
begin
   findIndex := index;
end;


{ When state is set to True it looks for bits that are turned On (1) }
{ and when it is set to False it looks for bits that are turned      }
{ off (0).                                                           }

function TBits.FindFirstBit(state : boolean) : longint;
var
   loop : longint;
   loop2 : longint;
   startIndex : longint;
   compareVal : cardinal;
begin
   result := -1; {should only occur if none are set}

   findState := state;

   if state = False then
      compareVal := $FFFFFFFF  { looking for off bits }
   else
      compareVal := $00000000; { looking for on bits }

   for loop := 0 to FSize - 1 do
   begin
      if FBits^[loop] <> compareVal then
      begin
         startIndex := loop * 32;
         for loop2 := startIndex to startIndex + 31 do
         begin
            if get(loop2) = state then
            begin
               result := loop2;
               break; { use this as the index to return }
            end;
         end;
         break;  {stop looking for bit in records }
      end;
   end;

   findIndex := result;
end;

function TBits.FindNextBit : longint;
var
   loop : longint;
   maxVal : longint;
begin
   result := -1;  { will occur only if no other bits set to }
                  { current findState                        }

   if findIndex > -1 then { must have called FindFirstBit first }
   begin                  { or set the start index              }
      maxVal := (FSize * 32) - 1;

      for loop := findIndex + 1 to maxVal  do
      begin
         if get(loop) = findState then
         begin
            result := loop;
            break;
         end;
      end;

      findIndex := result;
   end;
end;

function TBits.FindPrevBit : longint;
var
   loop : longint;
begin
   result := -1;  { will occur only if no other bits set to }
                  { current findState                        }

   if findIndex > -1 then { must have called FindFirstBit first }
   begin                  { or set the start index              }
      for loop := findIndex - 1 downto 0  do
      begin
         if get(loop) = findState then
         begin
            result := loop;
            break;
         end;
      end;

      findIndex := result;
   end;
end;


{
  $Log: bits.inc,v $
  Revision 1.9  2003/05/25 16:05:18  jonas
    * made Args parameter of BitsErrorFmt a const one

  Revision 1.8  2002/09/07 15:15:24  peter
    * old logs removed and tabs fixed

  Revision 1.7  2002/07/16 14:00:55  florian
    * raise takes now a void pointer as at and frame address
      instead of a longint, fixed

}
