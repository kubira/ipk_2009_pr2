#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <regex.h>
#include <signal.h>
#define BUFFER_LEN 1000

/*
  Autor:
    Radim KUBI� (xkubis03)
*/

int main(int argc, char *argv[]) {

  int sock, t;  //Deskriptor socketu
  int port = -1;  //��slo portu
  int sinlen;  //Velikost sockaddr_in sin
  int i, index, n;  //Indexy, po�et p�ijat�ch znak�
  int pocetArgs = 0;  //Po�et zadan�ch argument�
  int CRC = 0, hash = 0;  //Kontroln� sou�et spr�vnosti hledan�ho z�znamu
  int status;  //Status zpracov�n� ��dku regul�rn�m v�razem
  struct sockaddr_in sin;  //Struktura socketu
  char msg[BUFFER_LEN];  //Zpr�va od klienta
  struct hostent *hp;  //Struktura hosta
  char delims[] = " ";  //Odd�lova� argument�
  char *result = NULL;  //V�sledek rozd�len� argument�
  char arguments[9][40];  //Argumenty
  char Name[80], Surname[80], Login[80], Faculty[80];  //Zadan� krit�ria
  char radek[80];  //Prom�nn� pro na�ten� jednoho ��dku ze souboru
  char buffer[BUFFER_LEN];  //�et�zec pro chybu p�i rozd�lov�n� adresy serveru
  char *reg_vyraz = "^([0-9]+);(x[a-z]{5}[0-9]{2});([-A-Za-z]+);([A-Za-z]+);([A-Z]+)$";  //RE
  FILE *f;  //Deskriptor souboru
  regmatch_t re_struct[6];  //Struktura pro ulo�en� �et�zc� z RE
  regex_t re_compiled;  //Zkompilovan� RE
  pid_t pidPotomka;
  
  if(argc == 1) {  //Pokud je jen jeden argument - nic nebylo zad�no
    fprintf(stderr, "Nebyly zad�ny argumenty!!!\n");  //Vyp�i chybu
    return -1;  //a vrac�m -1
  }
  
  if(!strcmp(argv[1], "-p")) {
    if(argc >= 3) {
      port = atoi(argv[2]);
    } else {
      fprintf(stderr, "Nebylo zad�no ��slo portu!!!\n");
      return -1;
    }
  } else {
    fprintf(stderr, "Nebyl zad�n argument port!!!\n");
    return -1;
  }
  
  strcpy(Name, "");  //�et�zec pro krit�rium jm�no je zat�m pr�zdn�
  strcpy(Surname, "");  //�et�zec pro krit�rium p��jmen� je zat�m pr�zdn�
  strcpy(Login, "");  //�et�zec pro krit�rium login je zat�m pr�zdn�
  strcpy(Faculty, "");  //�et�zec pro krit�rium fakultu je zat�m pr�zdn�
  
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  //Vytvo��m socket
    fprintf(stderr, "Chyba vytvo�en� socketu!!!\n");  //Chyba
    return -1;  //a vrac�m -1
  }
  
  sin.sin_family = PF_INET;  //Nastaven� rodiny protokol�
  sin.sin_port = htons(port);  //Nastaven� portu
  sin.sin_addr.s_addr  = INADDR_ANY;  //Nastaven� adresy IP
  
  if(bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {  //Bind
    fprintf(stderr, "Chyba funkce bind!!!\n");  //Chyba
    return -1;  //a vrac�m -1
  }
  
  if(listen(sock, 5)) {  //Listen
    fprintf(stderr, "Chyba funkce listen!!!\n");  //Chyba
    return -1;  //a vrac�m -1
  }
  
  sinlen = sizeof(sin);  //Zjist�m velikost sin
  
  while(1) {  //Cyklus pro p��jem zpr�v od klient�
    if((t = accept(sock, (struct sockaddr *)&sin, &sinlen)) < 0) {  //Accept
      fprintf(stderr, "Chyba funkce accept!!!\n");  //Chyba
      return -1;  //a vrac�m -1
    }
    
    pidPotomka = fork();
    
    if(pidPotomka == 0) {
/******************************************************************************/
    hp = (struct hostent *)gethostbyaddr((char *)&sin.sin_addr,4,AF_INET);
    //Zjist�m hosta, kde jsem spu�t�n
    
    bzero(msg,sizeof(msg));  //Vyma�u �et�zec msg
    if((n = recv(t, msg, BUFFER_LEN, 0)) < 0) {  //P�ijmu parametry od clienta
      fprintf(stderr, "Chyba funkce recv!!!\n");  //Chyba
      return -1;  //a vrac�m -1
    }
    
    if(strcmp(msg, "END") == 0) {  //Pokud p��jde zpr�va END ukon�� se server
      pid_t rodic = getppid();
      kill(rodic, SIGKILL);
      return 0;  //�sp�n� ukon�en� programu
    }

    result = strtok(msg, delims);  //Zjist�m prvn� argument v�pisu
  
    for(i = 0; i < 9; i++) {  //Cyklem proch�z�m �et�zce argument� v�pisu
      strcpy(arguments[i], "");  //Zkop�ruju do ka�d�ho pr�zdn� �et�zec
    }
  
    pocetArgs = 0;  //Nastav�m po�et argument� na nulu
  
    while(result != NULL) {  //Pokud existuje argument
      strcpy(arguments[pocetArgs], result);  //Zkop�ruju ho do �et�zce
      result = strtok(NULL, delims);  //Z�sk�m dal�� odd�len�m
      pocetArgs++;  //Inkrementuji po�et argument�
    }
  
    for(i = 0; i < pocetArgs; i++) {  //Proch�z�m zadan� argumenty v cyklu
      if(strcmp(arguments[i], "-n") == 0) {  //Pokud je argument -n
        if(pocetArgs > (i+1)) {  //Pt�m se, jestli existuje dal�� a pokud ano
          strcpy(Name, arguments[i+1]);  //Zkop�ruju krit�rium
          i++;  //Posunu se na dal�� argument
          hash += 3;  //P�i�tu k hash ��slo t�i - mus� souhlasit celkov� sou�et
        } else {  //Pokud byl zad�n pouze argument -n
          fprintf(stderr, "Nebyl zad�n parametr name!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-s") == 0) {  //Pokud je argument -s
        if(pocetArgs > (i+1)) {  //Pt�m se, jestli existuje dal�� a pokud ano
          strcpy(Surname, arguments[i+1]);  //Zkop�ruju krit�rium
          i++;  //Posunu se na dal�� argument
          hash += 2;  //P�i�tu k hash ��slo dva - mus� souhlasit celkov� sou�et
        } else {  //Pokud byl zad�n pouze argument -s
          fprintf(stderr, "Nebyl zad�n parametr surname!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-l") == 0) {  //Pokud je argument -l
        if(pocetArgs > (i+1)) {  //Pt�m se, jestli existuje dal�� a pokud ano
          strcpy(Login, arguments[i+1]);  //Zkop�ruju krit�rium
          i++;  //Posunu se na dal�� argument
          hash += 1;  //P�i�tu k hash ��slo jedna - mus� souhlasit celkov� sou�et
        } else {  //Pokud byl zad�n pouze argument -l
          fprintf(stderr, "Nebyl zad�n parametr login!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-f") == 0) {  //Pokud je argument -f
        if(pocetArgs > (i+1)) {  //Pt�m se, jestli existuje dal�� a pokud ano
          strcpy(Faculty, arguments[i+1]);  //Zkop�ruju krit�rium
          i++;  //Posunu se na dal�� argument
          hash += 4;  //P�i�tu k hash ��slo �ty�i - mus� souhlasit celkov� sou�et
        } else {  //Pokud byl zad�n pouze argument -f
          fprintf(stderr, "Nebyl zad�n parametr faculty!!!\n");  //Chyba
          return -1;
        }
      }
    }
  
    if((f = fopen("ipk_database.txt", "r")) == NULL) {
    //Otev�en� souboru s datab�z� pro �ten�
      fprintf(stderr, "Nepoda�ilo se otev��t soubor s datab�z�!!!\n");  //Chyba
      return -1;  //a vrac�m -1
    }
  
    while(fscanf(f, "%s", radek) != EOF) {  //Cyklem proch�z�m soubor po ��dc�ch
    
      CRC = 0;  //Kontroln� sou�et nastav�m na nulu
    
      char casti[5][80] = {"","","","",""};  //�et�zce pro ulo�en� ��st� URL
  
      if(regcomp(&re_compiled, reg_vyraz, REG_EXTENDED) != 0) {
      //Pokud se nepoda�ila kompilace RE
        fprintf(stderr, "Chyba p�i kompilaci regul�rn�ho v�razu!!!\n");  //Chyba
        return -1;  //A vrac� se -1
      }
      
      status = regexec(&re_compiled, radek, 6, re_struct, 0);  //Rozd�len� URL
      
      if(status == 0) {  //Pokud se rozd�len� URL poda�ilo, ulo��m si v�sledek
        strncpy(casti[0],radek+re_struct[1].rm_so,re_struct[1].rm_eo-re_struct[1].rm_so);
        strncpy(casti[1],radek+re_struct[2].rm_so,re_struct[2].rm_eo-re_struct[2].rm_so);
        strncpy(casti[2],radek+re_struct[3].rm_so,re_struct[3].rm_eo-re_struct[3].rm_so);
        strncpy(casti[3],radek+re_struct[4].rm_so,re_struct[4].rm_eo-re_struct[4].rm_so);
        strncpy(casti[4],radek+re_struct[5].rm_so,re_struct[5].rm_eo-re_struct[5].rm_so);
      } else {  //Kdy� se rozd�len� nezda�ilo
        regerror(status, &re_compiled, buffer, BUFFER_LEN);  //Zjist�m pro�
        fprintf(stderr, "Chyba regul�rn�ho v�razu: %s\n", buffer);  //Vyp�u chybu
        return -1;  //A vrac�m -1
      }
      
      if(strcmp(Login, casti[1]) == 0) {  //Pokud kontroluju login a souhlas�
        CRC += 1;  //Kontroln� sou�et zv���m o 1
      }
      
      if(strcmp(Surname, casti[2]) == 0) {  //Pokud kontroluju p��jmen� a souhlas�
        CRC += 2;  //Kontroln� sou�et zv���m o 2
      }
      
      if(strcmp(Name, casti[3]) == 0) {  //Pokud kontroluju jm�no a souhlas�
        CRC += 3;  //Kontroln� sou�et zv���m o 3
      }
      
      if(strcmp(Faculty, casti[4]) == 0) {  //Pokud kontroluju fakultu a souhlas�
        CRC += 4;  //Kontroln� sou�et zv���m o 4
      }
  
      if(CRC == hash) {  //Pokud souhlas� v�echna krit�ria
        bzero(msg,sizeof(msg));  //Vyma�u �et�zec msg
        strcpy(msg, radek);  //Zkop�ruju do msg ��dek z DB
        strcat(msg, "\n");  //P�id�m na konec '\n'
        //Po�lu ��dek klientovi
        if(send(t, msg, strlen(msg), 0) < 0) {  //Pokud se neposlala data
          fprintf(stderr, "Chyba funkce send!!!\n");  //Chyba
          return -1;  //a vrac�m -1
        }
      }
    }
  
    if(fclose(f) == EOF) {  //Pokud se nepoda�ilo zav��t soubor s datab�z�
      printf("Nezav�el se soubor!!!\n");  //Chyba
      return -1;  //a vrac�m -1
    }
  
    hash = 0;  //Nastav�m hash na nulu
  
    bzero(msg,sizeof(msg));  //Vyma�u �et�zec msg
  
    strcpy(Name, "");  //Vyma�u �et�zec Name
    strcpy(Surname, "");  //Vyma�u �et�zec Surname
    strcpy(Login, "");  //Vyma�u �et�zec Login
    strcpy(Faculty, "");  //Vyma�u �et�zec Faculty
/******************************************************************************/
    exit(0);
}
    if(close(t) < 0) {  //Pokud se nezav�el socket
      printf("error on close");  //Chyba
      return -1;  //a vrac�m -1
    }
  }  //Konec WHILE(1)

  if(close(sock) < 0) {  //Pokud se nezav�el socket
    printf("close");  //Chyba
    return -1;  //a vrac�m -1
  }

  return 0;  //�sp�n� konec programu
}
