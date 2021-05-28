

# Projet Ballon Sonde 2021
![enter image description here](https://cdn.discordapp.com/attachments/789485236072742933/847763221209284638/ballon.png)

## Présentation du projet
Notre projet consiste à envoyer une nacelle contenant une ESP32 avec des capteurs (capteur de radiations, température, humidité, pression et GPS) pour obtenir des données en haute altitude. 
Ces données sont ensuite enregistrées dans une carte SD toutes les minutes, elles sont aussi envoyé avec la technologie Sigfox toutes les 10 minutes. 
Pour faciliter la récupération du ballon, un message SMS est envoyé avec la position et l'altitude du ballon lorsque l'altitude est inférieure à 2000 mètres.
Les données Sigfox sont décodées et les mesures sont affichées en temps réel sous forme de courbes et la position du ballon est visible sur une carte 3D sur le site web suivant: 
http://touchardinforeseau.servehttp.com/ballon2021/

 Membres de l'équipe: 
[SABRI Sofiane](https://github.com/SabriSofiane), [PACOT Antoine](https://github.com/apacot), [YOUSFI Ahmed](https://github.com/Ahmed-Ysf) et [BOUGEOT Louis](https://github.com/lbougeot)

 ### Technologies utilisées:
 - [Bootstrap](https://getbootstrap.com/)
 - [JQuery](https://jquery.com)
 - [Cesium](https://cesium.com/)
 - [Highcharts](https://www.highcharts.com/)
 - [Datatables](https://datatables.net/)
 - [Sigfox](https://www.sigfox.com/en)
 - [FreeRTOS](https://www.freertos.org/)
 - [Crontab](https://crontab-generator.org/)





