# FTIR-Spectro

A DIY Infrared spectrometry project based on michelson interferometry and Fourrier transform analysis
The system relies on a Atmega32u4 MCU as its core and the code is developed using the abstracted Arduino environment.

## What is the current state ?

Last Update: 27.08.2016
The FTIR DIY spectrometer is currently in stanbye and needs students for the project to continue on.
The project is currently based at Octanis Assocation, in the Hackuarium Hackerspace in Renens, Switzerland.
More can be found on these organisations at:

  http://octanis.org   
  http://wiki.hackuarium.ch/w/Main_Page
  
## What needs to be done ?

* Change CAD so that material swelling due to water uptake is taken into account (or change Nylon for a more stable material)
* Retest the stability of the AC-DC network 96V rectifier down to mV oscillations
* Make a proper PCB board for the 96V rectifier with BOM and dedicated documentation and Specs
* Align the mirrors system with a laser
* Test the mirror displacement due to the piezo crystal with a laser and charaterize it
* Program the Atmega MCU to drive the DAC output and test this output voltage (it might be necessary to reorder a board)
* Couple the Atmega MCU to the 0-5V --> 0-96V converter and charaterize the output voltage
* Test the Control of the piezo dilatation with the Atmega and the 0-96V output you charaterized
* Determine if the sensor sensitivity is high enough, if not various options can be explored: change the amplifier resistor bridge, cool the sensor down to -50°C with a Peltier dedicated device to reduce SNR, test a PIR type of sensor, find a more suited sensor.
* Once this stage is complete determine how much light is lost passing in the PE liquid holding cell when empty and scan with the piezo to get the empty cell response (need to program a control loop on the MCU with sensor reading)
* Test the device with different liquids and perform fft on the data provided by the Atmega board to get spectra
* Charaterize for ammonia detection
* Provide a deliverable kit with user manual

Each step must be documented so no work is lost and the community can enjoy the results !!

## Contact

You can contact me concerning the project at quentin.cabrol@gmail.com.
We are currently searching for members to enroll and work on this project
