#include "sigfox.h"
#include "structures.h"
#include "HardwareSerial.h"

//Message buffer
uint8_t msg[12];

Sigfox::Sigfox (uint8_t rxPin=26 , uint8_t txPin=27, bool debugEn=true) {
	serialSig = new HardwareSerial(SERIALSIGFOX); //Sur hardware serial 2
  rx = rxPin;
  tx = txPin;
  debug = debugEn;
}

// Lecture des datas reçu sur la liaison série HardwareSerial 
String Sigfox::obtenirData(void){
  String data = "";
  char output;

  while (!serialSig->available()){
    delay(100);
  }

  while(serialSig->available()){
    output = serialSig->read();
    if ((output != 0x0A) && (output != 0x0D)){       //0x0A Line feed | 0x0D Carriage return
      data += output;
      }
    delay(10);
  } 
  return data;  
}

// Initialise la liaison avec le composant Sigfox
void Sigfox::begin(){

	serialSig->begin(9600, SERIAL_8N1, rx, tx);

	if (debug) {
		Serial.begin(9600);
		delay(100);
    tester();
    delay(100);
		obtenirID();
		delay(100);
		obtenirPAC();
	}
}

//Méthode pour obtenir l'identifiant Sigfox
String Sigfox::obtenirID(){
	serialSig->print("AT$I=10\r");
	String id = obtenirData();

	if(debug){
		Serial.print("ID: ");
		Serial.println(id);
	}

	return id;
}


//Méthode pour obtenir le PAC number
String Sigfox::obtenirPAC(){
	serialSig->print("AT$I=11\r");
	String pac = obtenirData();

	if(debug){
		Serial.print("PAC: ");
		Serial.println(pac);
	}

	return pac;
}

//Méthode pour obtenir la température du module
uint16_t Sigfox::obtenirTemp(void){
	serialSig->print("AT$T?\r");
	uint16_t tempVal = obtenirData().toInt();

	if(debug){
		Serial.print("Module temperature : ");
		Serial.println(tempVal);
	}

	return tempVal;
}

/** Méthode pour tester le composant 
 *  la commande AT renvoie OK
 */

 String Sigfox::tester(){
  serialSig->print("AT\r");
  String status = obtenirData();

  if(debug){
    Serial.print("Status : ");
    Serial.println(status);
  }

  return status;
}

/**
 * Méthode pour envoyer des data
 * data un tableau de 12 octets au maximum
 * size le nombre d'octet à envoyer
 * retourne vrai si le message a été envoyé avec success
 */
bool Sigfox::envoyer(const void* data, uint8_t size){   
  String status = "";
  char output;

  uint8_t* bytes = (uint8_t*)data;

  serialSig->print("AT$SF=");
  if(debug) { Serial.print("Byte : "); }
  for(uint8_t i= 0; i<size; ++i){
    serialSig->print(bytes[i] < 16 ? "0" : "");
    serialSig->print(bytes[i], HEX);
    
    if(debug){
      Serial.print(bytes[i] < 16 ? "0" : "");     
      Serial.print(bytes[i], HEX);
    }
  }
  if(debug) { Serial.println(" "); }
  serialSig->print("\r");

  String res = obtenirData();

  if(res.indexOf("OK") >= 0) {
    Serial.println("Message envoyé avec succès");
    return true;
  }
  
  if(debug){
    Serial.print("Status : ");
    Serial.println(res);
  }
  return false;
}


void Sigfox::coderTrame(typeDonnees *lesDonnees)
{
    //conversion des donnees    
    unsigned short int altitude = lesDonnees->position.altitude;
    unsigned int latitude = lesDonnees->position.latitude * 1000000.0;
    unsigned int longitude;
    unsigned short int radiationCpm = lesDonnees->DonneesCapteurs.cpm;
    unsigned short int pression = lesDonnees->DonneesCapteurs.pression;
    byte temperature = lesDonnees->DonneesCapteurs.temperature;
    
    //traitement temperature
    if (lesDonnees->DonneesCapteurs.temperature < 0) {
        temperature = lesDonnees->DonneesCapteurs.temperature * (-1.0);
    }
    
    //traitement longitude
    if (lesDonnees->position.longitude < 0) {
        longitude = lesDonnees->position.longitude * (-1000000.0);
    }
    else
    {
        longitude = lesDonnees->position.longitude * (1000000.0);
    }
    
    byte octetCourant; //variable sur un octet (= 8bits)
    byte quartetGauche;
    byte quartetDroit;
    int i = 11; //indice du tableau (on commence à la fin du tableau)
    
    //------------------------------------------------codage trame (en partant de la fin) ----------------------------------------------------

    //codage du 12e octet de la trame
    octetCourant = radiationCpm;
    trame[i] = octetCourant; //stockage du 12 octet dans le tableau d'octet
    i--; //i = i-1 (pour accéder à la case suivant (en partant de la fin))

    //codage du 11e octet
    quartetGauche = radiationCpm >> 8; //décalage de 8 vers la droite
    quartetDroit = pression << 4; // décalage de 4 vers la gauche
    octetCourant = quartetDroit | quartetGauche; //octetCourant = quartetDroit OU quartetGauche;
    trame[i] = octetCourant; // stockage du 11e octet (à la case i (=10))
    i--;

    //codage du 10e octet
    quartetDroit = pression >> 4;
    quartetGauche = temperature << 7;
    octetCourant = quartetDroit | quartetGauche;
    trame[i] = octetCourant;
    i--;

    //codage du 9e octet
    quartetDroit = temperature >> 1;
    quartetGauche = altitude << 6;
    octetCourant = quartetDroit | quartetGauche;

    if (lesDonnees->DonneesCapteurs.temperature < 0) // Si température négatif
    {
        octetCourant = octetCourant | 0x20; //bit de signe à 1
    } else {
        octetCourant = octetCourant & 0xDF; //bit de signe à 0
    }
    trame[i] = octetCourant;
    i--;

    //codage du 8e octet
    octetCourant = altitude >> 2;
    trame[i] = octetCourant;
    i--;

    //codage du 7e octet
    quartetDroit = altitude >> 10;
    quartetGauche = longitude << 5;
    octetCourant = quartetDroit | quartetGauche;
    trame[i] = octetCourant;
    i--;

    //codage du 6e octet
    octetCourant = longitude >> 3;
    trame[i] = octetCourant;
    i--;

    //codage du 5e octet
    octetCourant = longitude >> 11;
    trame[i] = octetCourant;
    i--;

    //codage du 4e octet
    
    quartetDroit = longitude >> 19;
    quartetGauche = latitude << 5;
    octetCourant = quartetDroit | quartetGauche;

    if (lesDonnees->position.longitude < 0) {
        octetCourant = octetCourant | 0x10; //bit de signe à 1
    } else {
        octetCourant = octetCourant & 0xEF; //bit de signe à 0
    }
    trame[i] = octetCourant;
    i--;

    //codage du 3e octet
    octetCourant = latitude >> 3;
    trame[i] = octetCourant;
    i--;

    //codage du 2e octet
    octetCourant = latitude >> 11;
    trame[i] = octetCourant;
    i--;

    //codage du 1e octet
    octetCourant = latitude >> 19;
    trame[i] = octetCourant;
}