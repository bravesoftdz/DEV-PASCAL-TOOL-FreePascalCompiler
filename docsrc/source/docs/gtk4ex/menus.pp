Unit menus;

{$mode objfpc}

Interface

Uses Gtk,Gdk,Glib;


Function AddMenuToMenuBar (MenuBar : PGtkMenuBar; 
                           ShortCuts : PGtkAccelGroup;
                           Caption : AnsiString;
                           CallBack : TgtkSignalFunc;
                           CallBackdata : Pointer;
                           AlignRight : Boolean;
                           Var MenuItem : PgtkMenuItem
                           ) : PGtkMenu; 

Function AddItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkMenuItem; 

Function AddCheckItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkCheckMenuItem; 

Function AddImageItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        Bitmap : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkMenuItem; 

Function AddSeparatorToMenu(Menu : PgtkMenu) : PgtkMenuItem;

Implementation

Function AddMenuToMenuBar (MenuBar : PGtkMenuBar; 
                           ShortCuts : PGtkAccelGroup;
                           Caption : AnsiString;
                           CallBack : TgtkSignalFunc;
                           CallBackdata : Pointer;
                           AlignRight : Boolean;
                           Var MenuItem : PgtkMenuItem
                           ) : PGtkMenu; 

Var
  Key : guint;
  TheLabel : PGtkLabel;

begin
  MenuItem:=pgtkmenuitem(gtk_menu_item_new_with_label(''));
  If AlignRight Then
    gtk_menu_item_right_justify(MenuItem);
  TheLabel:=GTK_LABEL(GTK_BIN(MenuItem)^.child);
  Key:=gtk_label_parse_uline(TheLabel,Pchar(Caption));
  If Key<>0 then
    gtk_widget_add_accelerator(PGtkWidget(MenuItem),'activate_item',
                               Shortcuts,Key,
                               GDK_MOD1_MASK,GTK_ACCEL_LOCKED);         
  Result:=PGtkMenu(gtk_menu_new);
  If CallBack<>Nil then
    gtk_signal_connect(PGtkObject(result),'activate',
                        CallBack,CallBackdata);
  gtk_widget_show(PgtkWidget(MenuItem));  
  gtk_menu_item_set_submenu(MenuItem, PgtkWidget(Result));
  gtk_menu_bar_append(MenuBar,PgtkWidget(MenuItem));
end;

Function AddItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkMenuItem; 

Var
  Key,Modifiers : guint;
  LocalAccelGroup : PGtkAccelGroup;                      
  TheLabel : PGtkLabel;
  
begin
  Result:=pgtkmenuitem(gtk_menu_item_new_with_label(''));
  TheLabel:=GTK_LABEL(GTK_BIN(Result)^.child);
  Key:=gtk_label_parse_uline(TheLabel,Pchar(Caption));
  If Key<>0 then
     begin
{ $ifndef win32}
     LocalAccelGroup:=gtk_menu_ensure_uline_accel_group(Menu);
{ $endif}     
     gtk_widget_add_accelerator(PGtkWidget(result),'activate_item',
                                LocalAccelGroup,Key,
                                0,TGtkAccelFlags(0));         
     end;
  gtk_menu_append(Menu,pgtkWidget(result));
  If (ShortCut<>'') and (ShortCuts<>Nil) then  
    begin
    gtk_accelerator_parse (pchar(ShortCut), @key, @modifiers);
    gtk_widget_add_accelerator(PGtkWidget(result),'activate_item',
                               ShortCuts,Key,
                               modifiers, GTK_ACCEL_VISIBLE);
    end;
  If CallBack<>Nil then  
    gtk_signal_connect(PGtkObject(result),'activate',
                       CallBack,CallBackdata);
  gtk_widget_show(PgtkWidget(result));  
end;                       

Function AddCheckItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkCheckMenuItem; 

Var
  Key,Modifiers : guint;
  LocalAccelGroup : PGtkAccelGroup;                      
  TheLabel : PGtkLabel;
  
begin
  Result:=pgtkcheckmenuitem(gtk_check_menu_item_new_with_label(PChar(Caption)));
  gtk_check_menu_item_set_show_toggle(Result,True);  
  gtk_menu_append(Menu,pgtkWidget(result));
  If (ShortCut<>'') and (ShortCuts<>Nil) then  
    begin
    gtk_accelerator_parse (pchar(ShortCut), @key, @modifiers);
    gtk_widget_add_accelerator(PGtkWidget(result),'activate_item',
                               ShortCuts,Key,
                               modifiers, GTK_ACCEL_VISIBLE);
    end;
  If CallBack<>Nil then  
    gtk_signal_connect(PGtkObject(result),'toggled',
                       CallBack,CallBackdata);
  gtk_widget_show(PgtkWidget(result));  
end;                       

Function AddImageItemToMenu (Menu : PGtkMenu; 
                        ShortCuts : PGtkAccelGroup;
                        Caption : AnsiString;
                        ShortCut : AnsiString;
                        Bitmap : AnsiString;
                        CallBack : TgtkSignalFunc;
                        CallBackdata : Pointer
                       ) : PGtkMenuItem; 

Var
  Key,Modifiers : guint;
  LocalAccelGroup : PGtkAccelGroup;                      
  TheLabel : PGtkLabel;
  Image : PGtkPixmap;
  hbox : PGtkHBox;
  pixmap : PGdkPixmap;
  BitMapdata : PGdkBitmap;
  
begin
  Result:=pgtkmenuitem(gtk_menu_item_new);
  hbox:=PGtkHBox(gtk_hbox_new(false,0));
  gtk_container_add(pgtkcontainer(result),pgtkWidget(hbox));
  pixmap:=gdk_pixmap_create_from_xpm(Nil,@BitmapData,Nil,pchar(BitMap));
  Image := PgtkPixMap(gtk_pixmap_new(Pixmap,BitmapData));
  gtk_box_pack_start(PGtkBox(hbox),pgtkWidget(image),false,false,0);                                   
  TheLabel:=PgtkLabel(gtk_label_new(''));
  gtk_box_pack_start(PGtkBox(hbox),pgtkWidget(TheLabel),True,True,0);
  Key:=gtk_label_parse_uline(TheLabel,Pchar(Caption));
  If Key<>0 then
     begin
{ $ifndef win32}
     LocalAccelGroup:=gtk_menu_ensure_uline_accel_group(Menu);
{ $endif}
     gtk_widget_add_accelerator(PGtkWidget(result),'activate_item',
                                LocalAccelGroup,Key,
                                0,TGtkAccelFlags(0));         
     end;
  gtk_menu_append(Menu,pgtkWidget(result));
  If (ShortCut<>'') and (ShortCuts<>Nil) then  
    begin
    gtk_accelerator_parse (pchar(ShortCut), @key, @modifiers);
    gtk_widget_add_accelerator(PGtkWidget(result),'activate_item',
                               ShortCuts,Key,
                               modifiers, GTK_ACCEL_VISIBLE);
    end;
  If CallBack<>Nil then  
    gtk_signal_connect(PGtkObject(result),'activate',
                       CallBack,CallBackdata);
  gtk_widget_show_all(PgtkWidget(result));  
end;                       

Function AddSeparatorToMenu(Menu : PgtkMenu) : PgtkMenuItem;

begin
  Result:=pgtkmenuitem(gtk_menu_item_new());
  gtk_menu_append(Menu,pgtkWidget(result));
  gtk_widget_show(PgtkWidget(result));
end;

end.