/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   structures.h
 * Author: ssabri
 *
 * Created on 3 mai 2021, 11:06
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

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

    


#endif /* STRUCTURES_H */

