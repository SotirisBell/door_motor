/*
 Name:		DOOR_MOTOR.ino
 Created:	19/09/2023
 Author:	sotiris Bell
*/

/************************************************************///INCLUDES
#include <RCSwitch.h>

#include <avdweb_VirtualDelay.h>

/**************************************************************///INPUTS
#define D_LIMIT  14
#define INFRA  15//infra beam  
#define RF_433   16 //
#define BUTTON   17 
#define LDR_LIGHT A6  // light intensity
#define LDR_SET  A7 //pot light set


/*************************************************************///OUTPUTS
#define  MOTOR_A  4  // motor drive a
#define  MOTOR_B  5 // motor drive b
#define   MOTOR_TR 6  //Trigger
#define   FWTA 7  //
#define   MOTION_ON 8  //
#define  FLASHER  9  //flasher
#define  OUT10  10 


/***********************************************************///VARIABLES
int BUTTON_RF = 0;


typedef struct {
	char* ID;
	long BUA;
	long BUB;
	long BUC;
	long BUD;
}	record_type;


record_type BUTRF[10];

/**********************************************************************/

//**********************************************************************
char* DRC[3] = { "STOP ","OPEN ","CLOSE" };
char* DRS[5] = { "STOPED ","OPENING","CLOSING","OPENED ","CLOSED " };

//INPUT STATUS
bool Sdlimit = 0;  //sw door limit
bool Sinfra = 0; //infra beam
int Sfwta = 0;
bool Sbutton = 0;
bool Smotor_tr = 0;
int Smotor = 0;
int Smotion = 0;
bool Sflasher = 0;
bool motion_en = 1;
int autoclose = 1;
int ldr_light;
int ldr_set;
//int laser_sens = 50;   //0 - 100
int safedelay = 30;

/**********************************************************************/
//MOTOR_STATE
#define stop  0
#define open  1
#define close  2

/**********************************************************************/
//DOOR STATUS
int DOOR_STATE = 0;  //
#define Dstoped  0
#define Dopening 1
#define Dclosing 2
#define Dopened  3
#define Dclosed  4

int DOOR_COMM = 0;
/**********************************************************************/
bool STARTUP = 0;



//**********************************************************************
VirtualDelay flash_delay;
VirtualDelay motor_tr_delay;
VirtualDelay waitclose_delay;
VirtualDelay safe_delay;
VirtualDelay autoclose_delay;
//**********************************************************************

RCSwitch mySwitch = RCSwitch();


//**********************************************************************
//**********************************************************************
/***************************************************************///SETUP
void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	pinMode(MOTOR_A, OUTPUT);
	pinMode(MOTOR_B, OUTPUT);
	pinMode(FLASHER, OUTPUT);
	pinMode(MOTION_ON, OUTPUT);

	pinMode(MOTOR_TR, OUTPUT);
	pinMode(FWTA, OUTPUT);
	pinMode(OUT10, OUTPUT);
	pinMode(D_LIMIT, INPUT);
	pinMode(INFRA, INPUT);
	pinMode(LDR_SET, INPUT);
	pinMode(BUTTON, INPUT);
	pinMode(RF_433, INPUT);

	Serial.println("Starting...");
	delay(500);
	digitalWrite(MOTOR_A, 0);
	digitalWrite(MOTOR_B, 0);
	digitalWrite(FLASHER, 0);
	digitalWrite(MOTOR_TR, 0);
	digitalWrite(FWTA, 0);
	digitalWrite(MOTOR_TR, 0);
	digitalWrite(MOTION_ON, 0);

	BUTRF[1].BUA = 10036674;
	BUTRF[1].BUB = 10036680;
	BUTRF[1].BUC = 10036673;
	BUTRF[1].BUD = 10036676;

	BUTRF[2].BUA = 4961250;
	BUTRF[2].BUB = 4961256;
	BUTRF[2].BUC = 4961249;
	BUTRF[2].BUD = 4961252;

	mySwitch.enableReceive(0);

	STARTUP = 1;


}


/****************************************************************///LOOP
void loop() {

	if (STARTUP == 1)SET_DOOR(close); STARTUP = 0;

	GET_RF();
	GET_INPUTS();
	if (ldr_light < ldr_set && Sfwta == 0)
	{
		SET_FWTA(1);
	}

	if (ldr_light > ldr_set && Sfwta == 1)
	{
		SET_FWTA(0);
	}




	//----------------------------------------------------------------------
	if (flash_delay.elapsed()) {

		if (DOOR_COMM == open) {

			SET_MOTOR(open);
			delay(1000);
			SET_MOTOR_TR(1);
			safe_delay.start(safedelay * 1000);
			DOOR_STATE = Dopening;

		}
		else if (DOOR_COMM == close)
		{
			SET_MOTOR(close);
			delay(1000);
			SET_MOTOR_TR(1);

			safe_delay.start(safedelay * 1000);
			DOOR_STATE = Dclosing;
		}



	}

	//----------------------------------------------------------------------
	if (motor_tr_delay.elapsed()) SET_MOTOR_TR(0);

	if (safe_delay.elapsed()) SET_DOOR(Dstoped);
	//----------------------------------------------------------------------
	if (autoclose_delay.elapsed()) SET_DOOR(Dclosing);

	//----------------------------------------------------------------------


	Serial.print(">OUT>  mtr = " + String(Smotor) + " mtr_tr = " + String(Smotor_tr) + " fwta = " + String(Sfwta) + " motion = " + String(Smotion) + " flasher = " + String(Sflasher) + " DR_ST = " + DRS[DOOR_STATE]);

	Serial.println("  <INP<  limit = " + String(Sdlimit) + " infra = " + String(Sinfra) + " BUT_RF = " + String(BUTTON_RF) + " button = " + String(Sbutton) + " ldr_set = " + String(ldr_set) + " ldr_light = " + String(ldr_light) + " DR_COM = " + DRC[DOOR_COMM]);



	delay(500);

}


/***********************************************************/


void outtest()
{

	for (int i = 4; i < 11; i++)
	{

		digitalWrite(i - 1, 0);

		digitalWrite(10, 0);

		digitalWrite(i, 1);
		Serial.println("i=  " + String(i));
		delay(1000);

	}


}
//**********************************************************************

void SET_DOOR(int a)
{
	switch (a) {
	case 0:	//stop
		SET_FLASHER(0);
		SET_MOTION(1);
		SET_MOTOR(stop);
		SET_MOTOR_TR(0);
		DOOR_STATE = Dstoped;
		DOOR_COMM = stop;
		flash_delay.Stop();
		safe_delay.Stop();
		//if (autoclose!=0) autoclose_delay.start(autoclose);	
		break;
	case 1:		//opening
		SET_FLASHER(1);
		SET_MOTION(1);
		DOOR_COMM = open;
		flash_delay.start(3000);



		break;
	case 2:		//closing
		SET_FLASHER(1);
		SET_MOTION(1);
		DOOR_COMM = close;
		flash_delay.start(5000);



		break;

	case 3:	//opened
		SET_FLASHER(0);
		SET_MOTION(1);
		SET_MOTOR(stop);
		SET_MOTOR_TR(0);
		DOOR_STATE = Dopened;
		safe_delay.Stop();
		DOOR_COMM = stop;
		if (autoclose != 0) autoclose_delay.start(autoclose);
		break;


	case 4:	//closed
		SET_FLASHER(0);
		SET_MOTION(0);
		SET_MOTOR(stop);
		SET_MOTOR_TR(0);
		DOOR_STATE = Dclosed;
		safe_delay.Stop();
		DOOR_COMM = stop;
		break;



	}


}


//**********************************************************************

//**********************************************************************
void SET_MOTOR_TR(bool a)
{
	Smotor_tr = a;
	digitalWrite(MOTOR_TR, a);
	if (a == 1) motor_tr_delay.start(3000);
}


//**********************************************************************
void SET_MOTOR(int a)
{
	Smotor = a;
	switch (a) {
	case 0:		//stop
		digitalWrite(MOTOR_A, 0);
		digitalWrite(MOTOR_B, 0);
		break;
	case 1:		//open
		digitalWrite(MOTOR_A, 1);
		digitalWrite(MOTOR_B, 0);

		break;
	case 2:		//close
		digitalWrite(MOTOR_A, 0);
		digitalWrite(MOTOR_B, 1);
		break;
	}

}
//**********************************************************************
void SET_MOTION(bool a)
{
	Smotion = a;
	digitalWrite(MOTION_ON, a);


}

//**********************************************************************
void SET_FWTA(bool a)
{
	Sfwta = a;
	digitalWrite(FWTA, a);


}
//**********************************************************************
void SET_FLASHER(bool a)
{
	Sflasher = a;
	digitalWrite(FLASHER, a);


}
//**********************************************************************


//**********************************************************************
void GET_INPUTS()
{
	// if (Smotor_tr==0){
	Sdlimit = digitalRead(D_LIMIT);
	if (Sdlimit == 1)
	{
		switch (DOOR_STATE) {
		case  Dopening:
			SET_DOOR(Dopened);
			break;
		case Dclosing:
			SET_DOOR(Dclosed);
			break;
		}

	}




	Sinfra = !digitalRead(INFRA);
	Sbutton = !digitalRead(BUTTON);

	ldr_set = map(analogRead(LDR_SET), 0, 1024, 0, 1000);
	ldr_light = map(analogRead(LDR_LIGHT), 0, 1024, 0, 1000);


}


//**********************************************************************

void GET_RF()
{

	BUTTON_RF = 0;
	if (mySwitch.available()) {
		//Serial.print("Received ");

		long a = mySwitch.getReceivedValue();
		/*
		  Serial.print( mySwitch.getReceivedValue() );
		  Serial.print(" / ");
		  Serial.print( mySwitch.getReceivedBitlength() );
		  Serial.print("bit ");
		  Serial.print("Protocol: ");
		  Serial.println( mySwitch.getReceivedProtocol() );
		 */
		mySwitch.resetAvailable();

		for (int i = 1; i < 10; i++)
		{
			if (a == BUTRF[i].BUA) BUTTON_RF = 1;
			if (a == BUTRF[i].BUB) BUTTON_RF = 2;
			if (a == BUTRF[i].BUC) BUTTON_RF = 3;
			if (a == BUTRF[i].BUD) BUTTON_RF = 4;
			//Serial.println("BUTTON_RF = " + String(BUTTON_RF));
		}


		switch (BUTTON_RF) {
		case 1: //open

			if (DOOR_STATE == Dclosed || DOOR_STATE == Dstoped) SET_DOOR(open);

			break;
		case 2:		//close

			if (DOOR_STATE == Dopened || DOOR_STATE == Dstoped) SET_DOOR(close);
			break;
		case 3:		//stop
			SET_DOOR(stop);

			break;
		case 4:		//

			break;
		}

	}
}
//**********************************************************************

//**********************************************************************


//**********************************************************************
