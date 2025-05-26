# Development of an electronic optimizer to control pump operation.

## Introduction

This repository contains the work and resources related to my engineering thesis titled **"Development of an electronic optimizer to control pump operation."** 

## Abstract 

The engineering thesis focuses on the development and construction of an electronic
(microprocessor-based) model of a setpoint controller for regulating pump performance.
The device will be implemented using a selected microcontroller with appropriately
designed input/output interfaces to interact with external devices and facilitate user
communication. The system’s operating principle can be described as follows: the
controller receives an analog 0 − 10 V signal from a regulating valve, representing the
position of the valve ball. It then compares this value with the pump’s output setpoint.
If the signal from the valve is lower than the setpoint, the device reduces the output
signal by 0.1 V at regular time intervals determined by an encoder until the target value
is reached. Conversely, if the valve ball position value exceeds the setpoint, the
controller increases the output signal by 0.1 V at the specified time intervals. The
system is equipped with three encoders for configuring operating parameters, two analog
inputs 0 − 10 V, and one analog output 0 − 10 V, ensuring precise control and
adaptation of pump performance to operational requirements. The device operates in
two modes: automatic and manual. As part of the thesis, functional tests will be
conducted using available measurement instruments and voltage generators to verify the
correct operation and compliance of the device with the design assumptions. The task is
carried out in accordance with the guidelines and for the purposes of BELIMO -
BELIMO Siłowniki S.A.
