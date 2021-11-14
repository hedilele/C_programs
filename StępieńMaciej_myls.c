#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#define BUFS 1024

struct tm czas;
struct dirent *direntp;
//Pobieranie uzytkownika
void user(struct stat statbuf)
{
	struct passwd *uzyt = getpwuid(statbuf.st_uid);
	if(uzyt == NULL)
	{
		perror("getpwuid");
		printf("%d ",statbuf.st_uid);
	}
	else
	printf(" %s ",uzyt->pw_name);
}
//Pobieranie grupy
void group(struct stat statbuf)
{
	struct group *grupa = getgrgid(statbuf.st_gid);
	if(grupa == NULL)
	{
		perror("getpwgid");
		printf("%d ",statbuf.st_gid);
	}
	else
	printf("%s",grupa->gr_name);
}
//Nadawanie rodzaju (plik,katalog,blok,link,itp.)Calosc w man 2 stat;
void permissions_other(struct stat statbuf)
{
	switch (statbuf.st_mode & S_IFMT)
	{
		case S_IFBLK:
			printf("b");
			break;
		case S_IFCHR:
			printf("c");
			break;
		case S_IFDIR:
			printf("d");
			break;
		case S_IFIFO:
			printf("f");
			break;
		case S_IFLNK:
			printf("l");
			break;
		case S_IFREG:
			printf("-");
			break;
		case S_IFSOCK:
			printf("n");
			break;
		default:
			printf("unknown?");
			break;
	}
}
//Uprawnienia dla uzytkownika,grupy i pozostalych;
void permissions(struct stat statbuf)
{
	//Uzytkownik
	printf((statbuf.st_mode & S_IRUSR) ? "r" : "-");
	printf((statbuf.st_mode & S_IWUSR) ? "w" : "-");
	printf((statbuf.st_mode & S_IXUSR) ? "x" : "-");
	//Grupa
	printf((statbuf.st_mode & S_IRGRP) ? "r" : "-");
	printf((statbuf.st_mode & S_IWGRP) ? "w" : "-");
	printf((statbuf.st_mode & S_IXGRP) ? "x" : "-");
	//Pozostali
	printf((statbuf.st_mode & S_IROTH) ? "r" : "-");
	printf((statbuf.st_mode & S_IWOTH) ? "w" : "-");
	printf((statbuf.st_mode & S_IXOTH) ? "x" : "-");
}
//Wyswietlanie czasu
void time_myls(struct stat statbuf)
{
	localtime_r(&statbuf.st_mtime, &czas);
	char buffer[80];
	strftime(buffer,80,"%m-%d %H:%M",&czas);
	printf(" %s", buffer);
}
//Switch case do zamiany nazw miesiecy(z angielskiego na polski)
void  months()
{
	switch(czas.tm_mon + 1)
	{
		case 1:
			printf("stycznia");
			break;
		case 2:
			printf("lutego");
			break;
		case 3:
			printf("marca");
			break;
		case 4:
			printf("kwietnia");
			break;
		case 5:
			printf("maja");
			break;
		case 6:
			printf("czerwca");
			break;
		case 7:
			printf("lipca");
			break;
		case 8:
			printf("sierpnia");
			break;
		case 9:
			printf("wrzesnia");
			break;
		case 10:
			printf("pazdziernika");
			break;
		case 11:
			printf("listopada");
			break;
		case 12:
			printf("grudnia");
			break;

	}
}
//Naiwny cat do wyswietlania zawartosci w pliku
int cat(char *argument)
{
	int plik,n;
	char bufor[BUFS];
	//Otwieranie pliku i zwracanie bledu w przypadku niepowodzenia
	plik = open(argument, O_RDONLY);
	if(plik == -1)
	{
		perror("Blad");
		exit(EXIT_FAILURE);
	}
	//Czytanie pliku
	while((n=read(plik,bufor,BUFS))>0)
	{
		write(fileno(stdout),bufor,n);
	}
	close(plik);
	return 0;
}
//Funkcja dla uprawnien w trybie drugim - skrocona wersja 1
void permissions_tryb_2(struct stat statbuf,char* argv)
{
	switch(statbuf.st_mode & S_IFMT)
	{
		case S_IFREG:
			printf("zwykly plik\n");
			break;
		case S_IFDIR:
			printf("katalog\n");
			break;
		case S_IFLNK:
			printf("link symboliczny\n");
			break;
	}
}
//Tryb drugi
int myls_tryb_2(struct stat sb,char *argument)
{
	lstat(argument,&sb);//Wazna funkcja do pobierania danych z argumentu
	char cwd[PATH_MAX];
	char bufor[BUFS];
	char bufer[80];
	printf("Informacje o %s: \n",argument);
	printf("Typ pliku:	");
	permissions_tryb_2(sb,argument);
	//Getcwd do pobierania sciezki
	if(getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("Sciezka:	%s/%s\n",getcwd(cwd,sizeof(cwd)),
				argument);
		//Jezeli jest linkiem to wyswietl na co wskazuje
		if((sb.st_mode & S_IFMT) == S_IFLNK)
		{
			readlink(argument,bufor,BUFS);
		        printf("Wskazuje na:    %s/%s\n",getcwd(cwd,sizeof(cwd))
					,bufor);
		}

	}else
	{
		perror("getcwd() error");
		return (EXIT_FAILURE);
	}
	printf("Rozmiar:	 %ld",sb.st_size);
	if(sb.st_size == 1)
	{
		printf(" bajt\n");
	}
	else if(sb.st_size <5)
	{
		printf(" bajty\n");
	}
	else
	{
		printf(" bajtow\n");
	}
	printf("Uprawnienia: ");
	permissions(sb);
	printf("\n");
	//Data ostatniego uzywania

	localtime_r(&sb.st_atime,&czas);
	strftime(bufer,80,"%d",&czas);
	printf("Ostatnio uzywany:	 %s ",bufer);
	months();
	strftime(bufer,80,"%Y roku o %H:%M:%S",&czas);
	printf(" %s\n",bufer);
	//Data ostatniej modyfikacji

	localtime_r(&sb.st_mtime,&czas);
	strftime(bufer,80,"%d",&czas);
	printf("Ostatnio modyfikowany: 		%s ",bufer);
	months();
	strftime(bufer,80,"%Y roku o %H:%M:%S",&czas);
	printf(" %s\n",bufer);
	//Data ostatniej zmiany stani

	localtime_r(&sb.st_ctime,&czas);
	strftime(bufer,80,"%d", &czas);
	printf("Ostatnio zmieniany stan:	 %s ",bufer);
	months();
	strftime(bufer,80,"%Y roku o %H:%M:%S",&czas);
	printf(" %s\n",bufer);
	//Jesli argument jest plikkiem,wyswietl zawartosc
	if((sb.st_mode & S_IFMT) ==  S_IFREG)
	{
		printf("Poczatek zawartosci: \n");
		cat(argument);
	}
	return 0;
}
//Main
int main(int argc, char **argv)
{
	DIR *dirp;
	char bufor[BUFS];
	struct stat st;
	dirp = opendir(".");
	//Czy plik pusty
	if(dirp == NULL)
	{
		perror("Blad");
	}
	//Petla while do wypisywania informacji
	while((direntp = readdir(dirp)) != NULL)
	{
		if(lstat(direntp->d_name, &st) == -1)
			continue;
		//Czytanie pierwszego trybu
		if(argc == 1)
		{
			permissions_other(st);
			permissions(st);
			printf(" %li",st.st_nlink);
			user(st);
			group(st);
			printf(" %ld",st.st_size);
			time_myls(st);
			if((st.st_mode & S_IFMT) == S_IFLNK)
			{
				readlink(direntp->d_name,bufor,BUFS);
				printf(" %s -> %s\n",direntp->d_name,bufor);
			}
			else
			{
				printf(" %s\n", direntp->d_name);
			}
		}
	}	//Tryb drugi
		if(argc > 1)
		{
			myls_tryb_2(st,argv[1]);
		}
		closedir(dirp);
		return 0;
}
