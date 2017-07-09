#ifndef AIQB_Serial_h
#define AIQB_Serial_h

#define AIQB_Registry   16

#define AIQB_Return_Read  1
#define AIQB_Return_Write 2
#define AIQB_Return_List  3
#define AIQB_Return_Data  4

#define AIQB_Device_Read      0x0000
#define AIQB_Device_Write     0x0080
#define AIQB_Device_None      0x0000
#define AIQB_Device_Digital   0x0100
#define AIQB_Device_Potential 0x0200

#define AIQB_Timeout    2000
#define AIQB_Delay      100

#include "Arduino.h"

union _data_t {
  byte c[4];
  float f;
  int i[2];
};

struct _pack_t {
  byte type;
  union _data_t data;
  byte check;
};

class AIQB_Serial {
  public:
    AIQB_Serial (unsigned int Address, byte SwitchPin);
    void tx (unsigned int a, unsigned int r, char d[]);
    void tx (unsigned int a, unsigned int r, float d);
    void tx (unsigned int a, unsigned int r, int d[]);
    void tx (unsigned int a, unsigned int r);
    void tx (unsigned int a);

    void tx (char c, unsigned int a, unsigned int r, union _data_t d);
    void tx (char c, unsigned int a, unsigned int r);
    void tx (char c, unsigned int a);
    
    byte rx ();

    void reg (unsigned int * r);
    void radd (unsigned int r);
    void rrst ();
    void rnxt ();
    void rprv ();

    void val (float * d);
    void val (byte * d);
    void val (int * d);
    
  private:
    void _tx ();
    
    unsigned int Address;
    unsigned int Register;
    byte RegisterPointer;
    union _data_t Data;
    boolean State;
    byte SwitchPin;
    unsigned int Sensors[AIQB_Registry];
};

#endif
