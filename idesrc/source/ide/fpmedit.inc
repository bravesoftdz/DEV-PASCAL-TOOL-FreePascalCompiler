{
    $Id: fpmedit.inc,v 1.2 2002/09/07 15:40:43 peter Exp $
    This file is part of the Free Pascal Integrated Development Environment
    Copyright (c) 1998 by Berczi Gabor

    Edit menu entries

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

procedure TIDEApp.ShowClipboard;
begin
  ClipboardWindow^.Show;
  ClipboardWindow^.Focus;
end;

{
  $Log: fpmedit.inc,v $
  Revision 1.2  2002/09/07 15:40:43  peter
    * old logs removed and tabs fixed

}
