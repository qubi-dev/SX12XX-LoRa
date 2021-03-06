/*******************************************************************************************************
  lora Programs for Arduino - Copyright of the author Stuart Robinson - 03/02/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation -  This program is an example of a basic portable GPS tracker receiver. The program
  receives the location packets from the remote tracker and displays them on an OLED display. The program
  also reads a local GPS and when that has a fix, will display the distance and direction to the remote
  tracker.

  The program writes direct to the LoRa devices internal buffer, no memory buffer is used.

  The LoRa settings are configured in the Settings.h file.

  The received information is printed to screen in this order top to bottom;

  Latitude, Longitude, Altitude, HDOP, GPS Fixtime, Tracker battery mV, Number of received packets, Distance
  and direction to tracker, if local GPS fix. In addition if there is a recent tracker transmitter GPS fix
  a 'T' is shown on line 0 right of screen and if there is a recent local (receiver) GPS fix a 'R' is displayed
  line 1 right of screen.

  The received information is printed to the Serial Monitor as CSV data in this order;

  Packet Address information, Latitude, Longitude, Altitude, Satellites in use, HDOP, TX status byte,
  GPS Fixtime, Tracker battery mV, Number of received packets, Distance and direction to tracker, if local
  GPS fix.

  The program has the option of using a pin to control the power to the GPS, if the GPS module being used
  has this feature. To use the option change the define in Settings.h; '#define GPSPOWER -1' from -1 to
  the pin number being used. Also set the GPSONSTATE and GPSOFFSTATE to the appropriate logic levels.

  The program also has the option of using a logic pin to control the power to the lora and SD card
  devices, which can save power in sleep mode. If the hardware is fitted to your board these devices are
  powered on by setting the VCCPOWER pin low. If your board does not have this feature set VCCPOWER to -1.

  The program by default uses software serial to read the GPS, you can use hardware serial by commenting
  out this line in the Settings.h file;

  #define USE_SOFTSERIAL_GPS

  And then defining the hardware serial port you are using, which defaults to Serial1.

  Serial monitor baud rate is set at 9600.
*******************************************************************************************************/

#define Program_Version "V1.0"

#include <SPI.h>
#include <SX127XLT.h>
SX127XLT LT;

#include "Settings.h"
#include <ProgramLT_Definitions.h>

#include <U8x8lib.h>                                        //https://github.com/olikraus/u8g2 
U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);      //standard 0.96" SSD1306
//U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);     //1.3" OLED often sold as 1.3" SSD1306


#include <TinyGPS++.h>           //http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                 //create the TinyGPS++ object


uint32_t RXpacketCount;          //count of received packets
uint8_t RXPacketL;               //length of received packet
int8_t  PacketRSSI;              //signal strength (RSSI) dBm of received packet
int8_t  PacketSNR;               //signal to noise ratio (SNR) dB of received packet
uint8_t PacketType;              //for packet addressing, identifies packet type
uint8_t Destination;             //for packet addressing, identifies the destination (receiving) node
uint8_t Source;                  //for packet addressing, identifies the source (transmiting) node
uint8_t TXStatus;                //status byte from tracker transmitter
uint8_t TXSats;                  //number of sattelites in use
float TXLat;                     //latitude
float TXLon;                     //longitude
float TXAlt;                     //altitude
float RXLat;                     //latitude
float RXLon;                     //longitude
float RXAlt;                     //altitude
uint32_t TXHdop;                 //HDOP, indication of fix quality, horizontal dilution of precision, low is good
uint32_t TXGPSFixTime;           //time in mS for fix
uint16_t TXVolts;                //supply\battery voltage
uint16_t RXVolts;                //supply\battery voltage
float TXdistance;                //calculated distance to tracker
uint16_t TXdirection;            //calculated direction to tracker
uint16_t RXerrors;

uint32_t LastRXGPSfixCheck;      //used to record the time of the last GPS fix

bool TXLocation;                 //set to true when a tracker location packet has been received
bool RXGPSfix;                   //set tot true if the local GPS has a recent fix

uint8_t FixCount = DisplayRate;  //used to keep track of number of GPS fixes before display updated


void loop()
{
  RXPacketL = LT.receiveSXBuffer(0, 0, NO_WAIT);   //returns 0 if packet error of some sort

  while (!digitalRead(DIO0))
  {
    readGPS();                                     //If the DIO pin is low, no packet arrived, so read the GPS
  }

  //something has happened

  digitalWrite(LED1, HIGH);

  if (BUZZER > 0)
  {
    digitalWrite(BUZZER, HIGH);
  }

  RXPacketL = LT.readRXPacketL();
  PacketRSSI = LT.readPacketRSSI();
  PacketSNR = LT.readPacketSNR();

  if (RXPacketL == 0)
  {
    packet_is_Error();
  }
  else
  {
    packet_is_OK();
  }

  digitalWrite(LED1, LOW);

  if (BUZZER > 0)
  {
    digitalWrite(BUZZER, LOW);
  }
  Serial.println();
}


void readGPS()
{
  if (GPSserial.available() > 0)
  {
    gps.encode(GPSserial.read());
  }


  if ( millis() > (LastRXGPSfixCheck + NoRXGPSfixms))
  {
    RXGPSfix = false;
    LastRXGPSfixCheck = millis();
    dispscreen1();
  }


  if (gps.location.isUpdated() && gps.altitude.isUpdated())
  {

    RXGPSfix = true;
    RXLat = gps.location.lat();
    RXLon = gps.location.lng();
    RXAlt = gps.altitude.meters();
    printRXLocation();
    LastRXGPSfixCheck = millis();

    if ( FixCount == 1)                           //update screen when FIXcoount counts down from DisplayRate to 1
    {
      FixCount = DisplayRate;
      dispscreen1();
    }
    FixCount--;
  }
}


bool readTXStatus(byte bitnum)
{
  return bitRead(TXStatus, bitnum);
}


void printRXLocation()
{
  Serial.print(F("Local GPS "));
  Serial.print(RXLat, 5);
  Serial.print(F(","));
  Serial.print(RXLon, 5);
  Serial.print(F(","));
  Serial.print(RXAlt, 1);
  Serial.println();
}


void readPacketAddressing()
{
  LT.startReadSXBuffer(0);
  PacketType = LT.readUint8();
  Destination = LT.readUint8();
  Source = LT.readUint8();
  LT.endReadSXBuffer();
}


void packet_is_OK()
{
  uint16_t IRQStatus;
  float tempfloat;

  RXpacketCount++;

  readPacketAddressing();

  if (PacketType == PowerUp)
  {
    LT.startReadSXBuffer(0);
    LT.readUint8();                                      //read byte from SXBuffer, not used
    LT.readUint8();                                      //read byte from SXBuffer, not used
    LT.readUint8();                                      //read byte from SXBuffer, not used
    TXVolts = LT.readUint16();                           //read tracker transmitter voltage
    LT.endReadSXBuffer();
    Serial.print(F("Tracker Powerup - Battery "));
    Serial.print(TXVolts);
    Serial.println(F("mV"));
    dispscreen2();
  }

  if (PacketType == LocationBinaryPacket)
  {
    //packet has been received, now read from the SX1280 FIFO in the correct order.
    TXLocation = true;
    LT.startReadSXBuffer(0);                //start the read of received packet
    PacketType = LT.readUint8();            //read in the PacketType
    Destination = LT.readUint8();           //read in the Packet destination address
    Source = LT.readUint8();                //read in the Packet source address
    TXLat = LT.readFloat();                 //read in the tracker latitude
    TXLon = LT.readFloat();                 //read in the tracker longitude
    TXAlt = LT.readFloat();                 //read in the tracker altitude
    TXSats = LT.readUint8();                //read in the satellites in use by tracker GPS
    TXHdop = LT.readUint32();               //read in the HDOP of tracker GPS
    TXStatus = LT.readUint8();              //read in the tracker status byte
    TXGPSFixTime = LT.readUint32();         //read in the last fix time of tracker GPS
    TXVolts = LT.readUint16();              //read in the tracker supply\battery volts
    RXPacketL = LT.endReadSXBuffer();       //end the read of received packet


    if (RXGPSfix)                           //if there has been a local GPS fic do the distance and direction calculation
    {
      TXdirection = (int) TinyGPSPlus::courseTo(RXLat, RXLon, TXLat, TXLon);
      TXdistance = TinyGPSPlus::distanceBetween(RXLat, RXLon, TXLat, TXLon);
    }
    else
    {
      TXdistance = 0;
      TXdirection = 0;
    }

    //the serial monitor printout is in CSV format so that the output can be logged
    //to a file for later analysis

    Serial.write(PacketType);
    Serial.write(Destination);
    Serial.write(Source);
    Serial.print(F(","));
    Serial.print(TXLat, 5);
    Serial.print(F(","));
    Serial.print(TXLon, 5);
    Serial.print(F(","));
    Serial.print(TXAlt, 1);
    Serial.print(F(","));
    Serial.print(TXSats);
    Serial.print(F(","));

    tempfloat = ( (float) TXHdop / 100);           //need to convert Hdop read from GPS as uint32_t to a float for display
    Serial.print(tempfloat, 2);

    Serial.print(F(","));
    Serial.print(TXStatus);
    Serial.print(F(","));

    tempfloat = ((float) TXGPSFixTime) / 1000;
    Serial.print(tempfloat, 2);

    tempfloat = ((float) TXVolts / 1000);
    disp.print(tempfloat, 2);
    Serial.print(F(","));
    Serial.print(TXdistance, 0);
    Serial.print(F("m,"));
    Serial.print(TXdirection);
    Serial.print(F("d,RSSI,"));
    Serial.print(PacketRSSI);
    Serial.print(F("dBm,SNR,"));
    Serial.print(PacketSNR);
    Serial.print(F("dB,Packets,"));
    Serial.print(RXpacketCount);

    Serial.print(F(",Length,"));
    Serial.print(RXPacketL);
    IRQStatus = LT.readIrqStatus();
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);
    dispscreen1();                                  //and show the packet detail it on screen
    return;
  }
}


void packet_is_Error()
{
  uint16_t IRQStatus;

  if (BUZZER >= 0)
  {
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
  }

  IRQStatus = LT.readIrqStatus();                    //get the IRQ status
  RXerrors++;
  Serial.print(F("PacketError,RSSI"));

  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);

  Serial.print(F("dB,Length,"));
  Serial.print(LT.readRXPacketL());                  //get the real packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
  LT.printIrqStatus();
  digitalWrite(LED1, LOW);

  if (BUZZER >= 0)
  {
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  unsigned int index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);
    delay(delaymS);
    digitalWrite(LED1, LOW);
    delay(delaymS);
  }
}


void dispscreen1()
{
  //show received packet data on display
  float tempfloat;
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(TXLat, 5);
  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(TXLon, 5);
  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(TXAlt);
  disp.print(F("m"));
  disp.clearLine(3);
  disp.setCursor(0, 3);

  tempfloat = ( (float) TXHdop / 100);           //need to convert Hdop read from GPS as uint32_t to a float for display

  disp.print(F("HDOP "));
  disp.print(tempfloat);
  disp.clearLine(4);
  disp.setCursor(0, 4);

  tempfloat = ((float) TXGPSFixTime) / 1000;
  disp.print(F("Fix "));
  disp.print(tempfloat, 2);
  disp.print(F("s"));

  disp.clearLine(5);
  disp.setCursor(0, 5);

  tempfloat = ((float) TXVolts / 1000);
  disp.print(F("Batt "));
  disp.print(tempfloat, 2);

  disp.print(F("v"));
  disp.clearLine(6);
  disp.setCursor(0, 6);
  disp.print(F("Packets "));
  disp.print(RXpacketCount);

  if (RXGPSfix && TXLocation)           //only display distance and direction if have received tracker packet and have local GPS fix
  {
    disp.clearLine(7);
    disp.setCursor(0, 7);
    disp.print(TXdistance, 0);
    disp.print(F("m "));
    disp.print(TXdirection);
    disp.print(F("d"));
    disp.setCursor(15, 1);
    disp.print(F("R"));
  }
  else
  {
    disp.clearLine(7);
    disp.setCursor(0, 7);
    disp.print(F("No Local Fix"));
  }

  if (readTXStatus(GPSFix))
  {
    disp.setCursor(15, 0);
    disp.print(F("T"));
  }

}


void dispscreen2()
{
  //show tracker powerup data on display
  float tempfloat;
  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("Tracker Powerup"));
  disp.setCursor(0, 1);
  disp.print(F("Battery "));
  tempfloat = ((float) TXVolts / 1000);
  disp.print(tempfloat, 2);
  disp.print(F("v"));
}


void GPSON()
{
  if (GPSPOWER)
  {
    digitalWrite(GPSPOWER, GPSONSTATE);                         //power up GPS
  }
}


void GPSOFF()
{
  if (GPSPOWER)
  {
    digitalWrite(GPSPOWER, GPSOFFSTATE);                        //power off GPS
  }
}


void setup()
{
  uint32_t endmS;

  if (VCCPOWER >= 0)
  {
    pinMode(VCCPOWER, OUTPUT);                  //For controlling power to external devices
    digitalWrite(VCCPOWER, LOW);                //VCCOUT on. lora device on
  }

  if (GPSPOWER >= 0)
  {
    pinMode(GPSPOWER, OUTPUT);
    GPSON();
  }

  pinMode(LED1, OUTPUT);                        //setup pin as output for indicator LED
  led_Flash(2, 125);                            //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.print(F(__TIME__));
  Serial.print(F(" "));
  Serial.println(F(__DATE__));
  Serial.println(F(Program_Version));
  Serial.println();

  Serial.println(F("25_GPS_Tracker_Receiver_With_Display_and_GPS Starting"));

  if (BUZZER >= 0)
  {
    pinMode(BUZZER, OUTPUT);
  }

  SPI.begin();

  disp.begin();
  disp.setFont(u8x8_font_chroma48medium8_r);

  Serial.print(F("Checking LoRa device - "));         //Initialize LoRa
  disp.setCursor(0, 0);

  if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
  {
    Serial.println(F("Receiver ready"));
    disp.print(F("Receiver ready"));
    led_Flash(2, 125);
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    disp.print(F("No LoRa device"));
    while (1)
    {
      led_Flash(50, 50);                                            //long fast speed flash indicates device error
    }
  }

  LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println();
  Serial.println(F("Startup GPS check"));

  endmS = millis() + echomS;

  GPSserial.begin(GPSBaud);

  while (millis() < endmS)
  {
    while (GPSserial.available() > 0)
      Serial.write(GPSserial.read());
  }
  Serial.println();
  Serial.println();

  Serial.println(F("Receiver ready"));
  Serial.println();
  RXGPSfix = false;
  TXLocation = false;
}



