#include <iostream>
#include <vector>
#include <string.h>     // std::string, memcpy
#include <iomanip>      // std::setw
#include <unistd.h>  //usleep
#include <sstream>  //std::ostringstream

#include "usb2lin06Controler.h"

using namespace std;
/*
 * just some universal device descripor print out
 */
void printDescriptor(struct libusb_device_handle *udev)
{
  struct libusb_device * dev = libusb_get_device(udev);

  struct libusb_device_descriptor des;
  libusb_get_device_descriptor(dev,&des);

  struct keyName { std::string name; unsigned int value; std::string desc; };
  std::vector<keyName> data =
  {
    {        "bLength",              des.bLength,            "Size of the Descriptor in Bytes (18 bytes)"},
    {        "bDescriptorType",      des.bDescriptorType,    "Constant        Device Descriptor (0x01)"},
    {        "bcdUSB",               des.bcdUSB,             "USB Specification Number which device complies too."},
    {        "bDeviceClass",         des.bDeviceClass,       "Class   Class Code (Assigned by USB Org)"},
    {        "bDeviceSubClass",      des.bDeviceSubClass,    "SubClass        Subclass Code (Assigned by USB Org)"},
    {        "bDeviceProtocol",      des.bDeviceProtocol,    "Protocol        Protocol Code (Assigned by USB Org)"},
    {        "bMaxPacketSize0",      des.bMaxPacketSize0,    "Number  Maximum Packet Size for Zero Endpoint."},
    {        "idVendor",             des.idVendor,           "ID      Vendor ID (Assigned by USB Org)"},
    {        "idProduct",            des.idProduct,          "ID      Product ID (Assigned by Manufacturer)"},
    {        "bcdDevice",            des.bcdDevice,          "BCD     Device Release Number"},
    {        "iManufacturer",        des.iManufacturer,      "Index   Index of Manufacturer String Descriptor"},
    {        "iProduct",             des.iProduct,           "Index   Index of Product String Descriptor"},
    {        "iSerialNumber",        des.iSerialNumber,      "Index   Index of Serial Number String Descriptor"},
    {        "bNumConfigurations: ", des.bNumConfigurations, "1       Integer Number of Possible Configurations"}
  };

  cout<<"Descriptor (values in HEX)"<<endl;
  for ( auto d:data)
  {
    cout<<std::setw(20)<<d.name<<":"
      <<setw(10)<<hex<<d.value<<" "
      <<d.desc<<endl;
  }
}

const char SeparatorFields=',';
const char SeparatorStart='{';
const char SeparatorEnd='}';
std::ostream& operator << (std::ostream &o, const usb2lin06::LINIDvalidFlag &a)
{
  o<<"flags:" <<SeparatorStart
  <<dec<<"0b"
  //note: struct reversed
  <<(int)a.unknown
  <<(int)a.ID07_Ref8_pos_stat_speed
  <<(int)a.ID06_Ref7_pos_stat_speed
  <<(int)a.ID38_Handset2command
  <<(int)a.ID37_Handset1command
  <<(int)a.ID05_Ref6_pos_stat_speed
  <<(int)a.ID28_Diagnostic
  <<(int)a.ID04_Ref5_pos_stat_speed
  <<(int)a.ID13_Ref4_controlInput
  <<(int)a.ID12_Ref3_controlInput
  <<(int)a.ID11_Ref2_controlInput
  <<(int)a.ID10_Ref1_controlInput
  <<(int)a.ID03_Ref4_pos_stat_speed
  <<(int)a.ID02_Ref3_pos_stat_speed
  <<(int)a.ID01_Ref2_pos_stat_speed
  <<(int)a.ID00_Ref1_pos_stat_speed
  <<SeparatorEnd;
}

std::ostream& operator << (std::ostream &o, const usb2lin06::Status &a)
{
  o<<"status:"<<SeparatorStart
  <<dec
  <<(bool)a.positionLost << (bool)a.antiColision << (bool)a.overloadDown << (bool)a.overloadUp
  <<SeparatorEnd;
}

std::ostream& operator << (std::ostream &o, const usb2lin06::RefPosStatSpeed &a)
{
  o<<"posStatSpeed:" <<SeparatorStart
  <<hex<<setfill('0')
  <<hex<<"0x"<<setw(4)<<(short)a.pos   <<SeparatorFields
  <<                           a.status<<SeparatorFields
  <<hex<<"0x"<<setw(4)<<(short)a.speed <<SeparatorEnd;
}

std::ostream& operator << (std::ostream &o, const usb2lin06::Diagnostic &a)
{
  o<<"diag:"   <<SeparatorStart
  <<hex<<"0x"<<setw(4)<<(short)a.type     <<SeparatorFields
  <<dec
  <<(int)a.event[0] <<SeparatorFields
  <<(int)a.event[1] <<SeparatorFields
  <<(int)a.event[2] <<SeparatorFields
  <<(int)a.event[3] <<SeparatorFields
  <<(int)a.event[4] <<SeparatorFields
  <<(int)a.event[5] <<SeparatorEnd;
}

std::ostream& operator << (std::ostream &o, const usb2lin06::StatusReport &a) {

  auto handsetToStr = [](const uint16_t &h)-> const char*
  {
    switch(h)
    {
    case 0xffff: return "--";
    case 0x0047: return "B1";
    case 0x0046: return "B2";
    case 0x000e: return "B3";
    case 0x000f: return "B4";
    case 0x000c: return "B5";
    case 0x000d: return "B6";
    }
    return "??";
  };

  o<<"StatusReport:"<<SeparatorStart
  <<hex<<setfill('0')
//  <<"ID:"  << SeparatorStart <<setw(2)<<(int)a.featureRaportID <<SeparatorEnd<<SeparatorFields
//  <<"bytes:"<<SeparatorStart <<setw(2)<<(int)a.numberOfBytes   <<SeparatorEnd <<SeparatorFields
  <<a.validFlag;
  if(a.validFlag.ID00_Ref1_pos_stat_speed) o<<SeparatorFields<<"ref1"<< a.ref1;
  if(a.validFlag.ID01_Ref2_pos_stat_speed) o<<SeparatorFields<<"ref2"<< a.ref2;
  if(a.validFlag.ID02_Ref3_pos_stat_speed) o<<SeparatorFields<<"ref3"<< a.ref3;
  if(a.validFlag.ID03_Ref4_pos_stat_speed) o<<SeparatorFields<<"ref4"<< a.ref4;
  if(a.validFlag.ID04_Ref5_pos_stat_speed) o<<SeparatorFields<<"ref5"<< a.ref5;
  if(a.validFlag.ID05_Ref6_pos_stat_speed) o<<SeparatorFields<<"ref6"<< a.ref6;
  if(a.validFlag.ID06_Ref7_pos_stat_speed) o<<SeparatorFields<<"ref7"<< a.ref7;
  if(a.validFlag.ID07_Ref8_pos_stat_speed) o<<SeparatorFields<<"ref8"<< a.ref8;
  if(a.validFlag.ID10_Ref1_controlInput)   o<<SeparatorFields<<"ref1ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref1cnt)  <<SeparatorEnd;
  if(a.validFlag.ID11_Ref2_controlInput)   o<<SeparatorFields<<"ref2ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref2cnt)  <<SeparatorEnd;
  if(a.validFlag.ID12_Ref3_controlInput)   o<<SeparatorFields<<"ref3ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref3cnt)  <<SeparatorEnd;
  if(a.validFlag.ID13_Ref4_controlInput)   o<<SeparatorFields<<"ref4ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref4cnt)  <<SeparatorEnd;
  if(a.validFlag.ID28_Diagnostic)  o<<a.diagnostic <<SeparatorFields;
//  <<"undefined1:"   <<SeparatorStart <<(int)a.undefined1[0] <<(int)a.undefined1[1] <<SeparatorEnd  <<SeparatorFields
  if(a.validFlag.ID37_Handset1command) o<<SeparatorFields<<"handset1:"<< SeparatorStart << handsetToStr(a.handset1) <<SeparatorEnd;
  if(a.validFlag.ID38_Handset2command) o<<SeparatorFields<<"handset2:"<< SeparatorStart << handsetToStr(a.handset2) <<SeparatorEnd;
//  <<"undefined2:"   <<SeparatorStart <<(int)a.undefined2[0] <<(int)a.undefined2[1] <<(int)a.undefined2[2] <<(int)a.undefined2[3] <<(int)a.undefined2[4] <<(int)a.undefined2[5] <<SeparatorEnd

  o<<SeparatorEnd;
}

void printStatusReport(const usb2lin06::StatusReport &report)
{
  if(usb2lin06::isStatusReportNotReady(report))
    cerr<<"ERROR: statusReport -> device not ready"<<endl;
  else
    cout<<report<<endl;
}

/*
 * just an ugly way to get time in format HH:MM:SS:MS
 */
std::string getPreciseTime()
{
  std::ostringstream stm ;

  std::time_t t = std::time(NULL);
  timeval curTime;
  gettimeofday(&curTime, NULL);
  int milli = curTime.tv_usec / 1000;

  char mbstr[100]; mbstr[0]='\0';
  if (std::strftime(mbstr, sizeof(mbstr), "%0H:%0M:%0S", std::localtime(&t)))
  {
    stm << mbstr <<":"<<setfill('0')<<setw(3)<<milli;
  }

  return stm.str() ;
}

int main (int argc,char **argv)
{
  int SETTINGS_COUNT = 1000;//how may times should i get status
  unsigned int SETTINGS_CommadDelay = 1000000;//delay between sending commands [in micro Sec]

  DEBUGOUT("main() - get args");
  {
    if(argc>1)
    {
      SETTINGS_COUNT=::atoi(argv[1]);

      if(SETTINGS_COUNT<0 || SETTINGS_COUNT > 99999)
      {
        cerr<<"ERROR: arg1 - remetitions must be an integer in range <0,99999>"<<endl;
        return -1;
      }
    }

    if(argc==3)
    {
      float delaySec = -9.9f;
      delaySec=::atof(argv[2]);

      if(delaySec<0.1f and delaySec>10.0f)
      {
        cerr<<"ERROR: arg2 - time [sec] must get greater than 0.1 and less than 10 sec seconds"<<endl;
        return -1;
      }

      SETTINGS_CommadDelay = 1000000*delaySec;
    }
  }

  DEBUGOUT("main() - init");
  usb2lin06::usb2lin06Controler controler(false);

  DEBUGOUT("main() - print some device info");
  {
    printDescriptor(controler.udev);

    unsigned char buf[50];
    for(int i : {1,2})
    {
      int ret = libusb_get_string_descriptor_ascii(controler.udev, i, buf, sizeof(buf));
      if(ret<0)  { cerr<<"ERROR: to read device decriptor string"<<i<<" err"<<ret<<endl;}
      else       { cout<<"line "<<i<<": '"<<buf<<"'"<<endl; }
    }
  }

  DEBUGOUT("main() - check if device it ready");
  {
    if(!controler.getStatusReport())
    {
      cerr<<"ERROR: cant get initial status"<<endl;
      return 1;
    }else{
      if(usb2lin06::isStatusReportNotReady(controler.report)){
        cout<<" device is not ready"<<endl;
        if(!controler.initDevice())
        {
          cerr<<"ERROR: can't init device!"<<endl;
          return 1;
        }
      }else{
        cout<<" device is ready"<<endl;
      }
    }
  }

  DEBUGOUT("main() - getting status SETTINGS_COUNT times");
  {
    std::string sCOUNT = std::to_string(SETTINGS_COUNT);

    unsigned int i=1;
    while(true)
    {
      if(controler.getStatusReport())
      {      
          cout<<getPreciseTime()<<dec<<" ["<<setfill('0')<<setw(sCOUNT.length())<<i<<"/"<<sCOUNT<<"] ";
          printStatusReport(controler.report);        
      }

      if(i++==SETTINGS_COUNT) break;
      usleep(SETTINGS_CommadDelay);
    }
  }

  DEBUGOUT("main() - end");
  {
    cout<<"DONE"<<endl;
  }
  return 0;
}
