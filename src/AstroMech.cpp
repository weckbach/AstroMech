#include "Arduino.h"
#include "AstroMech.h"
#include "arduinoFFT.h"
#include "Decoder.h"

namespace AstroMech{
  AstroMech::AstroMech(AnalogReadFPT analogReadFP, int analogPin, bool debug){
    this->analogReadFP = analogReadFP;
    this->analogPin = analogPin;
    this->debug = debug;
    this->sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
    this->fft = arduinoFFT();
    this->lastDataTickInMS = 0;
    this->decoder = new Decoder();

    initFrequencies();
  }

  void AstroMech::initFrequencies(){
    uint16_t frequency = START_FREQUENCY;

    for(int i = 0; i < 16; i++){
      frequencies[i] = frequency;
      frequency += FREQUENCY_STEP_SIZE;
    }

    frequencies[DELIMITER_SYMBOL_INDEX] = DELIMTER_SYMBOL_FREQUENCY;
  }

  String AstroMech::detect(){
    String data = "";

    sample();

    if(isStartDetected()){
      lastDataTickInMS = millis();
      waitForNextDataTick();

      do{
        sample();
        addSymbol();
        waitForNextDataTick();
        yield();
      }while(!isStopOrTimeoutDetected());

      data = decoder->decode();

      if(debug)  Serial.println(data);
      waitForNextDataTick();
    }

    return data;
  }

  void AstroMech::sample(){
    unsigned long microseconds;


    for(int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      microseconds = micros();

      realValues[i] = analogReadFP(0);
      imaginaryValues[i] = 0;

      waitForNextSampleTick(microseconds);
    }

    fft.Windowing(realValues, NUMBER_OF_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft.Compute(realValues, imaginaryValues, NUMBER_OF_SAMPLES, FFT_FORWARD);
    fft.ComplexToMagnitude(realValues, imaginaryValues, NUMBER_OF_SAMPLES);
  }

  void AstroMech::addSymbol(){
    int symbol = getSymbol();

    if(symbol == DELIMITER_SYMBOL){
      decoder->delimit();
    }else {
      decoder->addSymbol(symbol);
    }
  }

  int AstroMech::getSymbol(){
    uint16_t dominantFrequency = getDominantFrequency();

    if(!dominantFrequency){
      return 0;
    }

    lastSymbolReceivedInMS = millis();

    if(dominantFrequency == DELIMTER_SYMBOL_FREQUENCY){
      return symbols[DELIMITER_SYMBOL_INDEX];
    }

    return symbols[(dominantFrequency - START_FREQUENCY) / FREQUENCY_STEP_SIZE];
  }

  uint16_t AstroMech::getDominantFrequency(){
    uint16_t dominantFrequency = 0;
    uint16_t amplitude;
    uint16_t highestAmplitude = 0;

    for(int i = 0; i < 17; i++){
      amplitude = getAmplitudeForFrequency(frequencies[i]);

      if(amplitude > AMPLITUDE_THRESHOLD && amplitude > highestAmplitude){
        highestAmplitude = amplitude;
        dominantFrequency = frequencies[i];
      }
    }

    return dominantFrequency;
  }

  uint16_t AstroMech::getAmplitudeForFrequency(uint16_t frequency){
    uint8_t lowBin = getBinForFrequency(frequency - FREQUENCY_RANGE);
    uint8_t highBin = getBinForFrequency(frequency + FREQUENCY_RANGE);

    uint16_t highestAmplitude = 0;
    for(uint8_t i = lowBin; i<= highBin; i++){
      if(realValues[i] > highestAmplitude){
        highestAmplitude = realValues[i];
      }
    }

    return highestAmplitude;
  }

  uint8_t AstroMech::getBinForFrequency(uint16_t frequency){
    return (frequency * NUMBER_OF_SAMPLES) / SAMPLING_FREQUENCY;
  }

  void AstroMech::waitForNextSampleTick(unsigned long lastMicroseconds){
    while(micros() < (lastMicroseconds + sampling_period_us)){
    }
  }

  bool AstroMech::isStartDetected(){
    if(getAmplitudeForFrequency(START_SYMBOL_FREQUENCY) > AMPLITUDE_THRESHOLD){
      if(debug) Serial.println("Start detected");
      return true;
    }

    return false;
  }

  bool AstroMech::isStopOrTimeoutDetected(){
    if((millis() - lastSymbolReceivedInMS) > TIMEOUT_IN_MS || getAmplitudeForFrequency(STOP_SYMBOL_FREQUENCY) > AMPLITUDE_THRESHOLD){
      if(debug) Serial.println("Stop detected");
      return true;
    }

    return false;
  }

  void AstroMech::waitForNextDataTick(){
    if(lastDataTickInMS == 0) lastDataTickInMS = millis();

    while(millis() < (lastDataTickInMS + DATA_PERIOD_IN_MILLISECONS)){
    }

    lastDataTickInMS = millis();
  }
}
