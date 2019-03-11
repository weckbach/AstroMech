from gpiozero import Button
from signal import pause
import subprocess
import os
import sys
from reedsolo import RSCodec
from binascii import hexlify

DATA = sys.argv[1] if len(sys.argv) > 1 else "hello world"
DATA_PERIOD_MS = 50
START_FREQUENCY = 4500
STOP_FREQUENCY = 5000
DELIMITER_FREQUENCY = 4000
NOTES = {
    "0": 600,
    "1": 750,
    "2": 900,
    "3": 1050,
    "4": 1200,
    "5": 1350,
    "6": 1500,
    "7": 1650,
    "8": 1800,
    "9": 1950,
    "a": 2100,
    "b": 2250,
    "c": 2400,
    "d": 2550,
    "e": 2700,
    "f": 2850
}

FIRST_TIME = True

def playDataString():
    global FIRST_TIME
    
    if FIRST_TIME:
        generateWave(DATA)
    
    playCombinedWav()
    FIRST_TIME = False
    
def generateWave(data):
    deleteOldWavs()

    recordStartChar()

    rs = RSCodec(10)

    dataHex = data.encode("ascii")
    dataHex = rs.encode(dataHex)
    dataHex = hexlify(dataHex)

    index = 1
    for c in dataHex:
        if index is ((len(data)+1)*2):
            recordSineWave(index, DELIMITER_FREQUENCY, DATA_PERIOD_MS)
            index = index + 1

        recordSineWave(index, NOTES[c], DATA_PERIOD_MS)
        index = index + 1

    recordStopChar(index)
    combineWavs(index+1)

def deleteOldWavs():
    subprocess.call("rm -fr sound", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    subprocess.call("mkdir sound", shell=True)
def recordStartChar():
    recordSineWave(0, START_FREQUENCY, DATA_PERIOD_MS)

def recordSineWave(index, hertz, durationInMilliseconds):
    recordCommand = "sox -c1 -n sound/%s.wav synth %s sine %s" % (index, durationInMilliseconds/1000.0, hertz)
    subprocess.call(recordCommand, shell=True)

def recordStopChar(index):
    recordSineWave(index, STOP_FREQUENCY, DATA_PERIOD_MS)

def combineWavs(numberOfWavs):
    files = []

    combineCommand = "sox --combine concatenate "

    for x in xrange(0, numberOfWavs):
        combineCommand = combineCommand + "sound/" + str(x) + ".wav" + " "

    combineCommand = combineCommand + "sound/output.wav"

    subprocess.call(combineCommand, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

def playCombinedWav():
    subprocess.call("play sound/output.wav", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

button = Button(14)

button.when_pressed = playDataString

pause()
