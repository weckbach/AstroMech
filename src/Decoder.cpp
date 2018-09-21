#include "Decoder.h"
#include "RS-FEC.h"

namespace AstroMech{
  Decoder::Decoder(){
  }

  bool Decoder::addSymbol(byte hexSymbol){
    if (bufferIndex >= BUFFER_SIZE)
      return false;

    byteIndex %= 2;
    byteBuffer[byteIndex++] = hexSymbol;

    if(byteIndex == 2){
      byte symbol = (byteBuffer[0] << 4) | byteBuffer[1];
      inputBuffer[bufferIndex++] = symbol;
    }

    return true;
  }

  void Decoder::delimit(){
    payloadLength = bufferIndex;
    bufferIndex = MESSAGE_LENGTH;
  }

  String Decoder::decode(){
    String result;
    RS::ReedSolomon rs(payloadLength, PARITY_LENGTH);

    char encoded[payloadLength+PARITY_LENGTH];
    char repaired[payloadLength];

    getEncoded(encoded, sizeof(encoded));
    bool success = !rs.Decode(encoded, repaired);

    if(success){
      result = getResult(repaired);
    }

    for(int i=0; i<payloadLength+PARITY_LENGTH, i++;){
      Serial.println(encoded[i], HEX);
    }

    reset();

    return result;
  }

  void Decoder::getEncoded(char buffer[], size_t bufferSize){
    memset(buffer, 0, bufferSize);

    // set payload
    for(int i=0; i<payloadLength; i++){
      buffer[i] = inputBuffer[i];
    }

    // set ecc
    for(int i=0; i<PARITY_LENGTH; i++){
      buffer[payloadLength+i] = inputBuffer[MESSAGE_LENGTH+i];
    }
  }

  String Decoder::getResult(char repaired[]){
    String result;
    for(int i=0; i<payloadLength; i++){
      result += String(repaired[i]);
    }

    return result;
  }

  void Decoder::reset(){
    memset(inputBuffer, 0, sizeof(inputBuffer));
    bufferIndex = 0;
    payloadLength = 0;

    memset(byteBuffer, 0, sizeof(byteBuffer));
    byteIndex = 0;
  }
}
