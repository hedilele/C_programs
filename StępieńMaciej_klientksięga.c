#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>

//Makara potrzebne do projektu
#define MY_MSG_SIZE 64
#define USER_SIZE 64

key_t shmkey; //Zmienna do tworzenia klucza
int shmid; //Zmienna do tworzenia segmentu pamiec
struct shmid_ds buf; //Wpisywanie danych do pamieci
int liczba_slotow; //Zmienna do ilosci slotow pamieci
int tmp; //Zmienna pomocnicza
//Struktura przechowujaca dane
struct my_data
{	
	char user[USER_SIZE];
	char txt[MY_MSG_SIZE];
	int counter;
} *shared_data;

//Funkcja sprawdzajaca ilosc argumentow
void arguments(int argc)
{
	if(argc == 1)
	{
		printf("[Klient]: Brak argumentow programu!\n");
		exit(1);
	}
	else if(argc == 2)
	{
		printf("[Klient]: Nie podano nazwy uzytkownika!\n");
		exit(1);
	}
	else if(argc > 3)
	{
		printf("[Klient]: Podano za duzo argumentow!\n");
		exit(1);
	}
}
//MAIN
int main(int argc, char *argv[])
{	
	//Sprawdzenie ilosc argumentow
	arguments(argc);
	//Tworzenie klucza
	if( (shmkey = ftok(argv[1],1)) == -1)//ID?
	{
		printf("Blad tworzenia klucza!\n");
		exit(1);
	}

	//Otwieranie segmentu pamieci 
	if( (shmid = shmget(shmkey, 0, 0)) == -1)
	{
		printf(" blad shmget\n");
		exit(1);
	}

	//Dolaczanie segmentu pamieci wspolnej
	shared_data = (struct my_data*) shmat(shmid, (void *)0, 0);
	if(shared_data == (struct my_data *)-1)
	{
		printf(" blad shmat!\n");
		exit(1);
	}
	//Pobranie rozmiaru potrzebnego do obliczenia ilosci slotow
	shmctl(shmid, IPC_STAT, &buf);

	//Obliczenie ilosci slotow
	liczba_slotow = buf.shm_segsz / sizeof (struct my_data);

	//ROZPOCZECIE PRACY WLASCIWEJ
	//przypisanie licznika do zmiennej tymczasowej
	tmp = shared_data -> counter;
		printf("Klient ksiegi skarg i wnioskow wita!\n");
		//Wypisanie slotow i slotow aktualnych
		//sloty aktualne beda zmienialy sie po kazdym wpisie
		printf("[Wolnych %d wpisow (na %d)\n"
				,liczba_slotow - tmp, liczba_slotow);
		//Sprawdzenie czy w pamieci sa jeszcze wolne sloty
		if(liczba_slotow - tmp == 0)
		{
			printf("Brak wolnych slotow w pamieci!\n");
			exit(0);
		}
		printf("Napisz co ci doskwiera: \n");
		//Pobranie napisu od uzytkownika i usuniecie technicznie
		//znaku konca linii, by wyswietlanie bylo dobrze
		fgets(shared_data[tmp].txt, MY_MSG_SIZE, stdin);
		shared_data[tmp].txt[MY_MSG_SIZE -1] = '\0';
		//PO wpisie zwiekszam licznik
		(shared_data -> counter)++;

		//Skopiowanie nazwy uzytkownika na serwer i usuniecie 
		//znaku konca linii
		strcpy(shared_data[tmp].user, argv[2]);
		shared_data[tmp].user[USER_SIZE - 1] = '\0';
		printf("Dziekujemy za dokonanie wpisu\n");
		
		//Klient odlacza zmienna
		shmdt(shared_data);
		return 0;

}




