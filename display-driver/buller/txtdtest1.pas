unit txtdtest1;

(*********************************************************
 * LC-Display-Testprogramm für Standard-LC-Displaymodule *
 * für SED1278 Textdisplaycontroller und Kompatible (Oki)*
 * copyright (c) Pollin Electronic http://www.pollin.de  *
 * & Tassilo Heeg http://www.theeg.de th@theeg.de        *
 * 06.04.2002                                            *
 * Sourcecode für Borland Delphi 5                       *
 * für Windows 95/98/ME/NT/2000/XP                       *
 *********************************************************)


interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Menus, StdCtrls, Spin, Buttons,inifiles,txtdtest2;

type
  TCharAdr = array[0..79] of byte;
  TForm1 = class(TForm)
    Label1: TLabel;
    Memo1: TMemo;
    BitBtn2: TBitBtn;
    GroupBox1: TGroupBox;
    SpinEdit1: TSpinEdit;
    Label2: TLabel;
    Label3: TLabel;
    SpinEdit2: TSpinEdit;
    CheckBox1: TCheckBox;
    CheckBox2: TCheckBox;
    CheckBox3: TCheckBox;
    MainMenu1: TMainMenu;
    Druckerschnittstelle1: TMenuItem;
    BenutzerdefinierteZeichen1: TMenuItem;
    Beenden1: TMenuItem;
    BitBtn1: TBitBtn;
    Displaytyp1: TMenuItem;
    procedure Druckerschnittstelle1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure BenutzerdefinierteZeichen1Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure BitBtn2Click(Sender: TObject);
    procedure SpinEdit1Change(Sender: TObject);
    procedure SpinEdit2Change(Sender: TObject);
    procedure CheckBox1Click(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure Beenden1Click(Sender: TObject);
    procedure DisplaytypClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    { Private-Deklarationen }
    // Displayeigenschaften
    cTxtResX,cTxtResY: integer;
    cDispType: string;
    cCharAdr: TCharAdr; // Zuordnung Zeichenpositionen-Displayadressen
    cDispInit: BYTE; // Wert für Display-Init
    // sonstiges
    msdelay,delz: int64; // Benötigt, um Delay auszumessen
    printerport: word; // Adresse der verwendeten LPT-Schnittstelle
    BiDirLPT: Boolean; // true, wenn Printerport bidirektional
    UserChars: TUserChars; // Die benutzerdefinierten Zeichen
    DispTypeList: TStringList; // Liste der verfügbaren Displaytypen
    procedure delay(ms: integer); // verzögern um ms Millisekunden
    procedure WriteCtrl(d: byte);
    procedure WriteCtrlDirect(d: byte);
    procedure WriteData(d: byte);
    procedure WaitDispRdy;
    function ANSI2Disp(c: char): byte; // Zeichen vom Windows-Zeichensatz in den
                                       // Display-Zeichensatz konvertieren, soweit möglich
    procedure String2Array(var ca: TCharAdr; a,b: byte; s: string); // Liest kommaseparierte Werte aus String in Array
    function GetIniName: string; // Pfad und Name der INI-Datei
    procedure SelectDisplay(dt: string); //Displaytyp dt auswählen
  public
    { Public-Deklarationen }
  end;

var
  Form1: TForm1;

implementation

uses txtdtest3,zlportio;

{$R *.DFM}

procedure TForm1.Druckerschnittstelle1Click(Sender: TObject);
begin
  case printerport of
    $378: form3.combobox1.itemindex:=0;
    $278: form3.combobox1.itemindex:=1;
    else form3.combobox1.itemindex:=2;
  end;
  form3.edit1.text:='$'+IntToHex(printerport,4);
  if form3.combobox1.itemindex<2 then begin
    form3.edit1.enabled:=FALSE;
    form3.edit1.color:=clBtnFace;
  end else begin
    form3.edit1.enabled:=true;
    form3.edit1.color:=clWhite;
  end;
  form3.checkbox1.checked:=BiDirLPT;
  if form3.showmodal<>idOK then exit;
  BiDirLPT:=form3.checkbox1.checked;
  case form3.combobox1.itemindex of
    0: printerport:=$378;
    1: printerport:=$278;
    else printerport:=StrToInt(form3.edit1.text);
  end;
end;

procedure TForm1.FormCreate(Sender: TObject);
var myini: TIniFile;
    z,y: integer;
    mi: TMenuItem;
    mt: TStringList;
    s: string;
begin
  // Konfiguration (Printerport)
  myini:=TInifile.create(GetIniName);
  printerport:=myini.readinteger('COMMON','LPTADDRESS',$378);
  BiDirLPT:=myini.readbool('COMMON','LPTBIDIR',FALSE);
  // Benutzerdef. Zeichen
  for z:=0 to 7 do
    for y:=0 to 7 do
      UserChars[z][y]:=myini.readInteger('UserdefChars','CHAR'+IntToStr(z)+'_ROW'+IntToStr(y),0);
  // Displaytypen einlesen
  mt:=TStringList.create;
  DispTypeList:=TStringList.create; // Die gibt die Liste der gefundenen Typen
  myini.ReadSections(mt);
  displaytyp1.clear;
  for z:=0 to mt.count-1 do begin
    s:=UpperCase(mt[z]);
    if (s<>'COMMON') AND (s<>'USERDEFCHARS') then begin
      // Dies ist eine Section für einen Displaytyp
      DispTypeList.Add(mt[z]);
      mi:=TMenuItem.create(Displaytyp1);
      mi.caption:=mt[z];
      mi.OnClick:=DisplaytypClick;
      mi.Tag:=DispTypeList.count-1;
      Displaytyp1.add(mi);
    end;
  end;
  SelectDisplay(myini.readstring('COMMON','DISPLAYTYPE','2420ST_GYBN'));
  myini.free;
end;

procedure TForm1.FormShow(Sender: TObject);
var t: TDateTime;
begin
  t:=now+(1/(86400*2));
  msdelay:=0;
  delz:=$7FFFFFFFFFFFFFFF;
  // Schleifenlaufzeit ausmessen, Durchläufe in einer halben Sekunde
  // brachial, funktioniert aber
  while (now<t) AND (msdelay<delz) do
    inc(msdelay);
  msdelay:=msdelay div 500;
  // Prüfen, ob I/O-Treiber geladen
  IF NOT ZlIOStarted THEN
    ShowMessage('Der benötigte Treiber ZLPORTIO.SYS konnte nicht geladen werden. Das Testprogramm wird ohne diesen Treiber nicht mit Windows NT/2000/XP funktionieren!');
end;

procedure TForm1.Delay(ms: integer);
var t: TDateTime;
    z: integer;
begin
  t:=now+(5/86400);  // Begrenzt auf 5 Sekunden
  for z:=1 to ms do begin
    delz:=0;
    while (now<t) AND (delz<msdelay) do
      inc(delz);
  end;
end;

procedure TForm1.BenutzerdefinierteZeichen1Click(Sender: TObject);
var z,y: integer;
begin
  if CharDefForm.Execute(UserChars) then begin
    // Zeichen-Def. an Display schicken
    WriteCtrl(64);
    for z:=0 to 7 do
      for y:=0 to 7 do
        WriteData(UserChars[z][y]);
  end;
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
var myini: TIniFile;
    z,y: integer;
begin
  // Konfiguration (Printerport) sichern
  myini:=TInifile.create(GetIniName);
  myini.writeinteger('COMMON','LPTADDRESS',printerport);
  myini.writebool('COMMON','LPTBIDIR',BiDirLPT);
  // Benutzerdef. Zeichen
  for z:=0 to 7 do
    for y:=0 to 7 do
      myini.writeInteger('UserdefChars','CHAR'+IntToStr(z)+'_ROW'+IntToStr(y),UserChars[z][y]);
  myini.free;
end;

procedure TForm1.BitBtn2Click(Sender: TObject);  // Button: Text übertragen
// Text ausgeben
var z: integer;
    s: string;
    disptxt: string;
    lsa,adr: integer;
begin
  disptxt:='';
  for z:=0 to cTxtResY-1 do begin
    if z<Memo1.Lines.Count then
      s:=Memo1.Lines[z]
    else s:='';
    // Benutzerdef. Zeichen ersetzen
    s:=StringReplace(s,'#0',#00,[rfReplaceAll]);
    s:=StringReplace(s,'#1',#01,[rfReplaceAll]);
    s:=StringReplace(s,'#2',#02,[rfReplaceAll]);
    s:=StringReplace(s,'#3',#03,[rfReplaceAll]);
    s:=StringReplace(s,'#4',#04,[rfReplaceAll]);
    s:=StringReplace(s,'#5',#05,[rfReplaceAll]);
    s:=StringReplace(s,'#6',#06,[rfReplaceAll]);
    s:=StringReplace(s,'#7',#07,[rfReplaceAll]);
    // ggf. mit ' ' auffüllen
    while length(s)<cTxtResX do s:=s+' ';
    // s enthält jetzt eine Anzeigen-Zeile
    disptxt:=disptxt+copy(s,1,cTxtResX);
  end;
  // disptxt enthält jetzt der Reihe nach alle Displayzeilen
  lsa:=-1; // Merker für die aktuell im Display gültige Zeichen-Adresse
  for z:=0 to length(disptxt)-1 do begin
    adr:=cCharAdr[z];
    if adr<>lsa then begin
      WriteCtrl(128+adr);
      lsa:=adr;
    end;
    WriteData(ANSI2Disp(disptxt[z+1]));
    inc(lsa);
  end;
  // Cursor wieder richtig platzieren
  WriteCtrl(128+cCharAdr[((SpinEdit2.value-1)*cTxtResX)+SpinEdit1.value-1]);
end;

function TForm1.ANSI2Disp;
begin
  case c of
    'ä': result:=225;
    'ö': result:=239;
    'ü': result:=245;
    'ß': result:=226;
    else result:=ord(c);
  end;
end;

procedure TForm1.SpinEdit1Change(Sender: TObject);
begin
  // Cursor wieder richtig platzieren
  WriteCtrl(128+cCharAdr[((SpinEdit2.value-1)*cTxtResX)+SpinEdit1.value-1]);
end;

procedure TForm1.SpinEdit2Change(Sender: TObject);
begin
  // Cursor wieder richtig platzieren
  WriteCtrl(128+cCharAdr[((SpinEdit2.value-1)*cTxtResX)+SpinEdit1.value-1]);
end;

procedure TForm1.CheckBox1Click(Sender: TObject);
var d: byte;
begin
  d:=8;
  if CheckBox3.checked then d:=d or 4;   // buko Anzeige einschalten 
  if CheckBox1.checked then d:=d or 2;   // buko LinienCursoranzeigen
  if CheckBox2.checked then d:=d or 1;   // buko blinkenden Cursor anzeigen
  WriteCtrl(d);
end;

procedure TForm1.BitBtn1Click(Sender: TObject);  // buko: Button: Display initialisieren
var z,y: integer;
begin
  //Display initalisieren, Busy-Flag noch nicht auswerten
  //Diese 3er-Sequenz ist für manche Displaycontroller erforderlich,
  //falls deren Power-On-Reset-Schaltung nicht einwandfrei funktioniert

  // ==> Anpassung an den Hardwarezustand beim LPT-LCD-ExperimentierModul mit buko: markiert
  // buko: PC LPT Steuerport C2  an    LCD Pin 4 = Register Select, Low=Befehlsregister, High=Datenregister
  // buko: PC LPT Steuerport /C1, hardwaremaeszig negiertes Register  an  LCD Pin 5 = R/W
  // buko Steuerportregister LPT_BA + 2: /C0 /C1  C2  /C3  x x x x

  WriteCtrlDirect($30); // buko: war WriteCtrlDirect(cDispInit);  mit cDispInit = $3A
  // 5ms warten
  Delay(5);
  WriteCtrlDirect($30); // buko: war WriteCtrlDirect(cDispInit);  mit cDispInit = $3A
  // 1ms warten
  Delay(1);
  WriteCtrlDirect($30); // buko: war WriteCtrlDirect(cDispInit);  mit cDispInit = $3A
  // ab hier das Busy-Flag auswerten

  // SystemSet fuer mehrzeilige LCD ->  $38  an Steuerregister senden
    WriteCtrl($38);   // buko:  fuer 2 oder 4 zeilige Displays notwendiges Steuerwort

  WriteCtrl(1); // Clear Display     0000 0001
  WriteCtrl(2); // Cursor Home       0000 001x
  WriteCtrl(6); // EntryMode = Increment, no shift   0000 01 Inkr/Dekr =1 ScrollAnz/Curs=0

  // Cursor etc. einstellen
  CheckBox1Click(self);
  SpinEdit1Change(self);
  // Benutzerdef. Zeichen an Display schicken
  WriteCtrl(64);
  for z:=0 to 7 do
      for y:=0 to 7 do
        WriteData(UserChars[z][y]);
end;

procedure TForm1.WriteCtrlDirect;
begin
  PortWriteB(printerport,d);
//  war original fuer Pollin-Beschaltung:
//  PortWriteB(printerport+2,0+2+1); //R/W=0, RS=0, E=0
//  PortWriteB(printerport+2,0+2+0); //R/W=0, RS=0, E=1
//  PortWriteB(printerport+2,0+2+1); //R/W=0, RS=0, E=0

// buko: Anpassung an Display-ExperimentierModul Schaltung
// C2 an RegisterSet und /C1 an R/W , zufaellig aber keine Aenderungen der Bits!
  PortWriteB(printerport+2,0+2+1); // RS=0, R/W=0, E=0
  PortWriteB(printerport+2,0+2+0); // RS=0, R/W=0, E=1
  PortWriteB(printerport+2,0+2+1); // RS=0, R/W=0, E=0
end;

procedure TForm1.WriteCtrl;
begin
  WaitDispRdy;
  WriteCtrlDirect(d);
end;

procedure TForm1.WriteData;
begin
  WaitDispRdy;
  PortWriteB(printerport,d);
// war original fuer Pollin-Beschaltung:
//  PortWriteB(printerport+2,0+0+1); //R/W=0, RS=1, E=0
//  PortWriteB(printerport+2,0+0+0); //R/W=0, RS=1, E=1
//  PortWriteB(printerport+2,0+0+1); //R/W=0, RS=1, E=0

// buko: Anpassung an Display-ExperimentierModul Schaltung
// C2 an RegisterSet und /C1 an R/W
  PortWriteB(printerport+2,4+2+1); // RS=1, R/W=0, E=0
  PortWriteB(printerport+2,4+2+0); // RS=1, R/W=0, E=1
  PortWriteB(printerport+2,4+2+1); // RS=1, R/W=0, E=0

end;

procedure TForm1.WaitDispRdy;
// Warten, bis das Display bereit ist
var tmp: byte;
    t: TDateTime;

begin
  if BiDirLPT THEN begin
    // LPT im ECP-Control-Register auf PS/2-Mode setzen
    // nötig, falls es ein ECP-Port ist
    tmp:=PortReadB(printerport+$402);
    tmp:=tmp AND $1F;
    tmp:=tmp OR $20; // Bidirektionaler Modus = PS/2
    PortWriteB(printerport+$402,tmp);
    t:=now+(5/86400);  // Begrenzt auf 5 Sekunden
    repeat
      // Auf Eingang schalten und abfragen
      PortWriteB(printerport+2,$20+4+2+1); //In, R/W=1 RS=0 E=0
      PortWriteB(printerport+2,$20+4+2+0); //In, R/W=1 RS=0 E=1
      tmp:=PortReadB(printerport);
      PortWriteB(printerport+2,$20+4+2+1); //In, R/W=1 RS=0 E=0
    until ((tmp AND 128)=0) OR (now>t); // Maximal 5 Sekunden warten
    PortWriteB(printerport+2,0+2+1);//Out, R/W=0 RS=0 E=0
  end else
    Delay(3); // Ohne bidirektionalen Parallelport ist Handshaking nicht möglich
end;

procedure TForm1.Beenden1Click(Sender: TObject);
begin
  close;
end;

procedure TForm1.String2Array;
var zae: byte;
    p: integer;
begin
  if s='' then s:='0';
  for zae:=a to b do begin
    p:=pos(',',s);
    if p=0 then begin
      ca[zae]:=StrToInt(s);
      s:='0';
    end else begin
      ca[zae]:=StrToInt(copy(s,1,p-1));
      s:=copy(s,p+1,length(s)-p);
    end;
  end;
end;

function TForm1.GetIniName;
begin
  result:=CHangeFileExt(Application.exename,'.INI');
end;

procedure TForm1.SelectDisplay;
var z: integer;
    myini: TIniFile;
begin
  myini:=TInifile.create(GetIniName);
  cDispType:=dt;
  // Displayeigenschaften
  cTxtResX:=myini.readinteger(cDispType,'TxtResX',20); // Anzahl Spalten
  cTxtResY:=myini.readinteger(cDispType,'TxtResY',4); // Anzahl Zeilen
  cDispInit:=myini.readinteger(cDispType,'DispInit',$3C); // Initialisierungswert
  // Zuordnung Zeichenpositionen-Displayadressen:
  String2Array(cCharAdr,0,19,myini.readstring(cDispType,'CharAdr1','00,01,02,03,04,05,06,07,08,09,10,11,12,13,14,15,16,17,18,19'));
  String2Array(cCharAdr,20,39,myini.readstring(cDispType,'CharAdr2','64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83'));
  String2Array(cCharAdr,40,59,myini.readstring(cDispType,'CharAdr3','20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39'));
  String2Array(cCharAdr,60,79,myini.readstring(cDispType,'CharAdr4','84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103'));
  // Standard-Textanzeige
  Memo1.Lines.Clear;
  Memo1.Lines.add(myini.readstring(cDispType,'Text1',' #3#1 Pollin'));
  Memo1.Lines.add(myini.readstring(cDispType,'Text2','#2#0#0 ELECTRONIC'));
  Memo1.Lines.add(myini.readstring(cDispType,'Text3','Tel.: 08403/920-920'));
  Memo1.Lines.add(myini.readstring(cDispType,'Text4','http://www.pollin.de'));
  // Displaytyp-Anzeige
  caption:=cDispType+' Display-Testprogramm';
  // Cursor-Positionen
  SpinEdit1.MaxValue:=cTxtResX;
  SpinEdit2.MaxValue:=cTxtResY;
  SpinEdit1.value:=1;
  SpinEdit2.value:=1;
  // Menü updaten
  for z:=0 to Displaytyp1.count-1 do
    Displaytyp1.items[z].checked:=DispTypeList[Displaytyp1.items[z].tag]=cDispType;
  myini.free;
end;

procedure TForm1.DisplaytypClick;
begin
  // Ein anderer Displaytyp wurde ausgewählt
  SelectDisplay(DispTypeList[(sender as TMenuItem).tag]);
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
  DispTypeList.free;
end;

end.
