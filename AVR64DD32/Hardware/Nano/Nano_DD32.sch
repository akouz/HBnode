EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Arduino Nano_DD32"
Date "2023-02-10"
Rev "1.0"
Comp "=AK="
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	1600 1400 2050 1400
Wire Wire Line
	1600 1500 2050 1500
Wire Wire Line
	1600 1800 2050 1800
Wire Wire Line
	1600 1900 2050 1900
Wire Wire Line
	1600 2000 2050 2000
Wire Wire Line
	1600 2100 2050 2100
Wire Wire Line
	1600 2200 2050 2200
Wire Wire Line
	1600 2300 2050 2300
Wire Wire Line
	1600 2400 2050 2400
Wire Wire Line
	1600 2500 2050 2500
Wire Wire Line
	1600 1700 1650 1700
Text Label 2050 1400 2    50   ~ 0
TX1
Text Label 2050 1500 2    50   ~ 0
RX1
Text Label 2050 1800 2    50   ~ 0
D2
Text Label 2050 1900 2    50   ~ 0
D3
Text Label 2050 2000 2    50   ~ 0
D4
Text Label 2050 2100 2    50   ~ 0
D5
Text Label 2050 2200 2    50   ~ 0
D6
Text Label 2050 2300 2    50   ~ 0
D7
Text Label 2050 2400 2    50   ~ 0
D8
Text Label 2400 1900 0    50   ~ 0
D9-SDA
Wire Wire Line
	2900 1500 2850 1500
Wire Wire Line
	2900 1700 2650 1700
Wire Wire Line
	2900 1800 2400 1800
Wire Wire Line
	2900 1900 2400 1900
Wire Wire Line
	2900 2000 2400 2000
Wire Wire Line
	2900 2100 2400 2100
Wire Wire Line
	2900 2200 2400 2200
Wire Wire Line
	2900 2400 2400 2400
Wire Wire Line
	2900 2500 2400 2500
Wire Wire Line
	2900 2300 2400 2300
NoConn ~ 2900 1400
Text Label 2400 2200 0    50   ~ 0
A3
Text Label 2400 2300 0    50   ~ 0
A2
Text Label 2400 2400 0    50   ~ 0
A1
Text Label 2400 2500 0    50   ~ 0
A0
Text Label 2050 2700 2    50   ~ 0
D11-MOSI
Text Label 2050 2800 2    50   ~ 0
D12-MISO
Text Label 2400 2800 0    50   ~ 0
D13-SCK
Wire Wire Line
	8300 3650 8300 4350
Wire Wire Line
	8400 3650 8400 4350
Wire Wire Line
	8500 3650 8500 4350
Wire Wire Line
	8700 3650 8700 4350
Wire Wire Line
	9100 3250 9600 3250
Text Label 8700 4350 1    50   ~ 0
D2
Text Label 9600 3250 2    50   ~ 0
D3
Text Label 9600 2950 2    50   ~ 0
D4
Text Label 9600 2850 2    50   ~ 0
D5
Text Label 9600 2750 2    50   ~ 0
D6
Text Label 9600 2650 2    50   ~ 0
D7
Wire Wire Line
	8300 2150 8300 2100
Wire Wire Line
	8300 2100 9150 2100
Wire Wire Line
	9150 2100 9150 3050
Wire Wire Line
	9150 3050 9100 3050
Wire Wire Line
	9150 3050 9150 4250
Connection ~ 9150 3050
Wire Wire Line
	8400 2150 8400 2000
Wire Wire Line
	8400 2000 9250 2000
Wire Wire Line
	9250 2000 9250 3150
Wire Wire Line
	9250 3150 9100 3150
Wire Wire Line
	8100 3750 9250 3750
Wire Wire Line
	9250 3750 9250 3150
Connection ~ 9250 3150
$Comp
L power:COM #PWR011
U 1 1 63AFEE27
P 9150 4300
F 0 "#PWR011" H 9150 4100 50  0001 C CNN
F 1 "COM" H 9150 4200 50  0000 C CNN
F 2 "" H 9150 4300 50  0001 C CNN
F 3 "" H 9150 4300 50  0001 C CNN
	1    9150 4300
	1    0    0    -1  
$EndComp
$Comp
L power:COM #PWR01
U 1 1 63AFF2E3
P 1650 1700
F 0 "#PWR01" H 1650 1500 50  0001 C CNN
F 1 "COM" H 1650 1600 50  0000 C CNN
F 2 "" H 1650 1700 50  0001 C CNN
F 3 "" H 1650 1700 50  0001 C CNN
	1    1650 1700
	0    -1   -1   0   
$EndComp
$Comp
L power:+5V #PWR012
U 1 1 63B001AD
P 9250 2000
F 0 "#PWR012" H 9250 1850 50  0001 C CNN
F 1 "+5V" H 9265 2173 50  0000 C CNN
F 2 "" H 9250 2000 50  0001 C CNN
F 3 "" H 9250 2000 50  0001 C CNN
	1    9250 2000
	1    0    0    -1  
$EndComp
Connection ~ 9250 2000
$Comp
L power:+5V #PWR02
U 1 1 63B00723
P 2650 1250
F 0 "#PWR02" H 2650 1100 50  0001 C CNN
F 1 "+5V" H 2665 1423 50  0000 C CNN
F 2 "" H 2650 1250 50  0001 C CNN
F 3 "" H 2650 1250 50  0001 C CNN
	1    2650 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 1250 2650 1700
$Comp
L power:COM #PWR03
U 1 1 63B00F26
P 2850 1500
F 0 "#PWR03" H 2850 1300 50  0001 C CNN
F 1 "COM" H 2850 1400 50  0000 C CNN
F 2 "" H 2850 1500 50  0001 C CNN
F 3 "" H 2850 1500 50  0001 C CNN
	1    2850 1500
	0    1    1    0   
$EndComp
Wire Wire Line
	9100 2950 9600 2950
Wire Wire Line
	9100 2850 9600 2850
Wire Wire Line
	9100 2750 9600 2750
Wire Wire Line
	9100 2650 9600 2650
Text Label 7050 2950 0    50   ~ 0
A0
Text Label 7050 3250 0    50   ~ 0
A1
Text Label 8200 4350 1    50   ~ 0
A2
Text Label 8300 4350 1    50   ~ 0
A3
Text Label 7050 2650 0    50   ~ 0
D11-MOSI
Text Label 7050 2750 0    50   ~ 0
D12-MISO
Text Label 7050 2850 0    50   ~ 0
D13-SCK
Wire Wire Line
	7600 2650 7050 2650
Wire Wire Line
	7600 2750 7050 2750
Wire Wire Line
	7600 2850 7050 2850
$Comp
L Nano_DD32-rescue:Crystal_GND24_Small-Device Y1
U 1 1 63B13B13
P 7950 1750
F 0 "Y1" H 7450 1900 50  0000 L CNN
F 1 "24MHz" H 7350 1800 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_2520-4Pin_2.5x2.0mm" H 7950 1750 50  0001 C CNN
F 3 "~" H 7950 1750 50  0001 C CNN
	1    7950 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 63B142EB
P 8200 1400
F 0 "C3" H 8315 1446 50  0000 L CNN
F 1 "22p" H 8315 1355 50  0000 L CNN
F 2 "Discret:0603" H 8238 1250 50  0001 C CNN
F 3 "~" H 8200 1400 50  0001 C CNN
	1    8200 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 63B14CA8
P 7650 1400
F 0 "C2" H 7765 1446 50  0000 L CNN
F 1 "22p" H 7765 1355 50  0000 L CNN
F 2 "Discret:0603" H 7688 1250 50  0001 C CNN
F 3 "~" H 7650 1400 50  0001 C CNN
	1    7650 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8200 1550 8200 1750
Wire Wire Line
	8200 1750 8050 1750
Wire Wire Line
	7850 1750 7650 1750
Wire Wire Line
	7650 1750 7650 1550
Wire Wire Line
	8200 2150 8200 1750
Connection ~ 8200 1750
Wire Wire Line
	8100 2150 8100 2050
Wire Wire Line
	8100 2050 7650 2050
Wire Wire Line
	7650 2050 7650 1750
Connection ~ 7650 1750
Wire Wire Line
	7950 1850 7950 1900
Wire Wire Line
	7950 1900 7800 1900
Wire Wire Line
	7800 1900 7800 1600
Wire Wire Line
	7800 1600 7950 1600
Wire Wire Line
	7950 1600 7950 1650
Wire Wire Line
	7650 1250 7650 1200
Wire Wire Line
	7650 1200 7950 1200
Wire Wire Line
	8200 1200 8200 1250
Wire Wire Line
	7950 1600 7950 1200
Connection ~ 7950 1600
Connection ~ 7950 1200
Wire Wire Line
	7950 1200 8200 1200
$Comp
L power:COM #PWR010
U 1 1 63B20C59
P 7950 1900
F 0 "#PWR010" H 7950 1700 50  0001 C CNN
F 1 "COM" H 7950 1800 50  0000 C CNN
F 2 "" H 7950 1900 50  0001 C CNN
F 3 "" H 7950 1900 50  0001 C CNN
	1    7950 1900
	1    0    0    -1  
$EndComp
Connection ~ 7950 1900
Text Label 4800 4750 2    50   ~ 0
BOOT-SDA
Text Label 4800 4850 2    50   ~ 0
BOOT-SCL
Text Label 9600 2550 2    50   ~ 0
D8
Wire Wire Line
	8500 2150 8500 1600
Wire Wire Line
	8600 2150 8600 1600
Text Label 8500 1600 3    50   ~ 0
UPDI
Text Label 8600 1600 3    50   ~ 0
~RESET
Text Label 8700 1200 3    50   ~ 0
~LED-R
$Comp
L Device:C C4
U 1 1 63B1179E
P 9500 4050
F 0 "C4" H 9615 4096 50  0000 L CNN
F 1 "100n" H 9615 4005 50  0000 L CNN
F 2 "Discret:0603" H 9538 3900 50  0001 C CNN
F 3 "~" H 9500 4050 50  0001 C CNN
	1    9500 4050
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 63B11F4D
P 9900 4050
F 0 "C5" H 10015 4096 50  0000 L CNN
F 1 "10uF" H 10015 4005 50  0000 L CNN
F 2 "Discret:0805" H 9938 3900 50  0001 C CNN
F 3 "~" H 9900 4050 50  0001 C CNN
	1    9900 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 3750 9500 3750
Wire Wire Line
	9900 3750 9900 3900
Connection ~ 9250 3750
Wire Wire Line
	9500 3900 9500 3750
Connection ~ 9500 3750
Wire Wire Line
	9500 3750 9900 3750
Wire Wire Line
	9900 4200 9900 4250
Wire Wire Line
	9900 4250 9500 4250
Connection ~ 9150 4250
Wire Wire Line
	9150 4250 9150 4300
Wire Wire Line
	9500 4200 9500 4250
Connection ~ 9500 4250
Wire Wire Line
	9500 4250 9150 4250
$Comp
L power:+5V #PWR09
U 1 1 63B273FC
P 5450 1600
F 0 "#PWR09" H 5450 1450 50  0001 C CNN
F 1 "+5V" H 5465 1773 50  0000 C CNN
F 2 "" H 5450 1600 50  0001 C CNN
F 3 "" H 5450 1600 50  0001 C CNN
	1    5450 1600
	1    0    0    -1  
$EndComp
$Comp
L power:COM #PWR07
U 1 1 63B31150
P 5000 1900
F 0 "#PWR07" H 5000 1700 50  0001 C CNN
F 1 "COM" H 5000 1800 50  0000 C CNN
F 2 "" H 5000 1900 50  0001 C CNN
F 3 "" H 5000 1900 50  0001 C CNN
	1    5000 1900
	1    0    0    -1  
$EndComp
Text Label 5800 2200 2    50   ~ 0
UPDI
Text Label 5800 1700 2    50   ~ 0
~RESET
NoConn ~ 1600 1600
NoConn ~ 2900 1600
Text Notes 1900 1600 0    50   ~ 0
~RST
Text Notes 2400 1600 0    50   ~ 0
~RST
Text Notes 2400 1400 0    50   ~ 0
RAW
Text Notes 1900 1700 0    50   ~ 0
GND
Text Notes 2400 1500 0    50   ~ 0
GND
Text Notes 2400 1700 0    50   ~ 0
VCC
$Comp
L ic:24C256 U1
U 1 1 63B616F5
P 1950 4650
F 0 "U1" H 2350 4915 50  0000 C CNN
F 1 "24C512" H 2350 4824 50  0000 C CNN
F 2 "Package_SO:TSSOP-8_4.4x3mm_P0.65mm" H 2350 4500 50  0001 C CNN
F 3 "E:/Doc/Datasheets/Microchip/EEPROM/24LC256_1203P.pdf" H 2350 4831 60  0001 C CNN
	1    1950 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2750 4850 3650 4850
Wire Wire Line
	2750 4950 3350 4950
Wire Wire Line
	3650 4350 3650 4850
$Comp
L power:+5V #PWR04
U 1 1 63B7F213
P 3150 3950
F 0 "#PWR04" H 3150 3800 50  0001 C CNN
F 1 "+5V" H 3165 4123 50  0000 C CNN
F 2 "" H 3150 3950 50  0001 C CNN
F 3 "" H 3150 3950 50  0001 C CNN
	1    3150 3950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 63B7FB29
P 3150 5200
F 0 "C1" H 3265 5246 50  0000 L CNN
F 1 "100n" H 3265 5155 50  0000 L CNN
F 2 "Discret:0603" H 3188 5050 50  0001 C CNN
F 3 "~" H 3150 5200 50  0001 C CNN
	1    3150 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 3950 3350 3950
Wire Wire Line
	3650 3950 3650 4050
Connection ~ 3150 3950
Wire Wire Line
	3350 4050 3350 3950
Connection ~ 3350 3950
Wire Wire Line
	3350 3950 3650 3950
Wire Wire Line
	2750 4650 3150 4650
Connection ~ 3150 4650
Wire Wire Line
	3150 4650 3150 5050
Wire Wire Line
	2750 4750 2900 4750
Wire Wire Line
	2900 4750 2900 5400
Wire Wire Line
	2900 5400 3150 5400
Wire Wire Line
	3150 5400 3150 5350
Wire Wire Line
	1950 4650 1850 4650
Wire Wire Line
	1850 4650 1850 4750
Wire Wire Line
	1850 5400 2900 5400
Connection ~ 2900 5400
Wire Wire Line
	1950 4750 1850 4750
Connection ~ 1850 4750
Wire Wire Line
	1950 4950 1850 4950
Connection ~ 1850 4950
Wire Wire Line
	1850 4950 1850 5400
$Comp
L power:COM #PWR05
U 1 1 63BAC395
P 3150 5450
F 0 "#PWR05" H 3150 5250 50  0001 C CNN
F 1 "COM" H 3150 5350 50  0000 C CNN
F 2 "" H 3150 5450 50  0001 C CNN
F 3 "" H 3150 5450 50  0001 C CNN
	1    3150 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 5400 3150 5450
Connection ~ 3150 5400
Wire Wire Line
	3350 4350 3350 4750
Connection ~ 3650 4850
Connection ~ 3350 4750
Wire Wire Line
	3350 4750 3350 4950
$Comp
L Nano_DD32-rescue:LED-Device D2
U 1 1 63BD4432
P 10400 1550
F 0 "D2" H 10393 1767 50  0000 C CNN
F 1 "RED" H 10393 1676 50  0000 C CNN
F 2 "Discret:0603_LED" H 10400 1550 50  0001 C CNN
F 3 "~" H 10400 1550 50  0001 C CNN
	1    10400 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 1550 8700 2150
Wire Wire Line
	8700 1550 9750 1550
Wire Wire Line
	10250 1550 10050 1550
Wire Wire Line
	10550 1550 10700 1550
Wire Wire Line
	10700 1550 10700 1450
$Comp
L power:+5V #PWR014
U 1 1 63BF8D75
P 10700 1450
F 0 "#PWR014" H 10700 1300 50  0001 C CNN
F 1 "+5V" H 10715 1623 50  0000 C CNN
F 2 "" H 10700 1450 50  0001 C CNN
F 3 "" H 10700 1450 50  0001 C CNN
	1    10700 1450
	1    0    0    -1  
$EndComp
$Comp
L Nano_DD32-rescue:R-Device R1
U 1 1 63BF9942
P 3350 4200
F 0 "R1" H 3420 4246 50  0000 L CNN
F 1 "4.7k" H 3420 4155 50  0000 L CNN
F 2 "Discret:0603" V 3280 4200 50  0001 C CNN
F 3 "~" H 3350 4200 50  0001 C CNN
	1    3350 4200
	1    0    0    -1  
$EndComp
$Comp
L Nano_DD32-rescue:R-Device R2
U 1 1 63BFA29B
P 3650 4200
F 0 "R2" H 3720 4246 50  0000 L CNN
F 1 "4.7k" H 3720 4155 50  0000 L CNN
F 2 "Discret:0603" V 3580 4200 50  0001 C CNN
F 3 "~" H 3650 4200 50  0001 C CNN
	1    3650 4200
	1    0    0    -1  
$EndComp
$Comp
L Nano_DD32-rescue:R-Device R7
U 1 1 63BFF61D
P 9900 1550
F 0 "R7" V 9693 1550 50  0000 C CNN
F 1 "4.7k" V 9784 1550 50  0000 C CNN
F 2 "Discret:0603" V 9830 1550 50  0001 C CNN
F 3 "~" H 9900 1550 50  0001 C CNN
	1    9900 1550
	0    1    1    0   
$EndComp
Wire Wire Line
	8100 3650 8100 3750
Text Label 7050 2150 0    50   ~ 0
D9-SDA
Text Label 7050 2550 0    50   ~ 0
D10-SCL
$Comp
L MCU_Microchip_AVR:AVR64DD32 U2
U 1 1 63AFCA8E
P 7700 2550
F 0 "U2" H 8350 2250 50  0000 L CNN
F 1 "AVR64DD32" H 8200 2150 50  0000 L CNN
F 2 "Package_DFN_QFN:QFN-32-1EP_5x5mm_P0.5mm_EP3.1x3.1mm_pin0" H 8300 2100 50  0001 C CNN
F 3 "" H 8300 2100 50  0001 C CNN
	1    7700 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7600 3150 7050 3150
Text Label 7050 3050 0    50   ~ 0
PC0-TX1
Text Label 7050 3150 0    50   ~ 0
RX1
Wire Wire Line
	7600 2950 7050 2950
Text Label 7100 3650 0    50   ~ 0
CCL1-OUT
Wire Wire Line
	6850 3050 7600 3050
Text Label 6200 3050 0    50   ~ 0
TX1
$Comp
L Nano_DD32-rescue:R-Device R6
U 1 1 63B64AAB
P 9600 4900
F 0 "R6" V 9393 4900 50  0000 C CNN
F 1 "4.7k" V 9484 4900 50  0000 C CNN
F 2 "Discret:0603" V 9530 4900 50  0001 C CNN
F 3 "~" H 9600 4900 50  0001 C CNN
	1    9600 4900
	0    1    1    0   
$EndComp
$Comp
L Nano_DD32-rescue:LED-Device D1
U 1 1 63B64D36
P 10100 4900
F 0 "D1" H 10093 5117 50  0000 C CNN
F 1 "GRN" H 10093 5026 50  0000 C CNN
F 2 "Discret:0603_LED" H 10100 4900 50  0001 C CNN
F 3 "~" H 10100 4900 50  0001 C CNN
	1    10100 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 4900 10250 4900
Wire Wire Line
	9950 4900 9750 4900
Text Label 8600 5200 1    50   ~ 0
~LED-G
Wire Wire Line
	7600 2550 7050 2550
Wire Wire Line
	8000 2150 7050 2150
Text Label 2400 1800 0    50   ~ 0
D10-SCL
Text Label 8500 4350 1    50   ~ 0
BOOT-SDA
Text Label 8400 4350 1    50   ~ 0
BOOT-SCL
Wire Wire Line
	3650 4850 4200 4850
$Comp
L power:COM #PWR06
U 1 1 63B8E34F
P 4100 4250
F 0 "#PWR06" H 4100 4050 50  0001 C CNN
F 1 "COM" H 4100 4150 50  0000 C CNN
F 2 "" H 4100 4250 50  0001 C CNN
F 3 "" H 4100 4250 50  0001 C CNN
	1    4100 4250
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR08
U 1 1 63B8E03C
P 5150 3600
F 0 "#PWR08" H 5150 3450 50  0001 C CNN
F 1 "+5V" H 5165 3773 50  0000 C CNN
F 2 "" H 5150 3600 50  0001 C CNN
F 3 "" H 5150 3600 50  0001 C CNN
	1    5150 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 4750 4300 4750
Wire Wire Line
	4300 4150 4300 4750
Connection ~ 4300 4750
Wire Wire Line
	4300 4750 4800 4750
Wire Wire Line
	4200 4150 4200 4850
Connection ~ 4200 4850
Wire Wire Line
	4200 4850 4800 4850
$Comp
L conn:CONN_01X05 P3
U 1 1 63E25171
P 4500 4050
F 0 "P3" V 4650 3850 50  0000 R CNN
F 1 "CONN_01X05" V 4650 3850 50  0001 C CNN
F 2 "" H 4800 4150 50  0000 C CNN
F 3 "" H 4800 4150 50  0000 C CNN
	1    4500 4050
	0    1    -1   0   
$EndComp
Wire Wire Line
	5150 4250 5150 3650
Text Label 5000 4150 2    50   ~ 0
~RESET
Wire Wire Line
	4100 4150 4100 4250
Wire Wire Line
	4400 4150 4400 4250
Wire Wire Line
	4400 4250 5150 4250
Wire Wire Line
	4500 4150 4700 4150
$Comp
L Nano_DD32-rescue:R-Device R3
U 1 1 63E6D911
P 4700 3850
F 0 "R3" H 4770 3896 50  0000 L CNN
F 1 "4.7k" H 4770 3805 50  0000 L CNN
F 2 "Discret:0603" V 4630 3850 50  0001 C CNN
F 3 "~" H 4700 3850 50  0001 C CNN
	1    4700 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 4000 4700 4150
Connection ~ 4700 4150
Wire Wire Line
	4700 4150 5000 4150
Wire Wire Line
	4700 3700 4700 3650
Wire Wire Line
	4700 3650 5150 3650
Connection ~ 5150 3650
Wire Wire Line
	5150 3650 5150 3600
$Comp
L conn:CONN_01X04 P4
U 1 1 63E83A86
P 5200 1550
F 0 "P4" V 5350 1400 50  0000 L CNN
F 1 "CONN_01X04" V 5350 1400 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 5200 1550 50  0001 C CNN
F 3 "" H 5200 1550 50  0000 C CNN
	1    5200 1550
	0    1    -1   0   
$EndComp
Wire Wire Line
	5000 1650 5000 1900
Wire Wire Line
	5200 1650 5200 1700
Wire Wire Line
	5200 1700 5800 1700
Wire Wire Line
	5100 1650 5100 1800
Wire Wire Line
	5100 1800 5450 1800
Wire Wire Line
	5450 1800 5450 1600
Wire Wire Line
	4900 1650 4900 2200
Wire Wire Line
	4900 2200 5800 2200
Text Notes 4850 1250 0    50   ~ 10
PROG/DBG
Text Notes 4200 3800 0    50   ~ 10
MON
Wire Wire Line
	1850 4750 1850 4950
Wire Wire Line
	1950 4850 1650 4850
Wire Wire Line
	1650 4850 1650 3950
Wire Wire Line
	1650 3950 3150 3950
Wire Wire Line
	3150 3950 3150 4650
Wire Wire Line
	7050 3250 7600 3250
Wire Wire Line
	8200 3650 8200 4350
Wire Wire Line
	8600 3650 8600 4900
Wire Wire Line
	8600 4900 9450 4900
Wire Wire Line
	9100 2550 9600 2550
Wire Wire Line
	6850 3650 8000 3650
Wire Wire Line
	6550 3650 6500 3650
Wire Wire Line
	6500 3650 6500 3050
Wire Wire Line
	6500 3050 6550 3050
Wire Wire Line
	6500 3050 6200 3050
Connection ~ 6500 3050
$Comp
L Nano_DD32-rescue:R-Device R4
U 1 1 63EE6812
P 6700 3050
F 0 "R4" V 6493 3050 50  0000 C CNN
F 1 "1k" V 6584 3050 50  0000 C CNN
F 2 "Discret:0603" V 6630 3050 50  0001 C CNN
F 3 "~" H 6700 3050 50  0001 C CNN
	1    6700 3050
	0    1    1    0   
$EndComp
$Comp
L Nano_DD32-rescue:R-Device R5
U 1 1 63EE70E1
P 6700 3650
F 0 "R5" V 6493 3650 50  0000 C CNN
F 1 "1k" V 6584 3650 50  0000 C CNN
F 2 "Discret:0603" V 6630 3650 50  0001 C CNN
F 3 "~" H 6700 3650 50  0001 C CNN
	1    6700 3650
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR013
U 1 1 63EEDA0F
P 10400 4850
F 0 "#PWR013" H 10400 4700 50  0001 C CNN
F 1 "+5V" H 10415 5023 50  0000 C CNN
F 2 "" H 10400 4850 50  0001 C CNN
F 3 "" H 10400 4850 50  0001 C CNN
	1    10400 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 4850 10400 4900
Wire Wire Line
	8850 3650 8850 4250
Wire Wire Line
	8850 4250 9150 4250
$Comp
L conn:CONN_01X15 P2
U 1 1 64C68BF5
P 3100 2100
F 0 "P2" H 3050 2950 50  0000 L CNN
F 1 "CONN_01X15" H 3178 2050 50  0001 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x15_P2.54mm_Vertical" H 3100 2100 50  0001 C CNN
F 3 "" H 3100 2100 50  0000 C CNN
	1    3100 2100
	1    0    0    -1  
$EndComp
$Comp
L conn:CONN_01X15 P1
U 1 1 64C6AA02
P 1400 2100
F 0 "P1" H 1400 2900 50  0000 C CNN
F 1 "CONN_01X15" H 1317 2924 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x15_P2.54mm_Vertical" H 1400 2100 50  0001 C CNN
F 3 "" H 1400 2100 50  0000 C CNN
	1    1400 2100
	-1   0    0    -1  
$EndComp
NoConn ~ 2900 2600
NoConn ~ 2900 2700
Wire Wire Line
	2900 2800 2400 2800
Wire Wire Line
	1600 2600 2050 2600
Text Label 2400 2100 0    50   ~ 0
BOOT-SDA
Text Label 2400 2000 0    50   ~ 0
BOOT-SCL
Wire Wire Line
	8700 1550 8700 1200
Connection ~ 8700 1550
Wire Wire Line
	8600 4900 8600 5200
Connection ~ 8600 4900
Text Label 2050 2500 2    50   ~ 0
~LED-R
Text Label 2050 2600 2    50   ~ 0
~LED-G
Text Notes 2100 2600 0    50   ~ 0
~SS
Wire Wire Line
	1600 2700 2050 2700
Wire Wire Line
	1600 2800 2050 2800
Text Notes 2250 1800 0    50   ~ 0
A7
Text Notes 2250 1900 0    50   ~ 0
A6
Text Notes 2250 2000 0    50   ~ 0
A5
Text Notes 2250 2100 0    50   ~ 0
A4
Text Notes 6900 6250 0    197  ~ 39
Nano DD32 rev 1.0
Text Notes 2400 2600 0    50   ~ 0
Aref
Text Notes 2400 2700 0    50   ~ 0
3V3
Text Notes 6650 3800 0    50   ~ 0
DNP
$EndSCHEMATC
