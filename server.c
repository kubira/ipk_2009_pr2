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
    Radim KUBI© (xkubis03)
*/

int main(int argc, char *argv[]) {

  int sock, t;  //Deskriptor socketu
  int port = -1;  //Èíslo portu
  int sinlen;  //Velikost sockaddr_in sin
  int i, index, n;  //Indexy, poèet pøijatých znakù
  int pocetArgs = 0;  //Poèet zadaných argumentù
  int CRC = 0, hash = 0;  //Kontrolní souèet správnosti hledaného záznamu
  int status;  //Status zpracování øádku regulárním výrazem
  struct sockaddr_in sin;  //Struktura socketu
  char msg[BUFFER_LEN];  //Zpráva od klienta
  struct hostent *hp;  //Struktura hosta
  char delims[] = " ";  //Oddìlovaè argumentù
  char *result = NULL;  //Výsledek rozdìlení argumentù
  char arguments[9][40];  //Argumenty
  char Name[80], Surname[80], Login[80], Faculty[80];  //Zadané kritéria
  char radek[80];  //Promìnná pro naètení jednoho øádku ze souboru
  char buffer[BUFFER_LEN];  //Øetìzec pro chybu pøi rozdìlování adresy serveru
  char *reg_vyraz = "^([0-9]+);(x[a-z]{5}[0-9]{2});([-A-Za-z]+);([A-Za-z]+);([A-Z]+)$";  //RE
  FILE *f;  //Deskriptor souboru
  regmatch_t re_struct[6];  //Struktura pro ulo¾ení øetìzcù z RE
  regex_t re_compiled;  //Zkompilovaný RE
  pid_t pidPotomka;
  
  if(argc == 1) {  //Pokud je jen jeden argument - nic nebylo zadáno
    fprintf(stderr, "Nebyly zadány argumenty!!!\n");  //Vypí¹i chybu
    return -1;  //a vracím -1
  }
  
  if(!strcmp(argv[1], "-p")) {
    if(argc >= 3) {
      port = atoi(argv[2]);
    } else {
      fprintf(stderr, "Nebylo zadáno èíslo portu!!!\n");
      return -1;
    }
  } else {
    fprintf(stderr, "Nebyl zadán argument port!!!\n");
    return -1;
  }
  
  strcpy(Name, "");  //Øetìzec pro kritérium jméno je zatím prázdný
  strcpy(Surname, "");  //Øetìzec pro kritérium pøíjmení je zatím prázdný
  strcpy(Login, "");  //Øetìzec pro kritérium login je zatím prázdný
  strcpy(Faculty, "");  //Øetìzec pro kritérium fakultu je zatím prázdný
  
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  //Vytvoøím socket
    fprintf(stderr, "Chyba vytvoøení socketu!!!\n");  //Chyba
    return -1;  //a vracím -1
  }
  
  sin.sin_family = PF_INET;  //Nastavení rodiny protokolù
  sin.sin_port = htons(port);  //Nastavení portu
  sin.sin_addr.s_addr  = INADDR_ANY;  //Nastavení adresy IP
  
  if(bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {  //Bind
    fprintf(stderr, "Chyba funkce bind!!!\n");  //Chyba
    return -1;  //a vracím -1
  }
  
  if(listen(sock, 5)) {  //Listen
    fprintf(stderr, "Chyba funkce listen!!!\n");  //Chyba
    return -1;  //a vracím -1
  }
  
  sinlen = sizeof(sin);  //Zjistím velikost sin
  
  while(1) {  //Cyklus pro pøíjem zpráv od klientù
    if((t = accept(sock, (struct sockaddr *)&sin, &sinlen)) < 0) {  //Accept
      fprintf(stderr, "Chyba funkce accept!!!\n");  //Chyba
      return -1;  //a vracím -1
    }
    
    pidPotomka = fork();
    
    if(pidPotomka == 0) {
/******************************************************************************/
    hp = (struct hostent *)gethostbyaddr((char *)&sin.sin_addr,4,AF_INET);
    //Zjistím hosta, kde jsem spu¹tìn
    
    bzero(msg,sizeof(msg));  //Vyma¾u øetìzec msg
    if((n = recv(t, msg, BUFFER_LEN, 0)) < 0) {  //Pøijmu parametry od clienta
      fprintf(stderr, "Chyba funkce recv!!!\n");  //Chyba
      return -1;  //a vracím -1
    }
    
    if(strcmp(msg, "END") == 0) {  //Pokud pøíjde zpráva END ukonèí se server
      pid_t rodic = getppid();
      kill(rodic, SIGKILL);
      return 0;  //Úspì¹né ukonèení programu
    }

    result = strtok(msg, delims);  //Zjistím první argument výpisu
  
    for(i = 0; i < 9; i++) {  //Cyklem procházím øetìzce argumentù výpisu
      strcpy(arguments[i], "");  //Zkopíruju do ka¾dého prázdný øetìzec
    }
  
    pocetArgs = 0;  //Nastavím poèet argumentù na nulu
  
    while(result != NULL) {  //Pokud existuje argument
      strcpy(arguments[pocetArgs], result);  //Zkopíruju ho do øetìzce
      result = strtok(NULL, delims);  //Získám dal¹í oddìlením
      pocetArgs++;  //Inkrementuji poèet argumentù
    }
  
    for(i = 0; i < pocetArgs; i++) {  //Procházím zadané argumenty v cyklu
      if(strcmp(arguments[i], "-n") == 0) {  //Pokud je argument -n
        if(pocetArgs > (i+1)) {  //Ptám se, jestli existuje dal¹í a pokud ano
          strcpy(Name, arguments[i+1]);  //Zkopíruju kritérium
          i++;  //Posunu se na dal¹í argument
          hash += 3;  //Pøiètu k hash èíslo tøi - musí souhlasit celkový souèet
        } else {  //Pokud byl zadán pouze argument -n
          fprintf(stderr, "Nebyl zadán parametr name!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-s") == 0) {  //Pokud je argument -s
        if(pocetArgs > (i+1)) {  //Ptám se, jestli existuje dal¹í a pokud ano
          strcpy(Surname, arguments[i+1]);  //Zkopíruju kritérium
          i++;  //Posunu se na dal¹í argument
          hash += 2;  //Pøiètu k hash èíslo dva - musí souhlasit celkový souèet
        } else {  //Pokud byl zadán pouze argument -s
          fprintf(stderr, "Nebyl zadán parametr surname!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-l") == 0) {  //Pokud je argument -l
        if(pocetArgs > (i+1)) {  //Ptám se, jestli existuje dal¹í a pokud ano
          strcpy(Login, arguments[i+1]);  //Zkopíruju kritérium
          i++;  //Posunu se na dal¹í argument
          hash += 1;  //Pøiètu k hash èíslo jedna - musí souhlasit celkový souèet
        } else {  //Pokud byl zadán pouze argument -l
          fprintf(stderr, "Nebyl zadán parametr login!!!\n");  //Chyba
          return -1;
        }
      } else if(strcmp(arguments[i], "-f") == 0) {  //Pokud je argument -f
        if(pocetArgs > (i+1)) {  //Ptám se, jestli existuje dal¹í a pokud ano
          strcpy(Faculty, arguments[i+1]);  //Zkopíruju kritérium
          i++;  //Posunu se na dal¹í argument
          hash += 4;  //Pøiètu k hash èíslo ètyøi - musí souhlasit celkový souèet
        } else {  //Pokud byl zadán pouze argument -f
          fprintf(stderr, "Nebyl zadán parametr faculty!!!\n");  //Chyba
          return -1;
        }
      }
    }
  
    if((f = fopen("ipk_database.txt", "r")) == NULL) {
    //Otevøení souboru s databází pro ètení
      fprintf(stderr, "Nepodaøilo se otevøít soubor s databází!!!\n");  //Chyba
      return -1;  //a vracím -1
    }
  
    while(fscanf(f, "%s", radek) != EOF) {  //Cyklem procházím soubor po øádcích
    
      CRC = 0;  //Kontrolní souèet nastavím na nulu
    
      char casti[5][80] = {"","","","",""};  //Øetìzce pro ulo¾ení èástí URL
  
      if(regcomp(&re_compiled, reg_vyraz, REG_EXTENDED) != 0) {
      //Pokud se nepodaøila kompilace RE
        fprintf(stderr, "Chyba pøi kompilaci regulárního výrazu!!!\n");  //Chyba
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
        return -1;  //A vracím -1
      }
      
      if(strcmp(Login, casti[1]) == 0) {  //Pokud kontroluju login a souhlasí
        CRC += 1;  //Kontrolní souèet zvý¹ím o 1
      }
      
      if(strcmp(Surname, casti[2]) == 0) {  //Pokud kontroluju pøíjmení a souhlasí
        CRC += 2;  //Kontrolní souèet zvý¹ím o 2
      }
      
      if(strcmp(Name, casti[3]) == 0) {  //Pokud kontroluju jméno a souhlasí
        CRC += 3;  //Kontrolní souèet zvý¹ím o 3
      }
      
      if(strcmp(Faculty, casti[4]) == 0) {  //Pokud kontroluju fakultu a souhlasí
        CRC += 4;  //Kontrolní souèet zvý¹ím o 4
      }
  
      if(CRC == hash) {  //Pokud souhlasí v¹echna kritéria
        bzero(msg,sizeof(msg));  //Vyma¾u øetìzec msg
        strcpy(msg, radek);  //Zkopíruju do msg øádek z DB
        strcat(msg, "\n");  //Pøidám na konec '\n'
        //Po¹lu øádek klientovi
        if(send(t, msg, strlen(msg), 0) < 0) {  //Pokud se neposlala data
          fprintf(stderr, "Chyba funkce send!!!\n");  //Chyba
          return -1;  //a vracím -1
        }
      }
    }
  
    if(fclose(f) == EOF) {  //Pokud se nepodaøilo zavøít soubor s databází
      printf("Nezavøel se soubor!!!\n");  //Chyba
      return -1;  //a vracím -1
    }
  
    hash = 0;  //Nastavím hash na nulu
  
    bzero(msg,sizeof(msg));  //Vyma¾u øetìzec msg
  
    strcpy(Name, "");  //Vyma¾u øetìzec Name
    strcpy(Surname, "");  //Vyma¾u øetìzec Surname
    strcpy(Login, "");  //Vyma¾u øetìzec Login
    strcpy(Faculty, "");  //Vyma¾u øetìzec Faculty
/******************************************************************************/
    exit(0);
}
    if(close(t) < 0) {  //Pokud se nezavøel socket
      printf("error on close");  //Chyba
      return -1;  //a vracím -1
    }
  }  //Konec WHILE(1)

  if(close(sock) < 0) {  //Pokud se nezavøel socket
    printf("close");  //Chyba
    return -1;  //a vracím -1
  }

  return 0;  //Úspì¹ný konec programu
}
