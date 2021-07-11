/*
   _____________________________________________________________________________________________________________________________________________
   Vectrex nvSRAM Loader
   
   This program sends a ROM file to the nvSRAM Loader
   The Loader then programs the nvSRAM Cart
   Usage:
      nvSRAM.exe [filename], [COM port]   (sends a ROM file to the nvSRAM Loader)
      nvSRAM.exe read, [COM port]         (reads from the nvSRAM cart and compares to user entered ROM file)
      nvSRAM.exe test, [COM port]         (performs a pin test of an unpolulated nvSRAM cart, used to test the prototype)

   Make sure that the nvSRAM cart is facing the proper direction and properly seated in the edge connector (nvSRAM chip faces PSOC board)
   Unplug unplug the PSOC board's usb connector before removing the nvSRAM Cart
   Do not use the reset switch on the PSOC board (this corrupts nvSRAM Cart data)

   Some docs I found useful while developing this program:
       // https://docs.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
      // https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
      // https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-dcb
      // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcommtimeouts
   
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
   ______________________________________________________________________________________________________________________________________________
*/


#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>

void delay(int number_of_seconds);
void test_pins();
void end_program();
void send_data(unsigned char thedata);
void receive_data();
void compare_file();


HANDLE hComm;           // Handle for the Com Port as returned by CreateFile()
DCB dcb;                // The DCB struct for com port settings
BOOL fSuccess;          // 
FILE *fptr;             // Pointer for the .bin file
char inputfile[255];    // array that holds the .bin file name
char mystring[255];     // array that holds work strings
char comport[20];      // array that holds the name of the com port
char lpBuffer[255];
char lpBuffer2[255];
DWORD dNoOfBytesWritten;      // number of bytes that were written to Serial Port
DWORD lpNumberOfBytesRead;    // number of bytes read from the Serial Port


int main(int argc, char *argv[]) {

   unsigned int sizefile = 0;       // size in bytes of the .bin file
   const char magicinit[5] = {'g', ' ', 'G', 'C', 'E'};
   int x = 0;
   int z = 0;
   unsigned char y = 0;
   int a = 0;

   //check for serial port presence
   if(argc > 2) {
      strcpy(comport, "\\\\.\\");
      strcat(comport, argv[2]);
   }
   else 
   {
      printf("%s", "\nnvSRAM Loader");
      printf("%s", "\nTo load a file: nvSRAM.exe [Filename], [COM]\n");
      printf("%s", "\nTo check a file against nvSRAM memory: nvSRAM.exe read, [COM]\n");
      printf("%s", "Program will ask user to input filename of .bin to test.\n");
      printf("%s", "\nTo run a pin test: nvSRAM.exe test, [COM]");
      return 0;
   }
   hComm = CreateFileA(comport,                //port name
                      GENERIC_READ | GENERIC_WRITE, //Read/Write
                      0,                            // No Sharing
                      NULL,                         // No Security
                      OPEN_EXISTING,// Open existing port only
                      0,            // Non Overlapped I/O
                      NULL);        // Null for Comm Devices

   if (hComm == INVALID_HANDLE_VALUE) {
      printf("%s", "Error in opening serial port\n");
      return 0;
   }
   else
      printf("Opened: %s\n", argv[2]);

   //  Initialize the DCB structure
   SecureZeroMemory(&dcb, sizeof(DCB));   // fill DCB with zeros
   dcb.DCBlength = sizeof(DCB);           // set DCB length for initial use

   // Set up Com Port
   fSuccess = GetCommState(hComm, &dcb);        // Get current com port settings
   if(fSuccess) {
      dcb.BaudRate = CBR_115200;                   // Plug in new settings
      dcb.ByteSize = 8;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      fSuccess = SetCommState(hComm, &dcb);
      if(!fSuccess) {
         printf("%s", "Failed to set COM Port settings");
         CloseHandle(hComm);//Closing the Serial Port
         return 0;
      }
   }
   else {
      printf("%s", "Failed to read COM Port settings\n");
      CloseHandle(hComm);//Closing the Serial Port
      return 0;
   }

    // Check if asking for a pin test, if yes then send 'T', otherwise send 'A'
   strcpy(inputfile, argv[1]);
   if(strcmp(inputfile, "test") == 0)
   {
      lpBuffer[0] = 'T';      // pin test
   }
   if(strcmp(inputfile, "read") == 0)
   {
      lpBuffer[0] = 'R';      // check nvSRAM memory against a .bin file
   }
   else
   {
      lpBuffer[0] = 'P';      // program the nvSRAM loader with a .bin file
   }

   // Send out command to PSOC (blocking)
   for(;;) {
      send_data(lpBuffer[0]);
      while(lpNumberOfBytesRead == 0) 
      {
         receive_data();
      }
      //printf("%c\n", lpBuffer2[0]);
      break;
   }

   // check to see if user included a file name
   if(argc > 1) {
      printf("File name: %s \n", argv[1]);
   }
   else {
      (printf("%s", "no file \n"));
      end_program();
   }
   strcpy(inputfile, argv[1]);
   int len = strlen(inputfile);
   // Check if asking for a pin test
   if(strcmp(inputfile, "test") == 0)
   {
      test_pins();
      end_program();
   }
   if(strcmp(inputfile, "read") == 0)
   {
      compare_file();
      end_program();
   }
   // check for user inputting a valid file name
   if(len > 4) {
      strncpy(mystring, &inputfile[len-4], 4);
      mystring[4] = '\0'; 
      //printf("mystring: %s\n", mystring);
      //printf("end string: %s\n", mystring);
      if(strcmp(mystring, ".bin") == 0){
         printf("%s", "I see it's a .bin file \n");
      }
      else {
      printf("%s", "Doesn't seem to be a .bin file \n");
      end_program();
      }
   }
    else {
      printf("%s", "Not a .bin file");
      end_program();
   }

   // check if file exists
   if ((fptr = fopen(inputfile,"rb")) == NULL){
      printf("File name %s not found\n", inputfile);
      fclose(fptr);
      end_program();
   }
   printf("%s", "File found\n");

   // Get file size in bytes
   fseek(fptr, 0L, SEEK_END);
   sizefile = ftell(fptr);
   rewind(fptr);
   printf("File size in bytes: %d\n", sizefile);
   
   // Check for Magic Init string
   fread(&mystring, 1, 5, fptr);
   for(x = 0; x < 5; x++) {
      if(mystring[x] != magicinit[x]) 
      {
         printf("%s", "Invalid Magic Init String\n");
         end_program();
      }
   }
   rewind(fptr);
   printf("%s", "Valid Vectrex Magic Init String\n");
   
   // Send file size to PSOC
   //printf("%s\n", "Filesize (HEX): ");
   y = (unsigned char)(sizefile >> 8);
   send_data(y);                                         // send MSB to PSOC
    y = (unsigned char)sizefile;
   send_data(y);                                         // send LSB to PSOC
   receive_data();                                       // get echo from PSOC
   //printf("%02X", (unsigned char)lpBuffer2[0]);
   receive_data();
   //printf("%02X", (unsigned char)lpBuffer2[0]);
   //printf("%\n");

   // Send ROM file to PSOC
   printf("%s", "Programming  ");
   y = 0;
   for(x=0; x<sizefile; x++)
   {
      fread(&mystring, 1, 1, fptr);                      // get byte from file and send it to PSOC
      send_data(mystring[0]);
      receive_data();                                    // get echo back from PSOC
      // Show twirly thing as user waits
      a += 1;
      if(a == 256 || a == 256*4)
      {
         printf("%s", "\b|");
      }
      if(a == 256*2)
      {
         printf("%s", "\b/");
      }
      if(a == 256*3  || a == 256*5)
      {
         printf("%s", "\b-");
      }
      if(a == 256*6)
      {
         printf("%s", "\b\\");
         a = 0;
      }
      // remove comment block if you want to see hex data in terminal
      /*
      printf("%02X ", (unsigned char)lpBuffer2[0]);
      y += 1;
      if(y == 16)                                        // new line every 32 bytes received
      {
         printf("\n");   
         y = 0;
      }
      */
   }
   printf("\b "); 
   // check for PSOC ack when done
   receive_data();
   if((unsigned char)lpBuffer2[0] == 0xff) printf("\n%s\n", "Done");

   // close file and COM port
   fclose(fptr);
   end_program();
}


// *************************************************
// This function isn't used in the final program
// But I did use it for debugging, so here it stays
// *************************************************
void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}


// Function to close COM port and exit the program
void end_program()
{
   CloseHandle(hComm);  //Closing the Serial Port
   exit(0);
}


// ******************************************************
// This function assists in performing a PCB circuit test
// It instructs the PSOC to set one pin at a time
// The program ends upon completion
// This function is not used for programming the nvSRAM
// but is included for good measure
// ONLY USE THIS TEST WITH UNPOPULATED PCBs!!!!!!!!!!!
// To enter test mode:
//                      nvsram test, [COM]
// ******************************************************
void test_pins()
{
   char pin_test_pattern[27][4] =   {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                                     "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                                     "A8", "A9", "A10", "A11", "A12", "A13", "A14",
                                     "A15", "WE", "OE", "T"};
   char pin_string[10] = {"Pin_"};
   UINT8 test_pin = 0;
   UINT8 st_length = 0;
   int y = 0;
   int z = 0;
   int n = 0;
   // Test one pin at a time
   for(y=0; y<27; y++) {
      strcpy(lpBuffer, &pin_test_pattern[y][0]);      // get first pin from array
      if(lpBuffer[0] != 'T')
      {
         for(z=0; z<4; z++)
         {                            
            // check for string lenth
            if(lpBuffer[z] == 0)      
            {                       
               st_length = z;       // save the string length
               lpBuffer[st_length] = '\0';      // make sure it's a real string
               break;
            }
         }
         // Add pin ID
         strcat(pin_string, lpBuffer);
         st_length = strlen(pin_string)+1;
      }
      else{
         strcpy(pin_string, "T");
         st_length = 2;
      }
      // wait for keypress
      printf("Press ENTER key to Continue");  
      getchar();   
      // Send out test pin string through serial port
      for(n=0; n<st_length; n++) 
      {
         lpNumberOfBytesRead = 0;
         WriteFile(hComm,                    // Handle to the Serial port
                  &pin_string[n],            // Data to be written to the port
                  1,                         //No of bytes to write
                  &dNoOfBytesWritten,        //Bytes written
                  NULL);
         while(lpNumberOfBytesRead == 0) 
         {
            ReadFile(hComm,                  // Handle to the Serial port
                     &lpBuffer2[n],          // Data to be read from the port
                     1,                      // number of bytes to read
                     &lpNumberOfBytesRead,   //Bytes read
                     NULL);
         }
         // Print the returned string
         printf("%c", lpBuffer2[n]);
      }
      pin_string[4] = '\0';         // reset pin ID string 
      printf("\n");
   }
    // wait for keypress
   printf("Press ENTER key to End");  
   getchar();   
}


// *****************************************
// Send a single byte to serial port
// *****************************************
void send_data(unsigned char thedata) 
{
   lpBuffer[0] = thedata;
   lpNumberOfBytesRead = 0;
         WriteFile(hComm,                    // Handle to the Serial port
                  lpBuffer,                 // Data to be written to the port
                  1,                        //No of bytes to write
                  &dNoOfBytesWritten,       //Bytes written
                  NULL);
}

// *****************************************
// Receive a single byte from serial port
// Data is placed into lpBuffer2
// *****************************************
void receive_data()
{
   lpNumberOfBytesRead = 0;
   while(lpNumberOfBytesRead == 0) 
   {
      ReadFile(hComm,                  // Handle to the Serial port
               lpBuffer2,              // Data to be read from the port
               1,                      // number of bytes to read
               &lpNumberOfBytesRead,   //Bytes read
               NULL);
   }
}


// ******************************************************************
// Function to perform a compare between nvSRAM and a .bin file
// Creates a new file in current directory called "nvSRAM_log.bin"
// Compares the original file to newly created log file
// Could have done this all in memory, but I didn't
// To enter:
//          nvsram read, [COM]
// ******************************************************************
void compare_file()
{
   int a = 0;
   int b = 0;
   int x = 0;
   unsigned char aa = 0;
   unsigned char bb = 0;
   unsigned char y = 0;
   int errorcount = 0;
   int sizefile = 0;
   char inputfile[25];
   char readbuffer[25];
   const char magicinit[5] = {'g', ' ', 'G', 'C', 'E'};
   
   // get filename from user
   printf("%s", "Input file to compare: ");
   fgets(inputfile,25,stdin);
   // get file length (drop the CR)
   int len = strlen(inputfile) - 1;
   inputfile[len] = '\0';
   // check for user inputting a valid file name
   if(len > 4) {
      strncpy(mystring, &inputfile[len-4], 4);
      mystring[4] = '\0'; 
      //printf("mystring: %s\n", mystring);
      //printf("end string: %s\n", mystring);
      if(strcmp(mystring, ".bin") == 0){
         printf("%s", "I see it's a .bin file \n");
      }
      else {
      printf("%s", "Doesn't seem to be a .bin file \n");
      end_program();
      }
   }
   else {
   printf("%s", "Not a .bin file");
   end_program();
   }

   // check if file exists
   if ((fptr = fopen(inputfile,"rb")) == NULL){
      printf("File name %s not found\n", inputfile);
      fclose(fptr);
      end_program();
   }
   printf("%s", "File found\n");

   // Get file size in bytes
   fseek(fptr, 0L, SEEK_END);
   sizefile = ftell(fptr);
   rewind(fptr);
   printf("File size in bytes: %d\n", sizefile);
   
   // Check for Magic Init string
   fread(&mystring, 1, 5, fptr);
   for(x = 0; x < 5; x++) {
      if(mystring[x] != magicinit[x]) 
      {
         printf("%s", "Invalid Magic Init String\n");
         end_program();
      }
   }
   rewind(fptr);
   printf("%s", "Valid Vectrex Magic Init String\n");
   printf("%s", "Dumping ROM  ");

   // send file size to PSOC
   y = (unsigned char)(sizefile >> 8);
   send_data(y);                                         // send MSB to PSOC
   y = (unsigned char)sizefile;
   send_data(y);                                         // send LSB to PSOC
   receive_data();                                       // get echo from PSOC
   receive_data();
  
   // get bytes from nvSRAM and send to buffer
   FILE * logfile = fopen("nvSRAM_log.bin", "wb+");
   for(a=0; a < sizefile; a++)
   {
      receive_data();
      fwrite(&lpBuffer2[0],1,1,logfile);
      // Show twirly thing as user waits
      x += 1;
      if(x == 256 || x == 256*4)
      {
         printf("%s", "\b|");
      }
      if(x == 256*2)
      {
         printf("%s", "\b/");
      }
      if(x == 256*3  || x == 256*5)
      {
         printf("%s", "\b-");
      }
      if(x == 256*6)
      {
         printf("%s", "\b\\");
         x = 0;
      }
      // remove comment block if you want to see bytes printed to terminal
      /*
      printf("%02X ", (unsigned char) lpBuffer2[0]);
      b += 1;
      if(b == 16)
      {
         printf("%\n");
         b = 0;
      }
      */
   }
   printf("%s", "\b ");
   fclose(logfile);

   // check ROM dump against orignal file
   printf("%s%s\n", "\nChecking nvSRAM ROM against ", inputfile);
   fopen("nvSRAM_log.bin", "rb");
   for(a=0; a < sizefile; a++)
   {
      aa = fgetc(logfile);
      bb = fgetc(fptr);
      if(aa != bb)
      {
         printf("%s%04X %s%02X %s%02X\n", "Error at ", (WORD) a, "expected ", bb, "received ", aa);
         errorcount += 1;
      }
   }
   printf("%s%d", "Error Count = ", errorcount);
   fclose(logfile);
   fclose(fptr);
}