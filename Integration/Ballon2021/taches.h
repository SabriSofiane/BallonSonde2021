/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   taches.h
 * Author: ssabri
 *
 * Created on 6 mai 2021, 12:23
 */

#ifndef TACHES_H
#define TACHES_H
#define SERIAL_BAUD 115200
#define LED 22
#include "structures.h"
class Taches {
public:
    Taches();
    //void ObtenirTaches(void * parameter);
   static void tacheBME280(void * parameter);
   static void tacheRadiations(void* parameter);
   static void tacheGPS(void* parameter);
   static void tacheAffichage(void* parameter);
   static void tacheSigfox(void* parameter);
private:
   
  
};


#endif /* TACHES_H */

