# Indoor Basketball Scoreboard

This repository holds the development of a simple Arduino-based (Nano v3) scoreboard, designed for a child's indoor basketball hoop.  The key attributes (as prompted by grandchild!) are:
- Large 7-segment display digits (57mm/2.3")
- Countdown clock (2 digit)
- Hoop counter (2 digit) displaying successful shots within countdown period
- Audible buzzer with separate tones/melody sounding for:  precount period; hoops registered;  end of shooting period
- External pushbutton to initiate countdown and shooting period

The approach taken was informed by several previously-published scoreboard projects:
- https://www.instructables.com/Arduino-Home-Basketball-Hoop-Score-Detection-Syste/
- https://www.instructables.com/Arduino-Basketball-Pop-a-Shot-Upgrayedd/

but particularly around the challenge of circuit design for driving the higher current levels required by large size segment displays:
- http://wayoda.github.io/LedControl/index.html

The final segment-driver h/w design adopted a hybrid combination of MAX7219 LED Display Driver to source the segment current, in combination with MAX394 analog switches to control current sinking through MOSFETs.   This design is documented (for 2 digits) in the MAX7219 device datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf

A VL6180X Time of Flight sensor https://www.st.com/resource/en/datasheet/vl6180x.pdf mounted inside the basketball hoop was used to detect the passage of a ball through the hoop.  The DFRobot breakout and software library for this sensor https://wiki.dfrobot.com/DFRobot_VL6180X_TOF_Distance_Ranging_Sensor_Breakout_Board_SKU_SEN0427 provided access to an interrupt signal which can be set to recognise a distance ranging measurement corresponding to the passage of the ball.

The physical packaging design encompassed:
- display digits mounted through a thin wooden board that can be attached to the top of the basketball backboard
- separated circuit boards for independent construction & troubleshooting of:
    - Arduino controller and signal interfacing
    - 12v to +5V/-5V DC Buck Boost converter
    - Segment driver and current handling electronics
- boxing of electronics with external sensor connections and 12V DC power pack.
