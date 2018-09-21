# AstroMech

AstroMech is an audio protocol for Arduino that is capable of sending small chunks of data via sound. Its an open source pendant to https://chirp.io/. We use this audio protocol to exchange WiFi credentials between a Raspeberry Pi WiFi Access Point and an ESP32 in order to connect to the network with zero configuration efforts.

Todo: Example

## How it works

AstroMech is a fairly simple one way frequency modulated (FM) audio protocol. There is a sender broadcasting an audio signal that can be consumed by one or multiple receivers.

Each frequency has a hexadecimal symbol assigned to it. Currently there are 16 distinct frequencies defined, which means that each sound encodes four bits. The protocol works asynchronously, thus sender and receiver agree on a fixed baud rate which is currently set to 20. This means that the sender makes 20 sounds per second. The protocol operates in the audible range of `600 Hz` to `2850 Hz`. Each symbol is separated by `150 Hz` in order to avoid overlapping of the symbols. Each incoming sound is analyzed by a Fast Fourier transform (FFT) in order to calculate the matching frequency bucket. Because of the hardware demands of FFT, this library is more suitable for powerful hardware platforms like the ESP32. In addition to the frequencies assigned to hexadecimal symbols there other frequencies used by the protocol.

In order to make the protocol more reliable there is a Reed Solomon code appended to the payload. This makes it possible to correct some errors in transmission and to detect weather the transmission was successful or not.

## Hardware setup

The hardware setup is fairly simple. There is a receiver (Adafruit Feather ESP32) and a sender (Raspberry Pi).

### Receiver

The Adafruit Feather uses the MCP3008 to sample the audio signal of a microphone. Most built-in ADCs aren't sampling fast enough (sampling rate should be at least twice as fast as the highest frequency to detect). This is why most of the time you find yourself including an additional ADC converter like the MCP3008, which is fast enough for our purposes. We have tested it with a SparkFun MEMS microphone (https://www.sparkfun.com/products/9868), but an electred microphone (e.g. https://www.sparkfun.com/products/12758) should suffice as well. The following Fritzing illustrates the hardware setup on the receiver side.

![Fritzing](https://github.com/weckbach/AstroMech/blob/master/images/astromech.png "Fritzing")


### Sender

We use a Raspberry Pi with an active speaker connected to cinch connector. This is a really simple setup because the active speaker can get really loud. Also the audio driver of the Raspberry Pi can be used in order to create a sine wave signal. This is necessary, because a square wave signal can't be detected by the Fourier transform. You can use the little python script under ```RaspberryPi/main.py``` to emit a "Hello world" sound on the Raspberry.

## Software

### Installation

#### Arduino IDE

This library is fully self contained (includes all dependencies). This is because the Arduino does not support a sophisticated dependency management. Since the library is self contained you can simply download this repository as a zip file and include it into your Arduino libraries.

#### Platform IO

This library is also compliant with platform.io. Just add the following to your platform.ini:

```
lib_deps =
  AstroMech
```

### Usage

Check out the example, it is pretty much self explaining. To make the library reusable for other ICs or even the internal ADC, you need to pass the analogRead function that the library to use. Here is an example how to define this function:

```c++
Adafruit_MCP3008 adc;

int analogReadFP(uint8_t pin){
  return adc.readADC(pin);
}

```

When initializing AstroMech you have to pass it to the constructor like this:

```c++
... new AstroMech::AstroMech(analogReadFP, ANALOG_PIN, DEBUG);
```

The debug flag indicates if AstroMech should print its internals to the console.

### Configuration

Check out the macros defined in ```AstroMech.h```. Those can be used to tweak the protocol for your needs (e.g. different speaker or microphone). The following table explains the most important macros:

Name | Description | Default
--- | --- | ---
DATA_PERIOD_IN_MILLISECONS | How long to wait for the next symbol to be sent. The baudrate is `1 / (DATA_PERIOD_IN_MILLISECONS / 1000)`. | `50`
TIMEOUT_IN_MS | Determines how long the receiver waits for the stop bit before it times out. | `500`
START_FREQUENCY | Frequency of lowest symbol (`0x0`). | `600`
FREQUENCY_STEP_SIZE | Distance between each symbol. | `150`
FREQUENCY_RANGE |  Determines how far away from the frequency center for each symbol should consider peaks. E.g. a frequency of `560` is considered the match the frequency `600` if the Frequency range is greater than `40`. This makes the protocol more resilient. Maximum range is `FREQUENCY_STEP_SIZE / 2`. | `60`
START_SYMBOL_FREQUENCY | Analogous to a start bit in serial communication. | `4500`
STOP_SYMBOL_FREQUENCY |  Analogous to a stop bit in serial communication. | `5000`
DELIMTER_SYMBOL_FREQUENCY | Separates payload from the error correction. | `4000`
AMPLITUDE_THRESHOLD | Determines how high the amplitude of a frequency has to be in order to be considered a symbol and not background noise. | `300`

## A note on ultrasonic

Ultrasonic has not been tested yet. Though, it should be fairly straight forward to implement. All the frequencies have to be shifted into the ultrasonic range. The hardware has to be replaced as well. You need to pick a microphone with ultrasonic in the frequency range. Also the speakers must be able to play ultrasonic sounds. I havent't tried to sample the > `40 kHz` with the MCP3008. That has to  be tested as well. I would be glad for any feedback on that topic.

## Known issues

### Start/Stop

With our speaker we experienced that, if the microphone is too close to the speaker, the receiver detects a stop bit right after the start bit. This issue could be mitigated by lowering the priority of stop signal.

### Background noise

If there is too much background noise, like music, the transmission is likely to fail. This is due to the audible frequency range. This can either be fixed by implementing an ultrasonic setup (drawback is that you can't hear it anymore), or by adjust the algorithm to adapt the threshold to the background noise. Any pull requests or forks on that topic are appreciated!
