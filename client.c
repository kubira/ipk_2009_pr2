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
    Radim KUBI© (xkubis03)
*/

int main(int argc, char *argv[]) {

  int konec = 0;  //Ukonèit server???
  int sock;  //Deskriptor socketu
  int i, o, x, indexR = 0, index;  //Indexy øetìzcù a cyklù
  int port = -1;  //Èíslo portu
  int status;  //Status provedení rozdìlení øádku z databáze
  int name, surname, login, faculty;  //Jestli byli nastaveny výstupní sloupce
  struct sockaddr_in sin;  //Struktura pro socket
  struct hostent *hptr;  //Struktura pro hosta
  char msg[BUFFER_LEN] = "";  //Zpráva pro server
  char radek[BUFFER_LEN] = "";  //Øetìzec obsahující jeden øádek z databáze
  char *URL, *poradi;  //Ukazatele pro Server a pro poøadí sloupcù
  char Name[80], Surname[80], Login[80], Faculty[80];  //Kritéria
  char buffer[BUFFER_LEN];  //Øetìzec pro chybu pøi rozdìlování adresy serveru
  char *reg_vyraz = "^([0-9]+);(x[a-z]{5}[0-9]{2});([-A-Za-z]+);([A-Za-z]+);([A-Z]+)$";  //RE
  regmatch_t re_struct[6];  //Struktura pro ulo¾ení øetìzcù z RE
  regex_t re_compiled;  //Zkompilovaný RE
  
  name = surname = login = faculty = 0;  //Nebyl zatím zadán ¾ádný z -NSLF
  
  strcpy(Name, "");  //Øetìzec pro kritérium jméno je zatím prázdný
  strcpy(Surname, "");  //Øetìzec pro kritérium pøíjmení je zatím prázdný
  strcpy(Login, "");  //Øetìzec pro kritérium login je zatím prázdný
  strcpy(Faculty, "");  //Øetìzec pro kritérium fakultu je zatím prázdný
  
  URL = NULL;  //Ukazatel pro server zatím nikam neukazuje
  poradi = NULL;  //Ukazatel pro poøadí sloupcù zatím nikam neukazuje
  
  if(argc == 1) {  //Pokud je jen jeden argument - nic nebylo zadáno
    fprintf(stderr, "Nebyly zadány argumenty!!!\n");  //Vypí¹i chybu
    return -1;  //a vracím -1
  }
  
  for(i = 1; i < argc; i++) {  //Cyklem procházím parametry programu
    if(strcmp(argv[i], "-h") == 0) {  //Pokud je -h
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        if((URL = (char*)malloc(sizeof(char)*(strlen(argv[i+1])+1))) != NULL) {
        //Naalokuju pamì» pro název serveru
          strcpy(URL, argv[i+1]);  //a zkopíruji ho do URL
          i++;  //ji¾ jsem zpracoval následující argument - posunu index
        } else {  //Kdy¾ se nepodaøilo naalokovat pamì»
          fprintf(stderr, "Nepodaøilo se naalokovat pamì»!!!\n");  //Vypí¹i
          if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
            free(poradi);  //Uvolním po nìm pamì»
          }
          return -1;  //a vracím -1
        }
      } else {  //Pokud byl zadán pouze -h bez serveru
        fprintf(stderr, "Chybí argument hostname!!!\n");  //Vypí¹i chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if (strcmp(argv[i], "-p") == 0) {  //Pokud byl argument -p
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        port = atoi(argv[i+1]);  //Pøevedu dal¹í argument na èíslo portu
        i++;  //ji¾ jsem zpracoval následující argument - posunu index
      } else {  //Pokud byl zadán pouze -p bez portu
        fprintf(stderr, "Nebyl zadán parametr port!!!\n");  //Vypí¹i chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if(strcmp(argv[i], "-n") == 0) {  //Pokud byl argument -n
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        strcpy(Name, argv[i+1]);  //Zkopíruju ho do øetìzce Name
        i++;  //ji¾ jsem zpracoval následující argument - posunu index
      } else {  //Pokud byl zadán pouze argument -n bez jména
        fprintf(stderr, "Nebyl zadán parametr name!!!\n");  //Vypí¹i chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if(strcmp(argv[i], "-s") == 0) {  //Pokud byl argument -s
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        strcpy(Surname, argv[i+1]);  //Zkopíruju ho do øetìzce Surname
        i++;  //ji¾ jsem zpracoval následující argument - posunu index
      } else {  //Pokud byl zadán argument -s bez pøíjmení
        fprintf(stderr, "Nebyl zadán parametr surname!!!\n");  //Vypí¹i chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if(strcmp(argv[i], "-l") == 0) {  //Pokud byl argument -l
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        strcpy(Login, argv[i+1]);  //Zkopíruju ho do øetìzce Login
        i++;  //ji¾ jsem zpracoval následující argument - posunu index
      } else {  //Pokud byl zadán argument -l bez loginu
        fprintf(stderr, "Nebyl zadán parametr login!!!\n");  //Vypí¹i chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if(strcmp(argv[i], "-f") == 0) {  //Pokud byl argument -f
      if(argc > (i+1)) {  //Ptám se, jestli je za ním dal¹í a pokud ano
        strcpy(Faculty, argv[i+1]);  //Zkopíruju ho do øetìzce Faculty
        i++;  //ji¾ jsem zpracoval následující argument - posunu index
      } else {  //Pokud byl zadán argument -f bez fakulty
        fprintf(stderr, "Nebyl zadán parametr faculty!!!\n");  //Vypí¹u chybu
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
          free(poradi);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
    } else if((argv[i][0] == '-') && (isupper(argv[i][1]) != 0)) {
    //Pokud byl zadán argument zaèínající znakem '-' a za ním je velké písmeno
      if((poradi = (char*)malloc(sizeof(char)*(strlen(argv[i])+1))) == NULL) {
      //Naalokuju si pamì» pro poøadí sloupcù výpisu
        fprintf(stderr, "Nepodaøilo se naalokovat pamì»!!!\n");  //Chyba
        if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
          free(URL);  //Uvolním po nìm pamì»
        }
        return -1;  //a vracím -1
      }
      for(index = 1; index < strlen(argv[i]); index++) {
      //Cyklem procházím argument poøadí sloupcù
        switch(argv[i][index]) {  //Rozhodnu podle znaku, kam jít
          case 'L' : {  //Pokud bylo zadáno L
            login = 1;  //Poznaèím si, ¾e byl zadán login
            strcat(poradi, "L");  //Pøidám do øetìzce poøadí 'L'
            break;  //Ukonèím vìtev
          }
          case 'S' : {  //Pokud bylo zadáno S
            surname = 1;  //Poznaèím si, ¾e bylo zadáno pøíjmení
            strcat(poradi, "S");  //Pøidám do øetìzce poøadí 'S'
            break;  //Ukonèím vìtev
          }
          case 'N' : {  //Pokud bylo zadáno N
            name = 1;  //Poznaèím si, ¾e bylo zadáno jméno
            strcat(poradi, "N");  //Pøidám do øetìzce poøadí 'N'
            break;  //Ukonèím vìtev
          }
          case 'F' : {  //Pokud bylo zadáno F
            faculty = 1;    //Poznaèím si, ¾e byla zadána fakulta
            strcat(poradi, "F");  //Pøidám do øetìzce poøadí 'F'
            break;  //Ukonèím vìtev
          }
        }
      }
    } else if(strcmp(argv[i], "END") == 0) {  //Pokud je argument END
      konec = 1;  //Nastavím konec na hodnotu 1
    }
  }
  
  if(URL == NULL) {  //Pokud nebyl zadán server
    fprintf(stderr, "Nebyl zadán hostname!!!\n");  //Vypí¹i chybu
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //a vracím -1
  }
  
  if(port == -1) {  //Pokud nebyl zadán port
    fprintf(stderr, "Nebyl zadán port!!!\n");  //Vypí¹i chybu
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //a vracím -1
  }
  
  if(!(name || surname || login || faculty || konec)) {
  //Pokud nebyl zadán argument -NSLF
    fprintf(stderr, "Nebyl zadán ¾ádný z argumentù N S L F, bude pou¾ito -NSLF!!!\n");
    //Vypí¹i varování
    if((poradi = (char*)malloc(sizeof(char)*5)) == NULL) {
    //Naalokuji pamì» pro poøadí sloupcù
      fprintf(stderr, "Nepodaøilo se naalokovat pamì»!!!\n");  //Vypí¹i chybu
      if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
        free(URL);  //Uvolním po nìm pamì»
      }
      return -1;  //a vracím -1
    }
    strcpy(poradi, "NSLF");  //Nastavím implicitní -NSLF
  }
  
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  //Vytvoøení socketu
    fprintf(stderr, "Chyba vytvoøení socketu!!!\n");  //Pokud nastala chyba
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //vracím -1
  }
  
  sin.sin_family = PF_INET;  //Nastavím rodinu protokolù
  sin.sin_port = htons(port);  //Nastavím port
  
  if((hptr =  gethostbyname(URL)) == NULL) {  //Zjistím hosta
    fprintf(stderr, "Chyba funkce gethostbyname!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //a vracím -1
  }
  
  memcpy(&sin.sin_addr, hptr->h_addr, hptr->h_length);  //Zkopíruju hosta
  
  if(connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {  //Pøipojím se
    fprintf(stderr, "Chyba funkce connect!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //a vracím -1
  }

  if(konec != 1) {  //Pokud nebudu ukonèovat server
    for(i = 1; i < argc; i++) {
    //Cyklem procházím argumenty a kopíruji je do msg, který ode¹lu na server
      if(strcmp(argv[i], "-h") == 0) {  //Argument -h server nezajímá
        i++;  //Posunu se na dal¹í argument
        continue;  //Vynutím dal¹í krok cyklu
      }
      if(strcmp(argv[i], "-p") == 0) {  //Argument -p server nezajímá
        i++;  //Posunu se na dal¹í argument
        continue;  //Vynutím dal¹í krok cyklu
      }
      strcat(msg, argv[i]);  //Pøidám argument do zprávy
      strcat(msg, " ");  //Pøidám do zprávy mezeru
    }
  } else {  //Pokud server budu ukonèovat
    strcpy(msg, "END");  //Zkopíruji do zprávy END
  }

  if(send(sock, msg, (strlen(msg)+1), 0) < 0) {  //Po¹lu parametry na server
    fprintf(stderr, "Chyba funkce send!!!\n");  //Chyba
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    return -1;  //a vracím -1
  }
  
  if(konec == 1) {  //Pokud ukonèuji server
    if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
      free(URL);  //Uvolním po nìm pamì»
    }
    if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
      free(poradi);  //Uvolním po nìm pamì»
    }
    if(close(sock) < 0) { //Uzavøení socketu
      fprintf(stderr, "Chyba pøi zavírání socketu!!!\n");  //Chyba
      return -1;  //a vracím -1
    }
    return 0;  //Úspì¹né ukonèení programu
  }
  
  bzero(msg,sizeof(msg));  //Vyma¾u øetìzec msg
  
  while((i = (recv(sock, msg, sizeof(msg), 0))) > 0) {  //Pøijímám data z serveru
    msg[i] = '\0';  //Nastavím na konec zprávy nulový znak
    for(x = 0; x < i; x++) {  //Procházím zprávu po znacích
      if(msg[x] == '\n') {  //Pokud je znak '\n'
        radek[indexR] = '\0';  //Nastavím na konec øetìzce nulový znak

        char casti[5][80] = {"","","","",""};  //Øetìzce pro ulo¾ení èástí URL

        if(regcomp(&re_compiled, reg_vyraz, REG_EXTENDED) != 0) {
        //Pokud se nepodaøila kompilace RE
          fprintf(stderr, "Chyba pøi kompilaci regulárního výrazu!!!\n");  //Chyba
          if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
            free(URL);  //Uvolním po nìm pamì»
          }
          if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
            free(poradi);  //Uvolním po nìm pamì»
          }
          return -1;  //A vrací se -1
        }
    
        status = regexec(&re_compiled, radek, 6, re_struct, 0);  //Rozdìlení URL
    
        if(status == 0) {  //Pokud se rozdìlení URL podaøilo, ulo¾ím si výsledek
          strncpy(casti[0],radek+re_struct[1].rm_so,re_struct[1].rm_eo-re_struct[1].rm_so);
          strncpy(casti[1],radek+re_struct[2].rm_so,re_struct[2].rm_eo-re_struct[2].rm_so);
          strncpy(casti[2],radek+re_struct[3].rm_so,re_struct[3].rm_eo-re_struct[3].rm_so);
          strncpy(casti[3],radek+re_struct[4].rm_so,re_struct[4].rm_eo-re_struct[4].rm_so);
          strncpy(casti[4],radek+re_struct[5].rm_so,re_struct[5].rm_eo-re_struct[5].rm_so);
        } else {  //Kdy¾ se rozdìlení nezdaøilo
          regerror(status, &re_compiled, buffer, BUFFER_LEN);  //Zjistím proè
          fprintf(stderr, "Chyba regulárního výrazu: %s\n", buffer);  //Vypí¹u chybu
          if(URL != NULL) {  //Pokud jsem ji¾ zpracoval argument -h
            free(URL);  //Uvolním po nìm pamì»
          }
          if(poradi != NULL) {  //Pokud jsem ji¾ zpracoval argument -NSLF
            free(poradi);  //Uvolním po nìm pamì»
          }
          return -1;  //A vracím -1
        }
        
        for(o = 0; o < strlen(poradi); o++) {  //Cyklem procházím poøadí sloupcù
            switch(poradi[o]) {  //Rozhodnu podle znaku, kam jít
              case 'L' : {  //Pokud je znak 'L'
                printf("%s ", casti[1]);  //Vypí¹u login
                break;  //Ukonèím vìtev
              }
              case 'S' : {  //Pokud je znak 'S'
                printf("%s ", casti[2]);  //Vypí¹u pøíjmení
                break;  //Ukonèím vìtev
              }
              case 'N' : {  //Pokud je znak 'N'
                printf("%s ", casti[3]);  //Vypí¹u jméno
                break;  //Ukonèím vìtev
              }
              case 'F' : {  //Pokud je znak 'F'
                printf("%s ", casti[4]);  //Vypí¹u fakultu
                break;  //Ukonèím vìtev
              }
            }
          }
        printf("\n");  //Vypí¹u konec øádku
        indexR = 0;  //a vynuluju index získaného øádku
      } else {  //Pokud pøi¹el jiný znak ne¾ '\n'
        radek[indexR] = msg[x];  //Ulo¾ím znak na pozici do øádku
        indexR++;  //Zvý¹ím pozici znaku
      }
    }
    bzero(msg,sizeof(msg));  //Vyma¾u pøíchozí zprávu
  }

  if(close(sock) < 0) { //Uzavøení socketu
    fprintf(stderr, "Chyba pøi zavírání socketu!!!\n");  //Chyba
    return -1;  //a vracím -1
  }

  return 0;  //Úspì¹né ukonèení programu
}
