# Ham Spot Inc
##  HS-736USB
http://hamspot.com/33-prod-hs-736usb

### FT-736R to HRD USB Interface
The Yaesu FT-736R remains a popular satellite ground station transceiver.  However some modern computer software is not compatible with the CAT interface on the FT-736R.  Ham Radio Deluxe, in particular, does not support the FT-736R.  Ham Radio Deluxe does support the Yaesu FT-847 which has similar features.  We can solve this incompatibility by programming a CAT interface to present a FT-847 to the PC and emulate the new commands not supported on the FT-736R.  

Features not present on the FT-736R are ignored.  Memory channels are not supported and the radio must be set to VFO, also satellite mode must be set to VFO, before enabling CAT control of the radio.  1240 MHz is mapped to 240 MHz because the FT-847 does not have 4 MHz digits.

### Credits
The concept for this interface was developed by Dave, KA6BFB in 2010. He developed a proof of concept in software using VB and Windows. It was however quite cumbersome and used 3 virtual serial ports to work with HRD. But an excellent proof of concept nonetheless. This software can be found here:
[Archives/Software.md](Archives/Software.md)
(archived from http://home.comcast.net/~tinkyr/736/KA6BFB%20Windows%20Emulator.htm).

The hardware version was developed by Chuck, N6BIL. The source code is GPL licensed and the Eagle CAD files were published along with the code. The hardware CAD files, source code and binaries can be found here:
[Archives/Hardware.md](Archives/Hardware.md)
(archived from http://home.comcast.net/~tinkyr/736/N6BIL%20Hardware%20Emulator.htm).

Ham Spot Inc based the HS-736USB on their great work and thanks them immensely for their hard work on solving a challenging problem! The goal of Ham Spot Inc is that if this interface is popular we will develop similar interfaces for other older rigs which are also not supported by HRD. Specifically in mind are the FT-757GXII, FT-890 and various Icom and Kenwood radios. There will likely be firmware updates to this hardware available on the http://hamspot.com/ website in the future. If you have a PIC programmer the chip has been socketed to allow you to flash new firmware or easily replace the device. If not, there will be made available an inexpensive preprogrammed chip that you can plugin to upgrade the firmware or convert the FT-736USB to another model Yaesu rig as they become supported.

### Arduino / ATmega328P port

An Arduino Uno/Nano port of the CAT translator lives in **[Arduino/](Arduino/)**. It keeps the PIC 847↔736 mapping, adds native-736 passthrough (pin A1 or a CAT opcode), PTT/band GPIO, and an S-meter PWM. The PIC firmware in `Firmware/` is unchanged.

```
arduino-cli compile --fqbn arduino:avr:nano Arduino/firmware
cd Arduino && test/run_suite.sh
```

See `Arduino/README.md`, `Arduino/docs/WIRING.md`, and `Arduino/docs/PROTOCOLS.md`. Operating manuals for both the PIC cabinet and the Arduino port are in [`Manual/`](Manual/).
