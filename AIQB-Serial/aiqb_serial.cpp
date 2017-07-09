#include "Arduino.h"
#include "aiqb_serial.h"

AIQB_Serial::AIQB_Serial (unsigned int Address, byte SwitchPin) {
  byte c;
  
  this->Address = Address;
  this->SwitchPin = SwitchPin;
  this->State = false;
  pinMode (this->SwitchPin, OUTPUT);
  digitalWrite (this->SwitchPin, LOW);

  for (c = 0; c < AIQB_Registry; c++)
    this->Sensors[c] = AIQB_Device_Read | AIQB_Device_None;
  this->RegisterPointer = 0;
}

byte AIQB_Serial::rx () {
  unsigned long now, then;
  byte c, v[4], x = 0, m = 0, s = 0;
  unsigned int a, r;

  Serial.flush ();
  
  this->State = false;
  digitalWrite (this->SwitchPin, LOW);

  then = millis ();

  while (this->State == false) {
    while (Serial.available () > 0) {
    c = Serial.read ();
    
    if (s == 0 && c == 'R') { s = 1; continue; }
    if (s == 0 && c == 'W') { s = 6; continue; }
    if (s == 0 && c == 'L') { s = 15; continue; }
    if (s == 0 && c == 'D') { s = 17; continue; }

    if (s ==  1) { m = 0; a = c << 8; s = 2; continue; }
    if (s ==  2) { a |= c; s = this->Address == a ? 3 : 0; continue; }
    if (s ==  3) { r = c << 8; m ^= c; s = 4; continue; }
    if (s ==  4) { r |= c; m ^= c; s = this->Address == a ? 5 : 0; continue; }
    if (s ==  5) { if (m == c) { this->Register = r; s = 252; } else s = 0; }
    
    if (s ==  6) { m = 0; a = c << 8; s = 7; continue; }
    if (s ==  7) { a |= c; s = this->Address == a ? 8 : 0; continue; }
    if (s ==  8) { r = c << 8; m ^= c; s = 9; continue; }
    if (s ==  9) { r |= c; m ^= c; s = this->Address == a ? 10 : 0; continue; }
    if (s > 9 && s < 14) { v[s - 10] = c; m ^= c; s++; continue; }
    if (s == 14) { if (m == c) { this->Register = r; memcpy (this->Data.c, v, 4); s = 253; } else s = 0; }

    if (s == 15) { m = 0; a = c << 8; s = 16; continue; }
    if (s == 16) { a |= c; s = this->Address == a ? 254 : 0; }

    if (s == 17) { m = 0; x = 0; a = c << 8; s = 18; continue; }
    if (s == 18) { a |= c; s = this->Address == a ? 19 : 0; continue; }
    if (s == 19) { r = c << 8; m ^= c; s = 20; continue; }
    if (s == 20) { r |= c; m ^= c; s = 21; continue; }
    if (s == 21) { x = c; s = 22; continue; }
    if (s > 21 && s < (x + 22)) {
      if (x == 4) { v[s - 22] = c; m ^= c; s++; continue; }
      if (x == 2 * AIQB_Registry) { this->Sensors[(s - 22) / 2] = s%2 ? c << 8 : (this->Sensors[(s - 22) / 2] | c); m ^= c; s++; continue; }
      s = 0;
      x = 0;
      continue;
    }
    if (s == x + 22) { if (m == c) { if (x == 4) memcpy (this->Data.c, v, 4); s = 255; } else s = 0; }

    if (s > 250) break;
    }
  if (s > 250) break;
  
  now = millis ();
  if (now > then) { if (now - then > AIQB_Timeout) break; } else { then = millis (); }
  }

  return s > 250 ? s - 251 : 0;
}

void AIQB_Serial::_tx () {
  this->State = true;
  Serial.flush ();  
  digitalWrite (this->SwitchPin, HIGH);
  delay (AIQB_Delay);
  }

void AIQB_Serial::tx (unsigned int a, unsigned int r) {
  byte c, m = 0;
  
  this->_tx ();
  Serial.print ('D');
  Serial.print ((char) (a >> 8));
  Serial.print ((char) a);
  Serial.print ((char) (r >> 8));
  Serial.print ((char) r);
  Serial.print ((char) 4);

  m = (byte) ((r >> 8) | r);

  for (c = 0; c < 4; c++) { m ^= this->Data.c[c]; Serial.print ((char) this->Data.c[c]); }
  Serial.print ((char) m);
}
void AIQB_Serial::tx (unsigned int a, unsigned int r, char d[]) {
  memcpy (this->Data.c, d, 4);
  this->tx (a, r);
}
void AIQB_Serial::tx (unsigned int a, unsigned int r, float d) {
  this->Data.f = d;
  this->tx (a, r);
}
void AIQB_Serial::tx (unsigned int a, unsigned int r, int d[]) {
  memcpy (this->Data.i, d, 2);
  this->tx (a, r);
}
void AIQB_Serial::tx (unsigned int a) {
  byte c, m = 0;

  this->_tx ();
  Serial.print ('D');
  Serial.print ((char) (2 * AIQB_Registry));
  Serial.print ((char) (a >> 8));
  Serial.print ((char) a);
  Serial.print ((char) 0);
  Serial.print ((char) 0);
  Serial.print ((char) (2 * AIQB_Registry));
  for (c = 0; c < AIQB_Registry; c++) {
    Serial.print ((char) (this->Sensors[c] >> 8));
    Serial.print ((char) this->Sensors[c]);
    m ^= (byte) ((this->Sensors[c] >> 8) ^ this->Sensors[c]);
    }
  Serial.print ((char) m);
}

void AIQB_Serial::tx (char c, unsigned int a, unsigned int r, union _data_t d) {
  byte n, m = 0;

  this->_tx ();

  Serial.print (c);
  Serial.print ((char) (a >> 8));
  Serial.print ((char) a);
  Serial.print ((char) (r >> 8));
  Serial.print ((char) r);
  m = (byte) ((r >> 8) ^ r);
  for (n = 0; n < 4; n++) {
    Serial.print ((char) d.c[n]);
    m ^= d.c[n];
  }
  Serial.print ((char) m);
}
void AIQB_Serial::tx (char c, unsigned int a, unsigned int r) {
  this->_tx ();

  Serial.print (c);
  Serial.print ((char) (a >> 8));
  Serial.print ((char) a);
  Serial.print ((char) (r >> 8));
  Serial.print ((char) r);
  Serial.print ((char) ((r >> 8) ^ r));
}
void AIQB_Serial::tx (char c, unsigned int a) {
  this->_tx ();

  Serial.print (c);
  Serial.print ((char) (a >> 8));
  Serial.print ((char) a);
}

void AIQB_Serial::reg (unsigned int * r) {
  * r = this->Register;
}

void AIQB_Serial::radd (unsigned int r) {
  if (this->RegisterPointer + 1 < AIQB_Registry) {
    this->Sensors[++this->RegisterPointer] = r;
  }
}

void AIQB_Serial::rrst () {
  this->RegisterPointer = 0;
}

void AIQB_Serial::rnxt () {
  if (this->RegisterPointer + 1 < AIQB_Registry) this->RegisterPointer ++;
}

void AIQB_Serial::rprv () {
  if (this->RegisterPointer > 0) this->RegisterPointer --;
}

void AIQB_Serial::val (byte * d) {
  memcpy (d, this->Data.c, 4);
}

void AIQB_Serial::val (float * d) {
  * d = this->Data.f;
}

void AIQB_Serial::val (int * d) {
  memcpy (d, this->Data.i, 2);
}
