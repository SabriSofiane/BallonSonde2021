/**
 * @file main.ino.cpp
 * @brief main contenant les taches du ballon sonde
 * @version 1.0
 * @author Sofiane SABRI
 * @date 5/5/2021
 */
#include <Arduino.h>
#include <Wire.h>
#include <BME280I2C.h>
#include <RadiationWatch.h>
#include <TinyGPS.h>
#include <HardwareSerial.h>

#define SERIAL_BAUD 115200
#define LED 22
SemaphoreHandle_t mutex;

RadiationWatch radiationWatch(32, 33);
TinyGPS gps;
HardwareSerial serialGps(2); // sur hardware serial 2

typedef struct {
    byte seconde;
    byte minute;
    byte heure;
} typeHeure;

typedef struct {
    byte jour;
    byte mois;
    unsigned int annee;
} typeDate;

typedef struct {
    float temperature;
    float humidite;
    float pression;
    float cpm;

} typeDonneesCapteurs;

typedef struct {
    float altitude;
    float latitude;
    float longitude;
} typePosition;

typedef struct {
    typePosition position;
    typeHeure heures;
    typeDate date;
    typeDonneesCapteurs DonneesCapteurs;
} typeDonnees;


typeDonneesCapteurs capteur;
typePosition donneesGPS;
typeDate dateGPS;
typeHeure heureGPS;


void onRadiation() {

    radiationWatch.uSvh();
    radiationWatch.uSvhError();
    radiationWatch.cpm();

}

/**
 * @brief tacheBME280 Reception des données du capteur BME280 et enregistrement dans la structure partagée
 * @param parameter
 */
void tacheBME280(void * parameter) {
    BME280I2C::Settings settings(
            BME280::OSR_X1,
            BME280::OSR_X1,
            BME280::OSR_X1, BME280::Mode_Forced,
            BME280::StandbyTime_1000ms,
            BME280::Filter_Off,
            BME280::SpiEnable_False,
            BME280I2C::I2CAddr_0x77 // 0x77 I2C address pour BME 280 Adafruit.
            );
    BME280I2C bme(settings);
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    while (!Serial) {
    } // Wait

    Wire.begin();

    while (!bme.begin()) {
        Serial.println("Could not find BME280 sensor!");
        delay(1000);
    }

    switch (bme.chipModel()) {
        case BME280::ChipModel_BME280:
            Serial.println("Found BME280 sensor! Success.");
            break;
        case BME280::ChipModel_BMP280:
            Serial.println("Found BMP280 sensor! No Humidity available.");
            break;
        default:
            Serial.println("Found UNKNOWN sensor! Error!");
    }
    for (;;) {

        float temp(NAN), hum(NAN), pres(NAN);
        BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
        BME280::PresUnit presUnit(BME280::PresUnit_hPa);

        bme.read(pres, temp, hum, tempUnit, presUnit);
        //ouverture du mutex
        xSemaphoreTake(mutex, portMAX_DELAY);
        capteur.temperature = temp;
        capteur.humidite = hum;
        capteur.pression = pres;
        //fermeture du mutex
        xSemaphoreGive(mutex);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(60000)); // reveille toutes les 60s
    }

}

/**
 * tacheGPS Reception des données du capteur de radiations et enregistrement dans la structure partagée
 * @param parameter
 */
void tacheRadiations(void* parameter) {

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    radiationWatch.registerRadiationCallback(&onRadiation);

    for (;;) {

        radiationWatch.loop();
        xSemaphoreTake(mutex, portMAX_DELAY);
        capteur.cpm = radiationWatch.cpm();
        //fermeture du mutex
        xSemaphoreGive(mutex);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(60000)); // reveille toutes les 60s

    }
}
/**
 * @brief tacheGPS Reception des données du GPS et enregistrement dans la structure partagée
 * @param parameter
 */

void tacheGPS(void* parameter) {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    Serial.print("Simple TinyGPS library v. ");
    Serial.println(TinyGPS::library_version());
    for (;;) {
        bool newData = false;
        unsigned long chars;
        unsigned short sentences, failed;

        // Pendant une seconde lecture des caractères envoyés par le GPS
        for (unsigned long start = millis(); millis() - start < 1000;) {
            while (serialGps.available()) {
                char c = serialGps.read();
                // Serial.write(c);   
                if (gps.encode(c)) // si des caratères sont reçus alors newdata = true
                    newData = true;
            }
        }

        if (newData) {
            float lat, lon, alt;
            unsigned long date, time;
            unsigned long age;
            //HEURE
            byte seconde;
            byte minute;
            byte heure;

            //DATE
            byte jour;
            byte mois;
            byte hundredths;
            int annee;
            gps.f_get_position(&lat, &lon, &age);
            gps.get_datetime(&date, &time, &age);
            gps.crack_datetime(&annee, &mois, &jour, &heure, &minute, &seconde, &hundredths, &age);
            alt = gps.f_altitude();
            heure = heure + 2;
            //ouverture du mutex
            xSemaphoreTake(mutex, portMAX_DELAY);
            donneesGPS.latitude = lat;
            donneesGPS.longitude = lon;
            donneesGPS.altitude = alt;
            dateGPS.jour = jour;
            dateGPS.mois = mois;
            dateGPS.annee = annee;
            heureGPS.heure = heure;
            heureGPS.minute = minute;
            heureGPS.seconde = seconde;
            //fermeture du mutex
            xSemaphoreGive(mutex);
        }

        gps.stats(&chars, &sentences, &failed);

        if (chars == 0)
            Serial.println("** No characters received from GPS: check wiring **");
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(60000)); // reveille toutes les 60s

    }


}

/**
 * @brief tacheAffichage affichage des données de la structure partagée
 * @param parameter
 */
void tacheAffichage(void* parameter) {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    delay(5000);

    for (;;) {
        //ouverture du mutex
        xSemaphoreTake(mutex, portMAX_DELAY);
        //GPS
        Serial.print("LAT: ");
        Serial.print(donneesGPS.latitude, 6);
        Serial.println(" ");
        Serial.print("LON: ");
        Serial.print(donneesGPS.longitude, 6);
        Serial.println(" ");
        Serial.print("ALT: ");
        Serial.print(donneesGPS.altitude);
        Serial.println(" ");
        Serial.print("date: ");
        Serial.print(dateGPS.jour);
        Serial.print("/");
        Serial.print(dateGPS.mois);
        Serial.print("/");
        Serial.print(dateGPS.annee);
        Serial.println(" ");
        Serial.print("heure: ");
        Serial.print(heureGPS.heure);
        Serial.print("h");
        Serial.print(heureGPS.minute);
        Serial.print("m");
        Serial.print(heureGPS.seconde);
        Serial.print("s");
        Serial.println(" ");
        //Radiations
        Serial.print("CPM: ");
        Serial.print(capteur.cpm);
        Serial.println(" ");
        //BME280
        Serial.print("Temperature: ");
        Serial.print(capteur.temperature);
        Serial.print("°C");
        Serial.println(" ");
        Serial.print("Humidité: ");
        Serial.print(capteur.humidite);
        Serial.print("%HR");
        Serial.println(" ");
        Serial.print("Pression: ");
        Serial.print(capteur.pression);
        Serial.print("hPa");
        Serial.println(" ");
        //fermeture du mutex
        xSemaphoreGive(mutex);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(60000)); // reveille toutes les 40s
    }

}

void setup() {

    Serial.begin(SERIAL_BAUD);
    serialGps.begin(4800, SERIAL_8N1, 16, 17);
    radiationWatch.setup();
    mutex = xSemaphoreCreateMutex();

    xTaskCreate(
            tacheBME280, /* Task function. */
            "tacheBME280", /* name of task. */
            10000, /* Stack size of task */
            NULL, /* parameter of the task */
            1, /* priority of the task */
            NULL); /* Task handle to keep track of created task */

    xTaskCreate(
            tacheRadiations, /* Task function. */
            "tacheRadiations", /* name of task. */
            10000, /* Stack size of task */
            NULL, /* parameter of the task */
            1, /* priority of the task */
            NULL); /* Task handle to keep track of created task */

    xTaskCreate(
            tacheGPS, /* Task function. */
            "tacheGPS", /* name of task. */
            10000, /* Stack size of task */
            NULL, /* parameter of the task */
            4, /* priority of the task */
            NULL); /* Task handle to keep track of created task */

    xTaskCreate(
            tacheAffichage, /* Task function. */
            "tacheAffichage", /* name of task. */
            2000, /* Stack size of task */
            NULL, /* parameter of the task */
            1, /* priority of the task */
            NULL); /* Task handle to keep track of created task */



}

void loop() {

}