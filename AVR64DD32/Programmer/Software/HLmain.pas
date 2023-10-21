// =============================================================================
// HEX Loader main unit
// =============================================================================
{
 * This PC software is a part of HBus HEX Loader. It works in conjunction with
 * HEXloader.ino sketch running on Arduino UNO.
 *
 * (c) 2023 Alex Kouznetsov,  https://github.com/akouz/hbus
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
}

unit HLmain;

{$mode objfpc}{$H+}
//##############################################################################
interface
//##############################################################################

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, Menus, ExtCtrls,
  StdCtrls, ComCtrls, CPort, IniFiles, Clipbrd, Registry, HLrxtxU;

type

  { TForm1 }

  TForm1 = class(TForm)
    BtnSendBuf: TButton;
    BtnSendHexChunk: TButton;
    BtnSendDescr: TButton;
    BtnRqRev: TButton;
    BtnQuery: TButton;
    BtnSetLow: TButton;
    BtnSetHi: TButton;
    BtnShowRxStr: TButton;
    BtnFile: TButton;
    ComBox: TComboBox;
    LblProgConnected: TLabel;
    LblEEconnected: TLabel;
    LblComStatus: TLabel;
    LblAddr: TLabel;
    LblComPort: TLabel;
    LblFn: TLabel;
    LB: TListBox;
    OpenDialog: TOpenDialog;
    Panel1: TPanel;
    SaveDialog: TSaveDialog;
    TabCtrl: TTabControl;
    Timer1: TTimer;
    Timer2: TTimer;
    procedure BtnCRCClick(Sender: TObject);
    procedure BtnFileClick(Sender: TObject);
    procedure BtnQueryClick(Sender: TObject);
    procedure BtnRqRevClick(Sender: TObject);
    procedure BtnSendBufClick(Sender: TObject);
    function BtnSendDescrClick(Sender: TObject; echo: char): integer;
    function BtnSendHexChunkClick(Sender: TObject; echo: char) : integer;
    procedure BtnSetHiClick(Sender: TObject);
    procedure BtnSetLowClick(Sender: TObject);
    procedure BtnShowRxStrClick(Sender: TObject);
    procedure ComBoxChange(Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
    procedure FormCreate(Sender: TObject);
    procedure LBDblClick(Sender: TObject);
    procedure LBKeyPress(Sender: TObject; var Key: char);
    procedure TabCtrlChange(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure Timer2Timer(Sender: TObject);
  private
    codebuf : array [0..$FFFF] of byte;     // 64K bytes - boot+appcode
    addr_min, addr_max, wraddr, prev_wraddr, codecnt : integer;
    bootbuf : array [0..$1000] of byte;     // boot only
    boot_min, boot_max, bootcnt : integer;
    buf_crc : word;
    HF : TStringList;
    Sent : TStringList;
    rxtx : THLxtx;
    state, rpt_cnt: integer;
    boot_size : integer;
    cmd_sent : char;
    Prog_connected : boolean;       // Arduino programmer connected to PC
    EE_connected : boolean;         // target board HBnode with EEPROM connected to the programmer
    since_last_rx : integer;
    state_log, rply_log : string;
    procedure ListComPorts;
    procedure ClrBuf;
    function CodeBufCRC(len : integer) : word;
    function ParseLB : boolean;
    function ParseString(ss : string) : boolean;
    function RestoreCheckSum(ss : string) : string;
    procedure OpenFile(fn : string);
    procedure ShowData;
    procedure ShowBootData;
    procedure ShowRx;
    procedure ShowTx;
    procedure ReplyRev(echo : char);
    procedure ReplyQuery(echo : char);
    procedure ReplyLo(echo : char);
    procedure ReplyHi(echo : char);
    procedure ReplyHexChunk(echo : char);
    procedure ReplyDescr(echo : char);
    procedure ShowRplyLog;
  public
    FileName, iniFileName, ComPortNo : string;
    function HexStrToBuf (ss : string; var buf : array of byte) : integer;
  end;

var
  Form1: TForm1;
  Clpbrd: TClipboard;
  CBuf : array [0..$100] of byte;

//##############################################################################
implementation
//##############################################################################

{$R *.lfm}

{ TForm1 }
//================================================
// Create
//================================================
procedure TForm1.FormCreate(Sender: TObject);
var ini : TIniFile;
begin
  LB.Clear;
  ClrBuf;
  Clpbrd:= TClipboard.Create;
  Clpbrd.AddFormat(CF_TEXT, CBuf, $100);
  Prog_connected := false;
  ini := TIniFile.Create('HEXloader.ini');
  ComPortNo := AnsiUpperCase(trim(ini.ReadString('ComPort','number','COM3')));
  iniFileName := trim(ini.ReadString('File','name',''));
  boot_size := ini.ReadInteger('Boot','size', $400);
  if boot_size > $1000 then
    boot_size := $1000; // 4K max
  ini.Free;
  FileName := '';
  LblComPort.Caption:='Port '+ComPortNo;
  LblFn.Caption := 'no file';
  ListComPorts;
  rxtx := THLxtx.Create(ComPortNo);
  HF := TStringList.Create;
  Sent := TStringList.Create;
  if rxtx.PortOk then begin
      LblComStatus.Caption:='COM port OK';
      BtnRqRevClick(Sender);
  end;
  EE_connected := false;
  TabCtrl.TabIndex:=3;
end;

//================================================
// Clear
//================================================
procedure TForm1.LBDblClick(Sender: TObject);
begin
  if TabCtrl.TabIndex = 3 then begin
    LB.Clear;
    rxtx.Clear;
    rxtx.newstr := true;
  end;
end;

//================================================
// On keypress
//================================================
procedure TForm1.LBKeyPress(Sender: TObject; var Key: char);
var i : integer;
    s : string;
begin
  for i:=0 to LB.Count-1 do begin
    if LB.Selected[i] then begin
      // -------------------
      // Copy - Ctrl+C
      // -------------------
      if Key = char(3) then begin
        s := LB.Items.Strings[i];
        Clpbrd.AsText := s;
      end;
      if TabCtrl.TabIndex = 0 then begin
        // -------------------
        // Paste - Ctrl+V or Ctrl+H
        // -------------------
        if (Key = char(22)) or (Key = char(6))  then begin
          s :=  Clpbrd.AsText;
          if (s <> '') and (s[1] = ':') then begin
            s := RestoreCheckSum(Clpbrd.AsText);
            LB.Items.Strings[i] := s;
            ParseLB;
            s := 'Address range [0x'+IntToHex(addr_min,4)+'..0x'+ IntToHex(addr_max,4)+'], ';
            s := s + IntToStr(codecnt) + ' bytes loaded';
            s := s + ', crc = 0x'+ IntToHex(buf_crc,4);
            LblAddr.Caption := s;
          end;
        end;
        // -------------------
        // Delete - Ctrl+X
        // -------------------
        if Key = char(24) then begin
          LB.DeleteSelected;
        end;
        // -------------------
        // Save - Ctrl+S
        // -------------------
        if Key = char(19) then begin
          SaveDialog.FileName := FileName;
          if SaveDialog.Execute then begin
            FileName := SaveDialog.FileName;
            LB.Items.SaveToFile(FileName);
            OpenFile(FileName);
          end;
        end;
      end;
      break;
    end;
  end;
end;

//================================================
// Close
//================================================
procedure TForm1.FormClose(Sender: TObject; var CloseAction: TCloseAction);
var ini : TIniFile;
begin
  ini := TIniFile.Create('HEXloader.ini');
  if (ComPortNo <> 'COM1') then
    ini.WriteString('ComPort','number', ComPortNo);
  if FileName <> '' then
    ini.WriteString('File','name',FileName);
  Clpbrd.Free;
  rxtx.Free;
  ini.Free;
  HF.Free;
  Sent.Free;
end;

//================================================
// Change com port number
//================================================
procedure TForm1.ComBoxChange(Sender: TObject);
begin
  ComPortNo := ComBox.Items[ComBox.ItemIndex];
  rxtx.ComPort.Close;
  rxtx.Clear;
  if TabCtrl.TabIndex = 3 then
    LB.Clear;
  Prog_connected := false;
  LblProgConnected.Caption := 'Programmer not detected';
  EE_connected := false;
  LblEEconnected.Caption:='EEPROM disconnected';
  rxtx.ComPort.port := ComPortNo;
  try
    rxtx.ComPort.Open;
    LblComStatus.Caption:='COM port OK';
    BtnRqRevClick(Sender);
  except
    if rxtx.ComPort.Connected then
      rxtx.ComPort.Connected := false;
    rxtx.PortOk := false;
    LblComStatus.Caption:='Cannot open COM port';
  end;
end;

//================================================
// Send codebuf
//================================================
procedure TForm1.BtnSendBufClick(Sender: TObject);
begin
  TabCtrl.TabIndex:=3;
  if not prog_connected then
    ShowMessage('Programmer not connected to COM port')
  else begin
    if not EE_connected then
       ShowMessage('Target board not connected to programmer')
    else begin
      if codecnt <= 0 then
        ShowMessage('No HEX file loaded, nothing to send to programmer')
      else begin
        Sent.Clear;
        wraddr := addr_min;
        prev_wraddr := wraddr;
        state := 1; // reset state machine
        state_log := '';
        rply_log := '';
        rxtx.RxStr := '';
        rpt_cnt := 0;
        rxtx.newstr := true;
        rxtx.Clear;
        if TabCtrl.TabIndex = 3 then
          LB.Clear;
      end;
    end;
  end;
end;

//================================================
// Request revision
//================================================
procedure TForm1.BtnRqRevClick(Sender: TObject);
var s : string;
begin
  s := 'R' + char(0);
  cmd_sent := 'R';
  if not rxtx.Tx(s) then
    Application.Terminate;
end;

//================================================
// Query EEPROM
//================================================
procedure TForm1.BtnQueryClick(Sender: TObject);
var s : string;
begin
  s := 'Q' + char(0);
  cmd_sent := 'Q';
  if not rxtx.Tx(s) then
    Application.Terminate;
end;

//================================================
// Send descriptor for bootloader
//================================================
function TForm1.BtnSendDescrClick(Sender: TObject; echo: char) : integer;
var s : string;
    len : integer;
    cs : byte;
    crc : word;
begin
  if echo = '.' then begin
    rpt_cnt := 0;
    result := 2;    // success
    exit;
  end else begin
    inc(rpt_cnt);
    if rpt_cnt > 3 then begin // abort
      result := 99;
      exit;
    end;
  end;
  s := '!04001001';    // header
  len := addr_max - addr_min + 1;
  s := s + IntToHex(len, 4);
  cs := 4+$10+1+byte(len shr 8) + byte(len); // code length
//  crc := CodeBufCRC(len);
  s := s + IntToHex(buf_crc, 4);
  cs := cs + byte(buf_crc shr 8) + byte(buf_crc);
  cs := (cs xor $FF) +1;
  s := s + IntToHex(cs,2);
  Sent.Add(s);
  cmd_sent := '!';
  if not rxtx.Tx(s+char(0)) then
    Application.Terminate;
  result := 1;
end;

//================================================
// Send chunk of data
//================================================
function TForm1.BtnSendHexChunkClick(Sender: TObject; echo: char): integer;
var s : string;
    i,  cnt : integer;
    val, cs : byte;
begin
  result := 0;
  if wraddr <= addr_max then begin
    if echo = '.' then begin
      prev_wraddr := wraddr; // next
      rpt_cnt := 0;
    end else begin
      wraddr := prev_wraddr; // repeat previous
      inc(rpt_cnt);
      if rpt_cnt > 3 then begin // abort
        result := 99;
        exit;
      end;
    end;
    s := IntToHex(wraddr,4)+'00';
    cnt := 0;
    cs := byte(wraddr shr 8) + byte(wraddr);
    for i:=0 to $0F do begin
      val := codebuf[wraddr];
      cs := cs+val;
      s := s+IntToHex(val,2);
      inc(cnt);
      inc(wraddr);
      if ((wraddr and $0F)=0) or (wraddr > addr_max) then begin
        cs := cs + cnt;
        cs := (cs xor $FF) + 1;
        s:=':'+IntToHex(cnt,2) + s + IntToHex(cs,2);
        Sent.Add(s);
        s := s + char(0);
        cmd_sent := ':';
        if not rxtx.Tx(s) then
          Application.Terminate;
        break;
      end;
    end;
  end else
    result := 1; // completed
end;

//================================================
// Set passive hi reset
//================================================
procedure TForm1.BtnSetHiClick(Sender: TObject);
var s : string;
begin
  s := 'H' + char(0);
  cmd_sent := 'H';
  if not rxtx.Tx(s) then
    Application.Terminate;
end;

//================================================
// Set active low reset
//================================================
procedure TForm1.BtnSetLowClick(Sender: TObject);
var s : string;
begin
  s := 'L' + char(0);
  cmd_sent := 'L';
  if not rxtx.Tx(s) then
    Application.Terminate;
end;

//================================================
// Show replies from programmer
//================================================
procedure TForm1.BtnShowRxStrClick(Sender: TObject);
begin
  LB.Clear;
  ShowRplyLog;
end;

//================================================
// Extract first decimal number from a string
//================================================
function extract_first_number(ss : string) : string;
var i : integer;
    c : char;
begin
  result := '';
  for i:=1 to length(ss)+1 do begin
    c := ss[i];
    if c in ['0'..'9'] then
      result := result + c
    else if result <> '' then  // number extracted
      break;
  end;
end;

//================================================
// Find all existing at the moment COM ports
//================================================
procedure TForm1.ListComPorts;
var
  i: integer;
  reg: TRegistry;
  ts: TStringList;
  pno: string;
  match : boolean;
begin
  ComBox.Clear;
  match := false;
  reg := TRegistry.Create(KEY_READ);
  reg.RootKey := HKEY_LOCAL_MACHINE;
  reg.OpenKey('hardware\devicemap\serialcomm', False);
  ts := TStringList.Create;
  // ------------------------------
  // read all COM ports from registry
  // ------------------------------
  reg.GetValueNames(ts);
  // ------------------------------
  // all serial ports
  // ------------------------------
  for i:=0 to ts.Count-1 do  begin
    pno := reg.ReadString(ts.Strings[i]);
    ComBox.AddItem(pno, Nil);
  end; // for
  ComBox.Sorted:=true;
  for i:=0 to ComBox.Items.Count-1 do  begin
    if (ComBox.Items[i] = ComPortNo) then begin
      match := true;
      ComBox.ItemIndex := i;
    end;
  end;
  if (not match) and (ts.Count > 0) then
    ComBox.ItemIndex := 0;
  ComPortNo := ComBox.Items[ComBox.ItemIndex];
  ts.Free;
  reg.Free;
end;

//================================================
// Tab change
//================================================
procedure TForm1.TabCtrlChange(Sender: TObject);
begin
  LB.ShowHint:=false;
  case TabCtrl.TabIndex of
  0: begin
      if (FileName <> '') then
        LB.Items.LoadFromFile(FileName)
      else
        LB.Clear;
      LB.ShowHint:=true;
    end;
  1: ShowData;
  2: ShowBootData;
  3: ShowRx;
  4: ShowTx;
  end;
end;

//================================================
// Clear buffer
//================================================
procedure TForm1.ClrBuf;
var i : integer;
begin
  for i:=0 to $FFFF do begin
    codebuf[i] := $FF;
  end;
  codecnt := 0;
  addr_min := $FFFF;
  addr_max := 0;
  for i:=0 to $3FF do begin
    bootbuf[i] := $FF;
  end;
  bootcnt := 0;
  boot_min := $FFFF;
  boot_max := 0;
end;

// ===========================================
// add CRC to transmit buffer
// ===========================================
{
 * Name  : CRC-16 CCITT
 * Poly  : 0x1021    x^16 + x^12 + x^5 + 1
 * Init  : 0xFFFF
 * Revert: false
 * XorOut: 0x0000
 * Check : 0x3B0A ("123456789" hex)
}
function TForm1.CodeBufCRC(len: integer): word;
var
    crc : word;
    i, j : integer;
    val : word;
begin
  crc := $FFFF;
  for i:=boot_size to (boot_size + len) do begin
    val := codebuf[i];
    crc := crc xor (val shl 8);
    for j:=0 to 7 do begin
       if (crc and $8000) = 0 then
          crc := crc shl 1
       else
          crc := (crc shl 1) xor $1021;
    end;
  end;
  result := crc;
end;

//================================================
// Convert hex string into bytes
//================================================
function TForm1.HexStrToBuf(ss : string; var buf : array of byte) : integer;
var i:integer;
    c : char;
    s : string;
begin
  result := 0;
  ss := AnsiUpperCase(ss);
  s := '';
  for i:=1 to length(ss) do begin
    c := ss[i];
    if (c in ['0'..'9']) or (c in ['A'..'F']) then begin
      s := s+c;
      if length(s) = 2 then begin
        buf[result] := StrToInt('$'+s);
        inc(result);
        s := '';
      end;
    end;
  end;
end;

//================================================
// Parse string
//================================================
function TForm1.ParseString(ss: string): boolean;
var i, len, cnt, rectype, chsum : byte;
    start, addr : integer;
    buf : array [0..$FF] of byte;
begin
  result := false;
  if (length(ss)<10) or (length(ss)>$200) then
    exit;
  if ss[1] = ':' then begin
    len := HexStrToBuf(ss, buf);
    chsum := 0;
    for i:=0 to len-1 do
      chsum := chsum+buf[i];
    if chsum = 0 then begin  // must be 0
      cnt := buf[0];
      if (len = cnt+5) and (cnt>0) then begin
        start := $100*buf[1] + buf[2];
        rectype := buf[3];
        if ((rectype=0) or (rectype=1)) and (len>0) then begin
          result := true;
          for i:=0 to cnt-1 do begin
            addr := start + i;
            // -----------------------
            // application code
            // -----------------------
            if (addr >= boot_size) then begin
              if addr < addr_min then
                addr_min := addr;
              if addr > addr_max then
                addr_max := addr;
              if (addr_max > $FFFF) then
                addr_max := $FFFF
              else begin
                codebuf[addr] := buf[4 + i];
                inc(codecnt);
              end;
            // -----------------------
            // bootloader code
            // -----------------------
            end else begin
              if (addr >= 0) and (addr < boot_min) then
                boot_min := addr;
              if (addr > boot_max) then
                boot_max := addr;
              bootbuf[addr] := buf[4 + i];
              inc(bootcnt);
            end;
          end;
        end; // rectype
        result := true; // other rectypes ignored
      end; // len matches cnt
    end; // chsum
  end; // :
end;

//================================================
// Assuming hex string was edited, restore its checksum
//================================================
function TForm1.RestoreCheckSum(ss: string): string;
var s : string;
    i, len : integer;
    buf : array [0..$FF] of byte;
    cs, val : byte;
begin
  result := ss;
  if length(ss) > 5 then begin
    if ss[1] = ':' then begin
      len := HexStrToBuf(ss, buf);
      cs := 0;
      for i:=0 to len-2 do
        cs := cs + buf[i];
      cs := (cs xor $FF) + 1;
      result := copy(ss, 1, length(ss)-2);
      result := result + IntToHex(cs,2);
    end;
  end;
end;

//================================================
// Display data
//================================================
procedure TForm1.ShowData;
var addr : integer;
    b : byte;
    s : string;
begin
  LB.Clear;
  s := '';
  if (addr_min > addr_max) then
    exit;
  for addr := (addr_min and $FFF0) to addr_max do begin
    if (addr and $F) = 0 then
      s := IntToHex(addr,4) + ': ';
    b := codebuf[addr];
    if addr < addr_min then
      s := s + '   '
    else
      s := s + IntToHex(b,2) + ' ';
    if (addr and $F) = 7 then
      s := s + ' ';
    if (addr and $F) = $F then
      LB.AddItem(s, Nil);
  end;
  if s <> '' then
    LB.AddItem(s, Nil);
end;

//================================================
// Display data in bootloader area
//================================================
procedure TForm1.ShowBootData;
var addr : integer;
    b : byte;
    s : string;
begin
  LB.Clear;
  s := '';
  if (boot_min > boot_max) then
    exit;
  for addr := (boot_min and $FFF0) to boot_max do begin
    if (addr and $F) = 0 then
      s := IntToHex(addr,4) + ': ';
    b := bootbuf[addr];
    if addr < boot_min then
      s := s + '   '
    else
      s := s + IntToHex(b,2) + ' ';
    if (addr and $F) = 7 then
      s := s + ' ';
    if (addr and $F) = $F then
      LB.AddItem(s, Nil);
  end;
  if s <> '' then
    LB.AddItem(s, Nil);
end;

//================================================
// Display received
//================================================
procedure TForm1.ShowRx;
begin
  LB.Clear;
  ShowRplyLog;
end;

//================================================
// Display sent
//================================================
procedure TForm1.ShowTx;
var i:integer;
begin
  LB.Clear;
  for i:=0 to Sent.Count-1 do
    LB.AddItem(Sent.Strings[i], Nil);
end;

//================================================
// Parse ListBox
//================================================
function TForm1.ParseLB: boolean;
var i, len : integer;
    s : string;
begin
  result := true;
  for i:=0 to LB.Items.Count-1 do begin
    s := LB.Items.Strings[i];
    result := ParseString(s);
    if result = false then
      break;
  end;
  len := addr_max - addr_min;
  buf_crc := CodeBufCRC(len);
end;

//================================================
// Open file
//================================================
procedure TForm1.OpenFile(fn: string);
var s : string;
begin
  LblFn.Caption := 'File '+ExtractFileName(FileName);
  ClrBuf;
  LB.Items.LoadFromFile(FileName);
  ParseLB;
  s := 'Address range [0x'+IntToHex(addr_min,4)+'..0x'+ IntToHex(addr_max,4)+'], ';
  s := s + IntToStr(codecnt) + ' bytes loaded';
  s := s + ', crc = 0x'+ IntToHex(buf_crc,4);
  LblAddr.Caption := s;
  BtnSendBuf.Enabled:=true;
end;


//================================================
// Open file button
//================================================
procedure TForm1.BtnFileClick(Sender: TObject);
var s : string;
begin
  if FileName = '' then begin
    s := ExtractFileDir(iniFileName);
    OpenDialog.InitialDir := s;
  end;
  if OpenDialog.Execute then begin
    TabCtrl.TabIndex := 0;
    LB.ShowHint:=true;
    FileName := OpenDialog.FileName;
    OpenFile(FileName);
  end;
end;

//================================================
// Calculate CRC
//================================================
procedure TForm1.BtnCRCClick(Sender: TObject);
var len : integer;
begin
  len := addr_max - addr_min;
  buf_crc := CodeBufCRC(len);
end;

//================================================
// Reply on 'R' command
//================================================
procedure TForm1.ReplyRev(echo: char);
begin
  Prog_connected := (echo = 'R');
  if Prog_connected then begin
    LblProgConnected.Caption:= 'Programmer connected, sketch '+AnsiLowerCase(rxtx.LastStr);
    if (state > 0) then // if it is a batch
      inc(state);
  end else begin
    LblProgConnected.Caption:= 'Programmer not detected';
    cmd_sent := ' ';
    if (state > 0) then begin
      state := 0;
      ShowMessage('Programmer not connected, batch aborted');
    end;
  end;
end;

//================================================
// Reply on 'Q' command
//================================================
procedure TForm1.ReplyQuery(echo: char);
begin
  EE_connected := (echo = '+');
  if  EE_connected then begin
    LblEEconnected.Caption:='Target board connected';
    EE_connected := true;
    if (state > 0) then // if it is a batch
      inc(state);
  end else begin
    LblEEconnected.Caption:='Target board disconnected';
    EE_connected := false;
    if (state > 0) then begin
      state := 0;
      ShowMessage('Target board not connected, batch aborted');
     end;
  end;
  cmd_sent := ' ';
end;

//================================================
// Reply on 'L' command
//================================================
procedure TForm1.ReplyLo(echo: char);
begin
  if (echo = 'L') then begin
    if (state > 0) then // if it is a batch
      inc(state);
  end;
  cmd_sent := ' ';
end;

//================================================
// Reply on 'H' command
//================================================
procedure TForm1.ReplyHi(echo: char);
begin
  if (echo = 'H') then begin
    if (state > 0) then // if it is a batch
      inc(state);
  end;
  cmd_sent := ' ';
end;

//================================================
// Reply on ':' command
//================================================
procedure TForm1.ReplyHexChunk(echo: char);
var res : integer;
begin
  if echo = '-' then begin
    EE_connected := false;
    LblEEconnected.Caption:='Target board disconnected';
    if (state > 0) then begin
      state := 0;
      ShowMessage('Target board not connected, batch aborted');
     end;
    cmd_sent := ' ';
  end else begin
    res := BtnSendHexChunkClick(Nil, echo); // repeat until all codebuf sent
    if res > 0 then begin
      if res > 2 then begin // abort
        state := 0;
        ShowMessage('Unable to send, batch aborted');
      end else if state > 0 then
        inc(state);
      cmd_sent := ' ';
    end;
  end;
end;

//================================================
// Reply on '!' command
//================================================
procedure TForm1.ReplyDescr(echo: char);
begin
  if echo = '-' then begin
    EE_connected := false;
    LblEEconnected.Caption:='Target board disconnected';
    if (state > 0) then begin
      state := 0;
      ShowMessage('Target board not connected, batch aborted');
     end;
  end else if echo = 'C' then begin
     state := 0;
     ShowMessage('CRC mismatch, batch aborted');
  end else if echo = '!' then begin // success
    inc(state);
  end else begin // 'x'
    state := 0;
    ShowMessage('Error, batch aborted');
  end;
  cmd_sent := ' ';
end;

//================================================
// Show reply log
//================================================
procedure TForm1.ShowRplyLog;
var s : string;
    pos, len : integer;
begin
  LB.Clear;
  pos := 0;
  len := length(rply_log);
  while pos < len do begin
    if (len - pos) <= 80 then begin
      s := Copy(rply_log, pos, len-pos );
      pos := len;
    end else begin
      s := Copy(rply_log, pos, 80);
      pos := pos + 80;
    end;
    LB.AddItem(s, Nil);
  end;
end;

//================================================
// On timer 2 ms process replies from the programmer
//================================================
procedure TForm1.Timer1Timer(Sender: TObject);
var echo : char;
    was : string;
begin
  rxtx.Tick2ms;
  echo := rxtx.LastReply;
  if (ord(echo) > $20) then begin
    was := rply_log;
    rply_log := rply_log + echo;
    since_last_rx := 0;
    case cmd_sent of
      //----------------------------
      // revision
      //----------------------------
      'R': ReplyRev(echo);
      //----------------------------
      // query EEPROM
      //----------------------------
      'Q': ReplyQuery(echo);
      //----------------------------
      // reset hi
      //----------------------------
      'H': ReplyHi(echo);
      //----------------------------
      // reset low
      //----------------------------
      'L': ReplyLo(echo);
      //----------------------------
      // send hex chunk
      //----------------------------
      ':': ReplyHexChunk(echo);
      //----------------------------
      // send hex chunk
      //----------------------------
      '!': ReplyDescr(echo);
    else
      rply_log := was;
    end; // case command sent
  end;
end;

//================================================
// On timer 50 ms process state machine
//================================================
procedure TForm1.Timer2Timer(Sender: TObject);
begin
//  if (rxtx.redraw) and (TabCtrl.TabIndex = 3) then
//    ShowRx;
  if (state > 0) then begin  // state machine in progress
    ShowRplyLog;
    if (Prog_connected) and (EE_connected) then begin
      if (state > 0) then
        state_log := state_log + ' '+ IntToStr(state);
      case state of
        1: BtnRqRevClick(Sender);
        2: BtnQueryClick(Sender);
        3: BtnSetLowClick(Sender);
        4: begin
            BtnSendHexChunkClick(Sender, '.'); // first chunk
            inc(state);
           end;
//      5:    // send other chunks
        6: BtnSendDescrClick(Sender, '?');
        7: BtnSetHiClick(Sender);
        8: begin
            state := 0;
            rply_log := rply_log + ' ';
            ShowRplyLog;
            Application.ProcessMessages;
            ShowMessage('Success, HEX file sent to target');
           end;
      end;
    end;
  end;
  inc(since_last_rx);
  if since_last_rx > 5 then begin
    if not prog_connected then
      BtnRqRevClick(Sender)
    else begin
      if not EE_connected then
        BtnQueryClick(Sender);
    end;
  end;
end;

end.
