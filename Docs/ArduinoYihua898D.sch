EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ArduinoNano 898D Microcontroller Replacement"
Date "2020-07-20"
Rev "1.0"
Comp "Denis Dowling"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Module:Arduino_Nano_v2.x A?
U 1 1 5F131BB9
P 6150 3650
F 0 "A?" H 5800 2550 50  0000 C CNN
F 1 "Arduino_Nano_v2.x" H 5650 2650 50  0000 C CNN
F 2 "Module:Arduino_Nano" H 6150 3650 50  0001 C CIN
F 3 "https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf" H 6150 3650 50  0001 C CNN
	1    6150 3650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x10_Counter_Clockwise J?
U 1 1 5F135D75
P 3650 3750
F 0 "J?" H 3700 3100 50  0000 C CNN
F 1 "Conn_02x10_Counter_Clockwise" H 3700 4276 50  0001 C CNN
F 2 "" H 3650 3750 50  0001 C CNN
F 3 "~" H 3650 3750 50  0001 C CNN
	1    3650 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 4050 3100 4050
Wire Wire Line
	3450 3350 3450 3050
Wire Wire Line
	3450 3050 2950 3050
Wire Wire Line
	3450 4150 3100 4150
Wire Wire Line
	3450 4250 3100 4250
$Comp
L power:GND #PWR?
U 1 1 5F1391E8
P 2950 3100
F 0 "#PWR?" H 2950 2850 50  0001 C CNN
F 1 "GND" H 2955 2927 50  0000 C CNN
F 2 "" H 2950 3100 50  0001 C CNN
F 3 "" H 2950 3100 50  0001 C CNN
	1    2950 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 3050 2950 3100
NoConn ~ 3450 3450
NoConn ~ 3450 3750
NoConn ~ 3450 3850
NoConn ~ 3450 3950
NoConn ~ 3950 3450
NoConn ~ 3950 3550
NoConn ~ 3950 3650
Text Label 3100 4050 0    50   ~ 0
DIO
Text Label 3100 4150 0    50   ~ 0
CLK
Text Label 3100 4250 0    50   ~ 0
STB
Wire Wire Line
	3450 3550 3100 3550
Wire Wire Line
	3450 3650 3100 3650
Text Label 3100 3550 0    50   ~ 0
SoftAir
Text Label 3100 3650 0    50   ~ 0
SoftIron
Wire Wire Line
	3950 4250 4350 4250
Wire Wire Line
	3950 4150 4350 4150
Wire Wire Line
	3950 4050 4350 4050
Wire Wire Line
	3950 3950 4350 3950
Wire Wire Line
	3950 3850 4350 3850
Wire Wire Line
	3950 3750 4350 3750
Wire Wire Line
	3950 3350 4050 3350
Wire Wire Line
	4050 3350 4050 3050
Text Label 4000 4250 0    50   ~ 0
IronTriacDriver
Text Label 4000 4150 0    50   ~ 0
AirTriacDriver
Text Label 4000 4050 0    50   ~ 0
IronOpampOutput
Text Label 4000 3950 0    50   ~ 0
AirOpampOutput
Text Label 4000 3850 0    50   ~ 0
HolsterDetect
Text Label 4000 3750 0    50   ~ 0
FanControl
$Comp
L power:+5V #PWR?
U 1 1 5F13F3C9
P 4050 3050
F 0 "#PWR?" H 4050 2900 50  0001 C CNN
F 1 "+5V" H 4065 3223 50  0000 C CNN
F 2 "" H 4050 3050 50  0001 C CNN
F 3 "" H 4050 3050 50  0001 C CNN
	1    4050 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 4650 6150 5000
$Comp
L power:GND #PWR?
U 1 1 5F13FF2D
P 6150 5000
F 0 "#PWR?" H 6150 4750 50  0001 C CNN
F 1 "GND" H 6155 4827 50  0000 C CNN
F 2 "" H 6150 5000 50  0001 C CNN
F 3 "" H 6150 5000 50  0001 C CNN
	1    6150 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 2650 6350 2350
$Comp
L power:+5V #PWR?
U 1 1 5F1408DC
P 6350 2350
F 0 "#PWR?" H 6350 2200 50  0001 C CNN
F 1 "+5V" H 6365 2523 50  0000 C CNN
F 2 "" H 6350 2350 50  0001 C CNN
F 3 "" H 6350 2350 50  0001 C CNN
	1    6350 2350
	1    0    0    -1  
$EndComp
NoConn ~ 5650 3050
NoConn ~ 5650 3150
NoConn ~ 5650 4250
NoConn ~ 5650 4350
NoConn ~ 6650 3150
NoConn ~ 6650 3050
Wire Wire Line
	6250 2650 6250 2500
Wire Wire Line
	6250 2500 6800 2500
Wire Wire Line
	6800 2500 6800 3450
Wire Wire Line
	6800 3450 6650 3450
Wire Wire Line
	6650 3650 7400 3650
Wire Wire Line
	6650 3750 7400 3750
Wire Wire Line
	5650 3250 5100 3250
Wire Wire Line
	5650 3350 5100 3350
Wire Wire Line
	5650 3450 5100 3450
Wire Wire Line
	5650 3550 5100 3550
Wire Wire Line
	5650 3650 5100 3650
Wire Wire Line
	5650 3750 5100 3750
Wire Wire Line
	5650 3850 5100 3850
Wire Wire Line
	5650 3950 5100 3950
Wire Wire Line
	5650 4050 5100 4050
Text Label 5100 3250 0    50   ~ 0
AirTriacDriver
Text Label 5100 3350 0    50   ~ 0
IronTriacDriver
Text Label 5100 3450 0    50   ~ 0
FanControl
Text Label 5100 3550 0    50   ~ 0
SoftIron
Text Label 5100 3650 0    50   ~ 0
SoftAir
Text Label 5100 3750 0    50   ~ 0
STB
Text Label 5100 3850 0    50   ~ 0
DIO
Text Label 5100 3950 0    50   ~ 0
CLK
Text Label 5100 4050 0    50   ~ 0
HolsterDetect
NoConn ~ 5650 4150
Text Label 6800 3650 0    50   ~ 0
AirOpampOutput
Text Label 6800 3750 0    50   ~ 0
IronOpampOutput
$EndSCHEMATC
