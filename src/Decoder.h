#ifndef AstroMechDecoder
#define AstroMechDecoder

#include "Arduino.h"
#include "RS-FEC.h"

#define MESSAGE_LENGTH 100
#define PARITY_LENGTH 10
#define BUFFER_SIZE MESSAGE_LENGTH + PARITY_LENGTH

namespace AstroMech{
  class Decoder {
  private:

    char inputBuffer[BUFFER_SIZE];
    uint8_t payloadLength = 0;
    uint8_t bufferIndex = 0;

    uint8_t byteBuffer[2];
    uint8_t byteIndex = 0;

    void getEncoded(char buffer[], size_t bufferSize);
    String getResult(char repaired[]);
  public:
    Decoder();
    bool addSymbol(byte hexSymbol);
    void delimit();
    String decode();
    void reset();
  };
}
#endif
