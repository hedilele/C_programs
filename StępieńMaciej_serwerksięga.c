#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
//Makra potrzebne do projektu
#define MY_MSG_SIZE 64
#define USER_SIZE 64

key_t shmkey; //Zmienna do tworzenia klucza
int shmid; //Zmienna do tworzenia segmentu pamieci
int n; //Zmienna do przechowywania drugiego argumentu - ilosci slotow
struct shmid_ds buf; //wpisywanie danych do pamieci
//Struktura przechowujaca licznik i uzytkownika i wpis od uzytkownika
struct my_data
{
	char user[USER_SIZE];	
	char txt[MY_MSG_SIZE];
	int counter;
}*shared_data;

//Obsluga sygnalu - SIGINT
void sgnhandle(int signum)
{
	printf("\n[Serwer]: dostalem SIGINT => koncze i sprzatam...\n");
	printf(" (odlaczenie: %s, usuniecie: %s)\n",
			(shmdt(shared_data) == 0) ? "OK":"blad shmdt",
			(shmctl(shmid, IPC_RMID, 0) == 0)?"OK":"blad shmctl");
	exit(0);
}

//Obsluga sygnalu - SIGTSTP
void sgnhandle2(int signum)
{	
	if(shared_data->counter == 0) 
	{
		printf("\nKsiega skarg i wnioskow jest jeszcze pusta\n");
	}
	else
	{	
		printf("\n__________ Ksiega skarg i wnioskow: __________\n");
		int i;
		for(i = 0;i < shared_data -> counter;i++)
		{
		printf("[%s]: %s", shared_data[i].user,shared_data[i].txt);
		}
	}
}
//Funkcja do sprawdzania ilosci argumentow
void arguments(int argc)
{
	if(argc == 1)
	{
		printf("[Serwer]: Brak argumentow programu!\n");
		exit(1);
	}
	else if(argc == 2)
	{
		printf("[Serwer]: Nie podano rozmiaru ksiegi!\n");
		exit(1);
	}
	else if(argc == 3)
	{
		printf("[Serwer]: Podano pomyslnie argumenty\n");
	}
	else
	{
		printf("[Serwer]: Za duzo argumentow!\n");
		exit(1);
	}
}	

//Main
int main(int argc, char* argv[])
{
	//Sprawdzenie poprawnosci podanych argumentow
	arguments(argc);
	n = atoi(argv[2]); //Funkcja atoi do wyciecia liczby slotow z arg.

	//Wywolanie sygnalow
	signal(SIGINT, sgnhandle);
	signal(SIGTSTP, sgnhandle2);

	printf("[Serwer]: ksiega skarg i wnioskow (WARIANT C)\n");
	//Tworzenie klucza
	printf("[Serwer]: tworze klucz na podstawie pliku %s...",argv[1]);
	if( (shmkey = ftok(argv[1], 1)) == -1)
	{
		printf("Blad tworzenie klucza!\n");
		exit(1);
	}
	printf(" OK (klucz: %d)\n", shmkey);

	//Tworzenie segmentu pamieci
	printf("[Serwer]: tworze segment pamieci wspolnej dla ksiegi na %d wpisow po %db...", n, MY_MSG_SIZE+USER_SIZE);
	
	//Dodanie odpowiednich flag systemowych IPC_CREAT oraz IPC_EXCL
	if( (shmid = shmget(shmkey, sizeof(struct my_data)*n, 0600 | IPC_CREAT 
					| IPC_EXCL)) == -1)
	{
		printf("blad shmget!\n");
		exit(1);
	}
	shmctl(shmid, IPC_STAT, &buf);
	printf(" OK (id: %d, rozmiar: %zub)\n", shmid, buf.shm_segsz);
	printf("[Serwer]: dolaczam pamiec wspolna...");
	
	//Dolaczenie pamieci wspolnej
	shared_data = (struct my_data*) shmat(shmid, (void*)0, 0);
	if(shared_data == (struct my_data*) -1)
	{
		printf(" blad shmat!\n");
		exit(1);
	}
	printf(" OK (adres: %lX)\n", (long int)shared_data);
	//Wywolanie przyciskow = sygnalow do zakonczenia pracy
	//lub wyswietlenia zawartosci
	printf("[Serwer]: nacisnij Ctrl^Z by wyswietlic stan ksiegi\n");
	printf("[Serwer]: nacisnij Ctrl^C by zakonczyc program\n");
	//Busy waiting - serwer przechodzi w stan nasluchiwania
	while(8)
	{
		sleep(1);
	}
	return 0;
}
