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
#define BUFFER_LEN 1000

/*
  Autor:
    Radim KUBI� (xkubis03)
*/

int main(int argc, char *argv[]) {

  int konec = 0;  //Ukon�it server???
  int sock;  //Deskriptor socketu
  int i, o, x, indexR = 0, index;  //Indexy �et�zc� a cykl�
  int port = -1;  //��slo portu
  int status;  //Status proveden� rozd�len� ��dku z datab�ze
  int name, surname, login, faculty;  //Jestli byli nastaveny v�stupn� sloupce
  struct sockaddr_in sin;  //Struktura pro socket
  struct hostent *hptr;  //Struktura pro hosta
  char msg[BUFFER_LEN] = "";  //Zpr�va pro server
  char radek[BUFFER_LEN] = "";  //�et�zec obsahuj�c� jeden ��dek z datab�ze
  char *URL, *poradi;  //Ukazatele pro Server a pro po�ad� sloupc�
  char Name[80], Surname[80], Login[80], Faculty[80];  //Krit�ria
  char buffer[BUFFER_LEN];  //�et�zec pro chybu p�i rozd�lov�n� adresy serveru
  char *reg_vyraz = "^([0-9]+);(x[a-z]{5}[0-9]{2});([-A-Za-z]+);([A-Za-z]+);([A-Z]+)$";  //RE
  regmatch_t re_struct[6];  //Struktura pro ulo�en� �et�zc� z RE
  regex_t re_compiled;  //Zkompilovan� RE
  
  name = surname = login = faculty = 0;  //Nebyl zat�m zad�n ��dn� z -NSLF
  
  strcpy(Name, "");  //�et�zec pro krit�rium jm�no je zat�m pr�zdn�
  strcpy(Surname, "");  //�et�zec pro krit�rium p��jmen� je zat�m pr�zdn�
  strcpy(Login, "");  //�et�zec pro krit�rium login je zat�m pr�zdn�
  strcpy(Faculty, "");  //�et�zec pro krit�rium fakultu je zat�m pr�zdn�
  
  URL = NULL;  //Ukazatel pro server zat�m nikam neukazuje
  poradi = NULL;  //Ukazatel pro po�ad� sloupc� zat�m nikam neukazuje
  
  if(argc == 1) {  //Pokud je jen jeden argument - nic nebylo zad�no
    fprintf(stderr, "Nebyly zad�ny argumenty!!!\n");  //Vyp�i chybu
    return -1;  //a vrac�m -1
  }
  
  for(i = 1; i < argc; i++) {  //Cyklem proch�z�m parametry programu
    if(strcmp(argv[i], "-h") == 0) {  //Pokud je -h
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        if((URL = (char*)malloc(sizeof(char)*(strlen(argv[i+1])+1))) != NULL) {
        //Naalokuju pam� pro n�zev serveru
          strcpy(URL, argv[i+1]);  //a zkop�ruji ho do URL
          i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
        } else {  //Kdy� se nepoda�ilo naalokovat pam�
          fprintf(stderr, "Nepoda�ilo se naalokovat pam�!!!\n");  //Vyp�i
          if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
            free(poradi);  //Uvoln�m po n�m pam�
          }
          return -1;  //a vrac�m -1
        }
      } else {  //Pokud byl zad�n pouze -h bez serveru
        fprintf(stderr, "Chyb� argument hostname!!!\n");  //Vyp�i chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if (strcmp(argv[i], "-p") == 0) {  //Pokud byl argument -p
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        port = atoi(argv[i+1]);  //P�evedu dal�� argument na ��slo portu
        i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
      } else {  //Pokud byl zad�n pouze -p bez portu
        fprintf(stderr, "Nebyl zad�n parametr port!!!\n");  //Vyp�i chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if(strcmp(argv[i], "-n") == 0) {  //Pokud byl argument -n
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        strcpy(Name, argv[i+1]);  //Zkop�ruju ho do �et�zce Name
        i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
      } else {  //Pokud byl zad�n pouze argument -n bez jm�na
        fprintf(stderr, "Nebyl zad�n parametr name!!!\n");  //Vyp�i chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if(strcmp(argv[i], "-s") == 0) {  //Pokud byl argument -s
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        strcpy(Surname, argv[i+1]);  //Zkop�ruju ho do �et�zce Surname
        i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
      } else {  //Pokud byl zad�n argument -s bez p��jmen�
        fprintf(stderr, "Nebyl zad�n parametr surname!!!\n");  //Vyp�i chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if(strcmp(argv[i], "-l") == 0) {  //Pokud byl argument -l
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        strcpy(Login, argv[i+1]);  //Zkop�ruju ho do �et�zce Login
        i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
      } else {  //Pokud byl zad�n argument -l bez loginu
        fprintf(stderr, "Nebyl zad�n parametr login!!!\n");  //Vyp�i chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if(strcmp(argv[i], "-f") == 0) {  //Pokud byl argument -f
      if(argc > (i+1)) {  //Pt�m se, jestli je za n�m dal�� a pokud ano
        strcpy(Faculty, argv[i+1]);  //Zkop�ruju ho do �et�zce Faculty
        i++;  //ji� jsem zpracoval n�sleduj�c� argument - posunu index
      } else {  //Pokud byl zad�n argument -f bez fakulty
        fprintf(stderr, "Nebyl zad�n parametr faculty!!!\n");  //Vyp�u chybu
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
          free(poradi);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
    } else if((argv[i][0] == '-') && (isupper(argv[i][1]) != 0)) {
    //Pokud byl zad�n argument za��naj�c� znakem '-' a za n�m je velk� p�smeno
      if((poradi = (char*)malloc(sizeof(char)*(strlen(argv[i])+1))) == NULL) {
      //Naalokuju si pam� pro po�ad� sloupc� v�pisu
        fprintf(stderr, "Nepoda�ilo se naalokovat pam�!!!\n");  //Chyba
        if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
          free(URL);  //Uvoln�m po n�m pam�
        }
        return -1;  //a vrac�m -1
      }
      for(index = 1; index < strlen(argv[i]); index++) {
      //Cyklem proch�z�m argument po�ad� sloupc�
        switch(argv[i][index]) {  //Rozhodnu podle znaku, kam j�t
          case 'L' : {  //Pokud bylo zad�no L
            login = 1;  //Pozna��m si, �e byl zad�n login
            strcat(poradi, "L");  //P�id�m do �et�zce po�ad� 'L'
            break;  //Ukon��m v�tev
          }
          case 'S' : {  //Pokud bylo zad�no S
            surname = 1;  //Pozna��m si, �e bylo zad�no p��jmen�
            strcat(poradi, "S");  //P�id�m do �et�zce po�ad� 'S'
            break;  //Ukon��m v�tev
          }
          case 'N' : {  //Pokud bylo zad�no N
            name = 1;  //Pozna��m si, �e bylo zad�no jm�no
            strcat(poradi, "N");  //P�id�m do �et�zce po�ad� 'N'
            break;  //Ukon��m v�tev
          }
          case 'F' : {  //Pokud bylo zad�no F
            faculty = 1;    //Pozna��m si, �e byla zad�na fakulta
            strcat(poradi, "F");  //P�id�m do �et�zce po�ad� 'F'
            break;  //Ukon��m v�tev
          }
        }
      }
    } else if(strcmp(argv[i], "END") == 0) {  //Pokud je argument END
      konec = 1;  //Nastav�m konec na hodnotu 1
    }
  }
  
  if(URL == NULL) {  //Pokud nebyl zad�n server
    fprintf(stderr, "Nebyl zad�n hostname!!!\n");  //Vyp�i chybu
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //a vrac�m -1
  }
  
  if(port == -1) {  //Pokud nebyl zad�n port
    fprintf(stderr, "Nebyl zad�n port!!!\n");  //Vyp�i chybu
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //a vrac�m -1
  }
  
  if(!(name || surname || login || faculty || konec)) {
  //Pokud nebyl zad�n argument -NSLF
    fprintf(stderr, "Nebyl zad�n ��dn� z argument� N S L F, bude pou�ito -NSLF!!!\n");
    //Vyp�i varov�n�
    if((poradi = (char*)malloc(sizeof(char)*5)) == NULL) {
    //Naalokuji pam� pro po�ad� sloupc�
      fprintf(stderr, "Nepoda�ilo se naalokovat pam�!!!\n");  //Vyp�i chybu
      if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
        free(URL);  //Uvoln�m po n�m pam�
      }
      return -1;  //a vrac�m -1
    }
    strcpy(poradi, "NSLF");  //Nastav�m implicitn� -NSLF
  }
  
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  //Vytvo�en� socketu
    fprintf(stderr, "Chyba vytvo�en� socketu!!!\n");  //Pokud nastala chyba
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //vrac�m -1
  }
  
  sin.sin_family = PF_INET;  //Nastav�m rodinu protokol�
  sin.sin_port = htons(port);  //Nastav�m port
  
  if((hptr =  gethostbyname(URL)) == NULL) {  //Zjist�m hosta
    fprintf(stderr, "Chyba funkce gethostbyname!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //a vrac�m -1
  }
  
  memcpy(&sin.sin_addr, hptr->h_addr, hptr->h_length);  //Zkop�ruju hosta
  
  if(connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {  //P�ipoj�m se
    fprintf(stderr, "Chyba funkce connect!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //a vrac�m -1
  }

  if(konec != 1) {  //Pokud nebudu ukon�ovat server
    for(i = 1; i < argc; i++) {
    //Cyklem proch�z�m argumenty a kop�ruji je do msg, kter� ode�lu na server
      if(strcmp(argv[i], "-h") == 0) {  //Argument -h server nezaj�m�
        i++;  //Posunu se na dal�� argument
        continue;  //Vynut�m dal�� krok cyklu
      }
      if(strcmp(argv[i], "-p") == 0) {  //Argument -p server nezaj�m�
        i++;  //Posunu se na dal�� argument
        continue;  //Vynut�m dal�� krok cyklu
      }
      strcat(msg, argv[i]);  //P�id�m argument do zpr�vy
      strcat(msg, " ");  //P�id�m do zpr�vy mezeru
    }
  } else {  //Pokud server budu ukon�ovat
    strcpy(msg, "END");  //Zkop�ruji do zpr�vy END
  }

  if(send(sock, msg, (strlen(msg)+1), 0) < 0) {  //Po�lu parametry na server
    fprintf(stderr, "Chyba funkce send!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    return -1;  //a vrac�m -1
  }
  
  if(konec == 1) {  //Pokud ukon�uji server
    if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
      free(URL);  //Uvoln�m po n�m pam�
    }
    if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
      free(poradi);  //Uvoln�m po n�m pam�
    }
    if(close(sock) < 0) { //Uzav�en� socketu
      fprintf(stderr, "Chyba p�i zav�r�n� socketu!!!\n");  //Chyba
      return -1;  //a vrac�m -1
    }
    return 0;  //�sp�n� ukon�en� programu
  }
  
  bzero(msg,sizeof(msg));  //Vyma�u �et�zec msg
  
  while((i = (recv(sock, msg, sizeof(msg), 0))) > 0) {  //P�ij�m�m data z serveru
    msg[i] = '\0';  //Nastav�m na konec zpr�vy nulov� znak
    for(x = 0; x < i; x++) {  //Proch�z�m zpr�vu po znac�ch
      if(msg[x] == '\n') {  //Pokud je znak '\n'
        radek[indexR] = '\0';  //Nastav�m na konec �et�zce nulov� znak

        char casti[5][80] = {"","","","",""};  //�et�zce pro ulo�en� ��st� URL

        if(regcomp(&re_compiled, reg_vyraz, REG_EXTENDED) != 0) {
        //Pokud se nepoda�ila kompilace RE
          fprintf(stderr, "Chyba p�i kompilaci regul�rn�ho v�razu!!!\n");  //Chyba
          if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
            free(URL);  //Uvoln�m po n�m pam�
          }
          if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
            free(poradi);  //Uvoln�m po n�m pam�
          }
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
          if(URL != NULL) {  //Pokud jsem ji� zpracoval argument -h
            free(URL);  //Uvoln�m po n�m pam�
          }
          if(poradi != NULL) {  //Pokud jsem ji� zpracoval argument -NSLF
            free(poradi);  //Uvoln�m po n�m pam�
          }
          return -1;  //A vrac�m -1
        }
        
        for(o = 0; o < strlen(poradi); o++) {  //Cyklem proch�z�m po�ad� sloupc�
            switch(poradi[o]) {  //Rozhodnu podle znaku, kam j�t
              case 'L' : {  //Pokud je znak 'L'
                printf("%s ", casti[1]);  //Vyp�u login
                break;  //Ukon��m v�tev
              }
              case 'S' : {  //Pokud je znak 'S'
                printf("%s ", casti[2]);  //Vyp�u p��jmen�
                break;  //Ukon��m v�tev
              }
              case 'N' : {  //Pokud je znak 'N'
                printf("%s ", casti[3]);  //Vyp�u jm�no
                break;  //Ukon��m v�tev
              }
              case 'F' : {  //Pokud je znak 'F'
                printf("%s ", casti[4]);  //Vyp�u fakultu
                break;  //Ukon��m v�tev
              }
            }
          }
        printf("\n");  //Vyp�u konec ��dku
        indexR = 0;  //a vynuluju index z�skan�ho ��dku
      } else {  //Pokud p�i�el jin� znak ne� '\n'
        radek[indexR] = msg[x];  //Ulo��m znak na pozici do ��dku
        indexR++;  //Zv���m pozici znaku
      }
    }
    bzero(msg,sizeof(msg));  //Vyma�u p��choz� zpr�vu
  }

  if(close(sock) < 0) { //Uzav�en� socketu
    fprintf(stderr, "Chyba p�i zav�r�n� socketu!!!\n");  //Chyba
    return -1;  //a vrac�m -1
  }

  return 0;  //�sp�n� ukon�en� programu
}
