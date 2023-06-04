// =============================================================================
// HEX Loader serial port
// =============================================================================
{
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
unit HLrxtxU;

{$mode objfpc}{$H+}

//##############################################################################
interface
//##############################################################################

uses
  Classes, SysUtils, Dialogs, CPort;

type

  // =====================================  
  { THLxtx }
  // =====================================  

  THLxtx = class(TStringList)
  private
    last_echo : char;
    reply : string;                     // when a reply received
  public
    ComPort: TComPort;
    PortOk : boolean;
    redraw : boolean;
    newstr : boolean;                   // new string
    LastStr : string;
    RxStr : string;
    function LastReply : char;
    function Tx(msg: string) : boolean; // transmit if not busy
    procedure Tick2ms;                  // process it every 2 ms
    constructor Create(port : string);
    destructor Destroy; override;
  end;


//##############################################################################
implementation
//##############################################################################

{ THLxtx }
// =====================================
// Transmit
// =====================================  
function THLxtx.Tx(msg: string): boolean;
begin
  result := false;
  if (ComPort.Connected and PortOk) then begin
    try
      if (msg <> '') then begin
        ComPort.WriteStr(msg);
        result := true;
      end;
    except
      if ComPort.Connected then
        ComPort.Connected:=false;
      PortOk := false;
    end;
  end else
    ShowMessage('COM port not connected');
end;

// =====================================  
// Time tick 2 ms, count time-out and re-send
// =====================================  
procedure THLxtx.Tick2ms;
var len : integer;
    ss : string;
    last : char;
    ok : boolean;  // debug
begin
  ss := '';
  if ComPort.Connected and PortOk then begin
    try
      len := ComPort.InputCount;
      if len > 0 then begin
         len := ComPort.ReadStr(ss, len);
         RxStr := RxStr + Trim(ss);
         ComPort.ClearBuffer(true, false);
         last := ss[length(ss)];
         reply := Trim(reply+ss);
         if (ord(last) = 0) and (length(reply) > 0) then begin
           ok := true;
           last_echo := reply[1];
           if last_echo = 'H' then begin
             redraw := true;
           end;
           LastStr := reply;
           redraw := true;
           reply := '';
          end else begin
           ok := false;
          end;
        end;
    except
      if ComPort.Connected then
         ComPort.Connected:=false;
      PortOk := false;
    end;
  end;
end;

// =====================================
// Last received reply
// =====================================
function THLxtx.LastReply: char;
begin
  Result := last_echo;
  last_echo := char(0);
end;


// =====================================
// Create
// =====================================
constructor THLxtx.Create(port : string);
begin
  ComPort:= TComPort.Create(nil);
  ComPort.BaudRate := br115200;
  ComPort.DataBits := dbEight;
  ComPort.StopBits := sbOneStopBit;
  ComPort.Port := port;
  reply := '';
  newstr := true;
  RxStr := '';
  try
    PortOk := true;
    ComPort.Open;
  except
    if ComPort.Connected then
      ComPort.Connected := false;
    PortOk := false;
  end;
end;

 // =====================================  
 // Destroy
 // =====================================  
destructor THLxtx.Destroy;
begin
  ComPort.Close;
  ComPort.Free;
  inherited Destroy;
end;

end.
