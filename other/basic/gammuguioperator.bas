$INCLUDE "RAPIDQ.INC"

dim File as QFileStream
dim car as byte
DIM File1 AS QFILESTREAM
DIM File2 AS QFILESTREAM

DIM BitMap AS QBITMAP
    BitMap.Height = 100
    BitMap.Width = 100
    BitMap.Paint(0,0,0,0)
    
DIM BitMapOperator AS QBITMAP
    BitMapOperator.Height = 100
    BitMapOperator.Width = 100
    BitMapOperator.Paint(0,0,0,0)

    
DECLARE SUB CanvasPaint (Sender AS QCanvas)
DECLARE SUB CanvasPaintOperator (Sender AS QCanvas) 
DECLARE SUB StartupClick (Sender AS QButton)
DECLARE SUB OperatorClick (Sender AS QButton)
DECLARE SUB WelcomeClick (Sender AS QButton)
DECLARE SUB DealerClick (Sender AS QButton)
DECLARE SUB TabChange
DECLARE SUB OpenLogo
DECLARE SUB OpenLogoOperator
DECLARE SUB OpenTextWelcome
DECLARE SUB OpenTextDealer
DECLARE Function GammuError as string
CREATE MainForm AS QForm
  Center
  Height = 480
  Width = 640
  Caption = "GAMMU GUI 0.01"
  CREATE Tab AS QTabControl
    AddTabs "Main","SMS","Ringtones","Calendar","LOGO","WAP","NetMonitor"
    Width = MainForm.ClientWidth
    Height = MainForm.ClientHeight
    OnChange = TabChange
    HotTrack = True
    CREATE Panel0 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      BevelInner = bvLowered
   CREATE Button1 AS QBUTTON
        Caption = "GAMMU Version"
        Left = 12
        Top = 13
        Width=100
    END CREATE
    CREATE Button2 AS QBUTTON
        Caption = "Identify phone"
        Left = 14
        Top = 51
        TabOrder = 1
    END CREATE
    CREATE cal AS QBUTTON
        Caption = "Call number"
        Left = 12
        Top = 87
        TabOrder = 2
    END CREATE
    CREATE DialBox AS QRICHEDIT
        Left = 97
        Top = 89
        Height = 25
        TabOrder = 3
        AddStrings "DialNumber"
    END CREATE      
    END CREATE
    CREATE Panel1 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 1"
      BevelInner = bvLowered
      Visible = False
    END CREATE
    CREATE Panel2 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 2"
      BevelInner = bvLowered
      Visible = False      
    END CREATE
    CREATE Panel3 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 3"
      BevelInner = bvLowered
      Visible = False
    END CREATE
     CREATE Panel4 AS QPanel 
'' LOGOs main panel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 4"
      BevelInner = bvLowered
      Visible = False
    CREATE Label1 AS QLABEL
        Caption = "Startup"
        Left = 150
        Top = 38
        Width = 40
        Transparent = 1
    END CREATE
    CREATE Label2 AS QLABEL
        Caption = "Operator"
        Left = 155
        Top = 168
        Width = 48
        Transparent = 1
    END CREATE
    CREATE Label3 AS QLABEL
        Caption = "Welcome text"
        Left = 140
        Top = 240
        Width = 72
        Transparent = 1
    END CREATE
    CREATE Label4 AS QLABEL
        Caption = "Dealer text"
        Left = 149
        Top = 282
        Width = 64
        Transparent = 1
    END CREATE
    CREATE StartupPanel AS QPANEL
'' Startup logo
        Left = 104
        Top = 56
        Width = 145
        Height = 89
        CREATE Canvas1 AS QCANVAS
            Left = 34
            Top = 16
            Width = 84
            Height = 48
            Color = &HFF0000
            OnPaint=CanvasPaint           
        END CREATE
    END CREATE
    CREATE operatorPanel AS QPANEL
'' Operator logo panel  
        Left = 104
        Top = 184
        Width = 145
        TabOrder = 1
        CREATE CanvasOperator AS QCANVAS
            Left = 34
            Top = 16
            Width = 72
            Height = 14
            Color = &HFF0000
            OnPaint=CanvasPaintOperator           
        END CREATE        
    END CREATE
    CREATE EditWelcome AS QEDIT
'' Welcome message editbox   
        Text = "Welcome message"
        Left = 56
        Top = 256
        Width = 241
    END CREATE
    CREATE EditDealer AS QEDIT
'' Dealer message editbox   
        Left = 56
        Top = 296
        Width = 241
        Height = 17
        TabOrder = 3
    END CREATE
    CREATE PicturePanel AS QPANEL
'' Picture message panel   
        Left = 398
        Top = 59
        Width = 145
        Height = 57
        TabOrder = 4
    END CREATE
    CREATE CallerPanel AS QPANEL
''Caller group panel    
        Left = 398
        Top = 171
        Width = 145
        Height = 57
        TabOrder = 5
    END CREATE
    CREATE ButtonGetStartup AS QBUTTON
'' Startup logo - GET button    
        Caption = "Get"
        Left = 56
        Top = 56
        Width = 43
        TabOrder = 6
        OnClick = StartupClick
    END CREATE
    CREATE ButtonSetStartup AS QBUTTON
'' Startup logo - SET button    
        Caption = "Set"
        Left = 256
        Top = 56
        Width = 43
        TabOrder = 7
        OnClick=StartupClick
    END CREATE
    CREATE ButtonGetOperator AS QBUTTON
'' Operator logo - GET button    
        Caption = "Get"
        Left = 56
        Top = 184
        Width = 43
        TabOrder = 8
        OnClick=OperatorClick
    END CREATE
    CREATE ButtonSetOperator AS QBUTTON
'' Operator logo - SET button    
        Caption = "Set"
        Left = 256
        Top = 184
        Width = 43
        TabOrder = 9
        OnClick=OperatorClick
    END CREATE
    CREATE ButtonGetWelcome AS QBUTTON
'' Welcome text - GET button    
        Caption = "Get"
        Left = 8
        Top = 256
        Width = 43
        TabOrder = 10
        OnClick = WelcomeClick
    END CREATE
    CREATE ButtonSetWelcome AS QBUTTON
'' Welcome text - SET button    
        Caption = "Set"
        Left = 304
        Top = 256
        Width = 43
        TabOrder = 11
        OnClick = WelcomeClick
    END CREATE
    CREATE ButtonGetDealer AS QBUTTON
'' Dealer text - GET button    
        Caption = "Get"
        Left = 8
        Top = 296
        Width = 43
        TabOrder = 12
        OnClick = DealerClick
    END CREATE
    CREATE ButtonSetDealer AS QBUTTON
'' Dealer text - SET button    
        Caption = "Set"
        Left = 304
        Top = 296
        Width = 43
        TabOrder = 13
        OnClick = DealerClick
    END CREATE
    CREATE Buttonl9 AS QBUTTON
        Caption = "Set"
        Left = 552
        Top = 56
        Width = 43
        TabOrder = 14
    END CREATE
    CREATE Buttonl10 AS QBUTTON
        Caption = "Set"
        Left = 552
        Top = 168
        Width = 43
        TabOrder = 15
    END CREATE
    CREATE Buttonl11 AS QBUTTON
        Caption = "Get"
        Left = 344
        Top = 56
        Width = 43
        TabOrder = 16
    END CREATE
    CREATE Buttonl12 AS QBUTTON
        Caption = "Get"
        Left = 344
        Top = 168
        Width = 43
        TabOrder = 17
    END CREATE
      
    END CREATE
     CREATE Panel5 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 5"
      BevelInner = bvLowered
      Visible = False
    END CREATE
     CREATE Panel6 AS QPanel
      Top = 40
      Left = 5
      Width = Tab.ClientWidth - 10
      Height = Tab.ClientHeight - 50
      Caption = "Panel 6"
      BevelInner = bvLowered
      Visible = False
    END CREATE
  END CREATE
END CREATE


SUB TabChange
  SELECT CASE Tab.TabIndex
    CASE 0
      Panel0.Visible = True
      Panel1.Visible = False
      Panel2.Visible = False
      Panel3.Visible = False
      Panel4.Visible = False
      Panel5.Visible = False
      Panel6.Visible = False
    CASE 1
      Panel0.Visible = False
      Panel1.Visible = True
      Panel2.Visible = False
      Panel3.Visible = False
      Panel4.Visible = False
      Panel5.Visible = False
      Panel6.Visible = False
    CASE 2
      Panel0.Visible = False
      Panel1.Visible = False
      Panel2.Visible = True
      Panel3.Visible = False
      Panel4.Visible = False
      Panel5.Visible = False
      Panel6.Visible = False
    CASE 3
      Panel0.Visible = False
      Panel1.Visible = False
      Panel2.Visible = False
      Panel3.Visible = True
      Panel4.Visible = False
      Panel5.Visible = False
      Panel6.Visible = False
    CASE 4
      Panel0.Visible = False
      Panel1.Visible = False
      Panel2.Visible = False
      Panel3.Visible = False
      Panel4.Visible = True
      Panel5.Visible = False
      Panel6.Visible = False
    CASE 5
      Panel0.Visible = False
      Panel1.Visible = False
      Panel2.Visible = False
      Panel3.Visible = False
      Panel4.Visible = False
      Panel5.Visible = True
      Panel6.Visible = False
    CASE 6
      Panel0.Visible = False
      Panel1.Visible = False
      Panel2.Visible = False
      Panel3.Visible = False
      Panel4.Visible = False
      Panel5.Visible = False
      Panel6.Visible = True
  END SELECT
END SUB

function gammu(BYREF param$ as string) as string
     result$=""
     IF  File.Open(CurDir$+"\go.bat", fmOpenRead) = FALSE THEN
        ShowMessage("Could not open GAMMU")
     Else
''     print "eseguo ";CurDir$+"\go.bat "+param$
     SHELL (CurDir$+"\go.bat "+param$)
        IF File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = FALSE THEN
        ShowMessage ("Could not open output file.")
        return
        end if
        print pid
     File.Open("gammu.tmp",fmOpenRead)
     do
      File.read(car)
      result$=result$+chr$(car)
     loop until File.EOF=-1
     File.Close
     ShowMessage(result$)
     gammu$=result$

     end if
end function

function gammu2(BYREF param$ as string) as string
     IF  File.Open(CurDir$+"\go.bat", fmOpenRead) = FALSE THEN
        ShowMessage("Could not open GAMMU")
     Else
''         print "eseguo ";CurDir$+"\go.bat "+param$
         SHELL (CurDir$+"\go.bat "+param$)
     end if
end function


sub CallNumber
 if val(DialBox.text)=0 then
  ShowMessage("Please specify a number!")
  return
 end if
 param$="--dialvoice "+DialBox.text
 ShowMessage(param$)
 gammu param$
end sub

sub version
 param$="--version"
 gammu param$
end sub 

sub identify
 ShowMessage("This function may require some seconds. Don't worry, program can't lock up, ther is a time out of 10 seconds: if you will wait more, it means that program IS working. Press OK to start")
 param$="--identify"
 gammu param$
end sub 


SUB CanvasPaint (Sender AS QCanvas)
    Sender.Draw(0,0,Bitmap.BMP)
END SUB

SUB CanvasPaintOperator (Sender AS QCanvas)
    Sender.Draw(0,0,BitmapOperator.BMP)
END SUB


SUB StartupClick (Sender AS QButton)
    SELECT CASE Sender.Caption
        CASE "Get"
        results$=""
        '' Download logo from phone
        param$="--getbitmap STARTUP logo.nlm"
        gammu2 param$
        '' If download is successful, GAMMU.TMP SHOULD BE (but it isn't!!!) empty
''        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then
         if File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.size>1000 then 
              ShowMessage("Logo downloaded and saved to LOGO.NLM")
             '' Convert logo to ASCII format.
 ''            param$="--getbitmap STARTUP logo.nlm"
 ''            gammu2 param$
 ''            IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size>1000 then 
 ''               print "Logo converted to ASCII, now displaying..."
                '' Display logo on PC.            
                OpenLogo
                Canvas1.Repaint
                return
  ''           end if
          else
             ShowMessage("Non riuscito: "+GammuError)
             return
         end if         
        CASE "Set"
          DIM OpenDialog AS QOpenDialog
          OpenDialog.Filter = "Logo files|*.BMP;*.NLM|All Files|*.*"
          OpenDialog.FilterIndex = 1    '' Use "All Files" as our default
    IF OpenDialog.Execute THEN
            '' Convert logo into NLM format.
            param$="--copybitmap "+OpenDialog.FileName+" "+CurDir$+"\logo.nlm STARTUP"
            gammu2 param$
         IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then 
             print "Logo converted to LOGO.NLM"            
             '' Convert logo to ASCII format.
             param$="--copybitmap "+OpenDialog.FileName
             gammu2 param$
             IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size>1000 then 
                print "Logo converted to ASCII, now displaying..."
                '' Display logo on PC. 
                           
                OpenLogo
                Canvas1.repaint
                print "Logo displayed, now uploading..."
                '' Upload logo to phone.
                param$="--setbitmap STARTUP logo.nlm"
                gammu2 param$
                '' When uploading NLM file, if upload was successfull GAMMU.TMP is empty.
                 IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then 
                     ShowMessage("Upload Ok!")
                 else
                     ShowMessage("Upload failed: "+GammuError)
                 end if
              else  
                 ShowMessage("Can't convert '"+OpenDialog.FileName+"' to ASCII: "+GammuError)
              end if ''ASCII conversion
          else
             ShowMessage("Can't convert '"+OpenDialog.FileName+"' to  LOGO.NLM: "+GammuError)
          end if '' NLM conversion
       else
        showmessage("?!?!?")
       end if  '' Dialog.execute          
    END SELECT
    Canvas1.Repaint           '-- Tell Canvas to repaint itself.
END SUB    

SUB OpenLogo
     result$=""
     File.Open(CurDir$+"\gammu.tmp",fmOpenRead)
     do
      File.read(car)
     loop until car=13
     do
      File.read(car)
      result$=result$+chr$(car)
     loop until File.EOF=-1
     File.Close
     if len(result$)<100 then
      ShowMessage("Failed! Please retry - "+GammuError)
      return
     end if
     for y = 0 to 47
         for x=1 to 85
         if mid$(result$,y*86+x,1)="#" then
             Bitmap.Pset(x-2,y,&H000000)
         else 
         if mid$(result$,y*86+x,1)=" " then
             Bitmap.Pset(x-2,y,&HFFFFFF)
         end if
         end if
               Canvas1.Repaint 
         next 
      next
      Canvas1.Repaint   
end sub

SUB WelcomeClick (Sender AS QButton)
    SELECT CASE Sender.Caption
        CASE "Get"
        results$=""
        '' Download welcome text from phone
        param$="--getbitmap TEXT"
        gammu2 param$
        '' If download is successful, GAMMU.TMP SHOULD BE  empty
        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size<65 then
                ShowMessage("Text downloaded")
                '' Display text on PC.            
                OpenTextWelcome
                return
          else
             ShowMessage("Non riuscito: "+GammuError)
             return
         end if         
        CASE "Set"
             if len(EditWelcome.text)=0 then text$=chr$(34)+chr$(34) else text$=EditWelcome.text
             param$="--setbitmap TEXT "+text$
             gammu2 param$ 
        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then
              ShowMessage("Welcome message changed to '"+EditWelcome.text+"'")
        else
              ShowMEssage("ERROR, Welcome Message not changed: "+GammuError)
        end if                         
    END SELECT
    Canvas1.Repaint       '-- Tell Canvas to repaint itself.
    END SUB    
    
SUB DealerClick (Sender AS QButton)
    SELECT CASE Sender.Caption
        CASE "Get"
        results$=""
        '' Download welcome text from phone
        param$="--getbitmap DEALER"
        gammu2 param$
        '' If download is successful, GAMMU.TMP SHOULD BE  empty
        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size<65 then
                ShowMessage("Dealer text downloaded")
                '' Display text on PC.            
                OpenTextDealer               
                return
          else
             ShowMessage("Non riuscito: "+GammuError)
             return
         end if         
        CASE "Set"
             if len(EditDealer.text)=0 then text$=chr$(34)+chr$(34) else text$=EditDealer.text
             param$="--setbitmap DEALER "+text$
             gammu2 param$ 
        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then
              ShowMessage("Dealer message changed to '"+Editdealer.text+"'")
        else  
              ShowMessage("ERROR, Dealer Message not changed: "+GammuError)
        end if        
    END SELECT
    Canvas1.Repaint       '-- Tell Canvas to repaint itself.
end sub

   

SUB OpenTextWelcome
     result$=""
     File.Open(CurDir$+"\gammu.tmp",fmOpenRead)
     do
      File.read(car)
      result$=result$+chr$(car)
     loop until File.EOF=-1
     File.Close
     count=1
     do
     count=count+1
     loop until mid$(result$,count,1)=chr$(34)
     testo$=""
     do
     count=count+1
     testo$=testo$+mid$(result$,count,1)
     loop until mid$(result$,count+1,1)=chr$(34)
     EditWelcome.text=testo$     
end sub

SUB OpenTextDealer
     result$=""
     File.Open(CurDir$+"\gammu.tmp",fmOpenRead)
     do
      File.read(car)
      result$=result$+chr$(car)
     loop until File.EOF=-1
     File.Close
     count=1
     do
     count=count+1
     loop until mid$(result$,count,1)=chr$(34)
     testo$=""
     do
     count=count+1
     testo$=testo$+mid$(result$,count,1)
     loop until mid$(result$,count+1,1)=chr$(34)
     EditDealer.text=testo$     
end sub

SUB OperatorClick (Sender AS QButton)
    SELECT CASE Sender.Caption
        CASE "Get"
        results$=""
        '' Download logo from phone
        param$="--getbitmap OPERATOR logo.nlm"
        gammu2 param$
        '' If download is successful, GAMMU.TMP SHOULD BE (but it isn't!!!) empty
''        IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then
         if File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.size>1000 then 
              ShowMessage("Logo downloaded and saved to LOGO.NLM")
             '' Convert logo to ASCII format.
 ''            param$="--getbitmap OPERATOR logo.nlm"
 ''            gammu2 param$
 ''            IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size>1000 then 
 ''               print "Logo converted to ASCII, now displaying..."
                '' Display logo on PC.            
                OpenLogo
                CanvasOperator.repaint
                return
  ''           end if
          else
             ShowMessage("Non riuscito: "+GammuError)
             return
         end if         
        CASE "Set"
          DIM OpenDialog AS QOpenDialog
          OpenDialog.Filter = "Logo files|*.BMP;*.NLM|All Files|*.*"
          OpenDialog.FilterIndex = 1    '' Use "All Files" as our default
    IF OpenDialog.Execute THEN
            '' Convert logo into NLM format.
            param$="--copybitmap "+OpenDialog.FileName+" "+CurDir$+"\logo.nlm OPERATOR"
            gammu2 param$
         IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then 
             print "Logo converted to LOGO.NLM"            
             '' Convert logo to ASCII format.
             param$="--copybitmap "+OpenDialog.FileName
             gammu2 param$
             IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size>1000 then 
''                ShowMessage( "Logo converted to ASCII, now displaying...")
                '' Display logo on PC.            
                OpenLogoOperator
                CanvasOperator.repaint
''                ShowMessage( "Logo displayed, now uploading...")
                '' Upload logo to phone.
                param$="--setbitmap OPERATOR logo.nlm"
                gammu2 param$
                '' When uploading NLM file, if upload was successfull GAMMU.TMP is empty.
                 IF  File.Open(CurDir$+"\gammu.tmp", fmOpenRead) = -1 and File.Size=0 then 
                     ShowMessage("Upload Ok!")
                 else
                     ShowMessage("Upload failed - "+GammuError)
                 end if
              else  
                 ShowMessage("Can't convert '"+OpenDialog.FileName+"' to  ASCII: '"+GammuError+"'")
              end if ''ASCII conversion
          else
             ShowMessage("Can't convert '"+OpenDialog.FileName+"' to  LOGO.NLM: "+GammuError)
          end if '' NLM conversion
       else
         
       end if  '' Dialog.execute          
    END SELECT
    CanvasOperator.Repaint        
end sub

SUB OpenLogoOperator
     result$=""
     File.Open(CurDir$+"\gammu.tmp",fmOpenRead)
''skip comment line
     do
      File.read(car)
     loop until car=13
     do
      File.read(car)
      result$=result$+chr$(car)
     loop until File.EOF=-1
     File.Close
     if len(result$)<100 then
      ShowMessage("Failed! Please retry - "+GammuError)
      return
     end if
     for y = 0 to 13
         for x=1 to 73
         if mid$(result$,y*74+x,1)="#" then
             BitmapOperator.Pset(x-2,y,&H000000)
         else 
         if mid$(result$,y*74+x,1)=" " then
             BitmapOperator.Pset(x-2,y,&HFFFFFF)
         end if
         end if
         next 
      next
      CanvasOperator.Repaint   
end sub

Function GammuError
     err$=""
     File.Open(CurDir$+"\gammu.tmp",fmOpenRead)     
     do
      File.read(car)
      err$=err$+chr$(car)
     loop until car=13
     File.Close
     Result=err$
end function     


Button1.OnClick = version   
Button2.OnClick = identify   
Cal.OnClick = CallNumber



MainForm.ShowModal

