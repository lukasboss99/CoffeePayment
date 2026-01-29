#include <DatabaseOnSD.h>
#include <SPI.h>
//#include <SD.h>
#include <SdFat.h>
SdFat SD;

#define SD_CS 47
#define SD_CLK 19
#define SD_MISO 20
#define SD_MOSI 21
SPIClass spiSD(FSPI);              // SPI 1 für RFID und Display
MyTable testTable("keytags.csv");  //this will create or open a table named test.csv in the root of the SD card

void setup() {

	Serial.begin(9600);
	delay(100);
	spiSD.begin(SD_CLK, SD_MISO, SD_MOSI);
	// SD Card
	if (!SD.begin(SD_CS, spiSD, 1000000)) {
		Serial.println("SD-Karte konnte nicht initialisiert werden");
		while (true) delay(1000);
	}


	Serial.println("SD-Karte erkannt");

	testTable.printSDstatus();  //[optional] print the initialization status of SD card
	                            //testTable.emptyTable();     //[optional] empty table content (make sure to call begin(rowN, colN) after emptying a table) // you could always add more rows.
	                            //testTable.begin(3, 2);      //[optional] initialize an empty table with 3 rows and 2 columns (has no effect if table is not empty)
	                            //NOTE: there is no need to call begin() if table in NOT empty

	/*	
			0   		1
0 		NAME		AGE
1 		Divino	23
2  		Fire		22

The table above has 3 rows and 2 columns
NAME is in cell (0, 0) and 21 is in cell (2, 1)


	//write table content
	Serial.println("writing to table...");
	testTable.writeCell(0, 0, "NAME");
	testTable.writeCell(0, 1, "AGE");
	testTable.writeCell(1, 0, "Divino");
	testTable.writeCell(1, 1, "23");
	testTable.writeCell(2, 0, "Fire");
	testTable.writeCell(2, 1, "22");
	Serial.println("finished writing!");
	//the max size of each cell is 20 characters for the sake of memory,
	//the max cell size can be changed in header file [but I do not recommend it]
	*/
}

void loop() {
	
	//read all table content cell by cell
	int numRows = testTable.countRows();
	int numCols = testTable.countCols();

	Serial.println("-------------read all table content cell by cell ----------------------");

	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			Serial.println(testTable.readCell(i, j));
			//delay(1000);  // one second delay after each read
		}
	}

	Serial.println();  // space separate the next serial print by an empty line

	delay(3000);
/*
	//read all table content at once
	Serial.println("-------------read all table content at once ----------------------");
	testTable.printTable();  //print entire table content to serial monitor

	Serial.println("\n\n\n\n\n");  // space separate the next serial print by 5 empty line

	//to erase the data in a cell, parse in an empty string as the data content
	//when you read a cell, it returns a String object, if it is a number, you can convert using toint or toFloat method on the String
*/
	delay(10000);
}
