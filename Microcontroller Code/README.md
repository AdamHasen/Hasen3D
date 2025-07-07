These are code samples to be run on Arduino or ESP32 based boards.

Use Arduino IDE and don't forget to install the included libraries at the top of the .ino files

AnimatedGIF Matrix plays a GIF stored on a microSD card. 
You can modify playback speed, size of matrix and number of matricies in series. 
the name of the GIF does not matter, it will play the first one it finds.


espFFT can be used to create an EQ Visualiser with addressable LEDs. 
Must be run on an ESP32 or greater power microcontroller as it is computationally intensive. 
Requires an analog audio signal and outputs a 8x8 or 16x16 matrix data signal. The signal needs to be levelshifted using a basic circuit which can be found on the original creator's repo https://github.com/s-marley/ESP32_FFT_VU
