## Caracteristici 

  - Comunicarea dintre pagina web si esp32 este asincron (bidirectionala) prin websockets cu sincronizare real time intre clientii conectati.
  - Partea de memorare agenda si setari este stocata pe partitia SPIFFS asemenea unei baze de date clasice
  - Cand se incarac codul pt primadata vor fi create setari default
  - In acest moment unele setari cum ar fi cele de wifi pot fi schimabate numai din cod 
  - Pagina web este realizata cu urmatoarele tehnologii: html, tailwind css, alpine js si este functionala si fara conexiune la internet
  - Partea de securitate este una simpla de tipul: Authorization: Basic
  - In consola serial sunt printate mai multe informatii legate de actiunile executate de placa
  - Un aspect foarte important este acela ca interogarea pe care o face placa esp32 
    pe modulul sim800l este realizata numai cand pinul ring de pe modulul sim800l isi schimba starea. Este foarte importanta ca interogarea
    sa se realizeze in acest fel pentru a elimina deleyurile care apar atunci cand interogarea se face in mod repetat
  - Proiectul contine o partitionarea a spatiului de stocare particularizata (fisierul: partition.csv) si utila in alte proiecte.
    Aceasta partitonare contine un spatiu marit pentru partea de cod. 
      RAM:   [==        ]  15.9% (used 51948 bytes from 327680 bytes)
      Flash: [======    ]  63.6% (used 1332990 bytes from 2097152 bytes)
  - Pini SIM800L - esp32: TX - TX2, RX -RX2, RST - D5, RING - D4 
  - Pini Relee: Releu 1 - D32, Releu 2 - D33, 
  - Pini butoane touch: Bt 1 - D26, Bt 2 - D27

## Actualizari Viitoare 



  - adaugare setari pentru wifi in pagina web
  - adaugare  pagina web pentru managementul numerelor de telefon;
  - adaugare senzor de umiditate si temperatura care vor controla releele
  - adaugare intrari care sa controleze iesirle 
  - adaugare noi comenzi sms
  - pagile web vor fi trimise ca arhiva pentru reducerea spatiului si al  timpului de incarcare al paginii

 

## Comenzi SMS



1. Comenzi SMS Generale 

Parola presetata este 1234

```
addnr:1234:0756090xxx,0756090xxx    //- adaugare numere pentru apeluri maxim 5 intr-un sms (romania) nr. fara prefix

delnr:1234:0756090xxx,0756090xxx   //- stergere numere pentru apeluri maxim 5 intr-un sms (romania) nr. fara prefix

delallnr:1234    //- sterge toate numerele de pe cartela sim 

restart:1234  //- repornire modul 

setzonaapel:1234:1  //- 1 reprezinta iesirea care va fi activa atunci cand se apeleaza modulul;

setpass:1234:2345   //- setare parola noua parola   :parola actuala:parola noua

delallsms:1234  //- stergerea tuturor sms-urilor    
```




---------------------

 2. Setari Zone Iesire (relee)

```
zout1:1234:sw   //- iesirea1:parola: mod contact de tip switch (contact permanent pana la urmatoarea comanda)

zout1:1234:bt:4   //- iesirea1:parola:mod contact de tip button:secunde (contact cu intarziere, cifra 4 reprezinta numarul de secunde dupa care isi schimba starea)                                                                                   

1on      //-releu 1 pornit

1off     //-releu 1 oprit
```





 