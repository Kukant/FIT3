// Tomas Kukan, xkukan00
/**
 *  Tomas Kukan, xkukan00
 *  IPS 1
 **/

#include <stdio.h>
#include<unistd.h>
#include<thread>
#include<queue>
#include<mutex>
#include<vector>
#include <iostream>
#include<string.h>
#include<regex>



std::vector<std::mutex *> zamky; /* pole zamku promenne velikosti */
std::mutex * score_lock;
std::mutex * done_lock;
std::mutex * all_done_lock;
std::vector<std::regex> regexs;
std::vector<int> add_nums;

int score;
bool end;
char *line;
int num_of_done;
int num_threads;

char *read_line(int *res) {
	std::string line;
	char *str;
	if (std::getline(std::cin, line)) {
		str=(char *) malloc(sizeof(char)*(line.length()+1));
		strcpy(str,line.c_str());
		*res=1;
		return str;
	} else {
		*res=0;
		return NULL;
	}

}

int test_args(int argc, char *argv[])
{
    // test num of args
    if (argc % 2 != 0 || argc < 4)
        return -1;

    return (argc - 2) / 2;
}


void f(int ID) {
	while (end == false)
    {
        (*zamky[ID]).lock();

        if (end)
            break;
        if (std::regex_match(line, regexs[ID])) // pricti cislo ke skore
        {
            (*score_lock).lock();
            score += add_nums[ID];
            (*score_lock).unlock();
        }

        (*done_lock).lock();
        num_of_done++;
        (*done_lock).unlock();
        if (num_of_done == num_threads)
            (*all_done_lock).unlock();
    }

}

int main(int argc, char *argv[])
{
    num_threads = test_args(argc, argv);
    if (num_threads == -1)
    {
        //fprintf(stderr, "Wrong args.\n");
        return 1;
    }

    int min_score = atoi(argv[1]);
	/*******************************
	 * Inicializace threadu a zamku
	 * *****************************/
	std::vector <std::thread *> threads; /* pole threadu promenne velikosti */
    score_lock = new std::mutex();
    done_lock = new std::mutex();
    all_done_lock = new std::mutex();
    (*all_done_lock).lock();

	/* vytvorime zamky */
	zamky.resize(num_threads); //nastavime si velikost pole zamky
	for(int i=0;i<num_threads;i++)
	{
		std::mutex *new_zamek = new std::mutex();
		zamky[i]=new_zamek;
		(*(zamky[i])).lock();
	}
    // regex resizing
    regexs.resize(num_threads);
    add_nums.resize(num_threads);

	/* vytvorime thready */
	threads.resize(num_threads); /* nastavime si velikost pole threads */
	for(int i=0;i<num_threads;i++)
	{
		std::thread *new_thread = new std::thread (f,i);
		threads[i]=new_thread;
		std::regex reg(argv[2*i+2]);
		fflush(stdout);
		regexs[i] = reg;
		add_nums[i] = atoi(argv[2*i+3]);
	}
	/**********************************
	 * Vlastni vypocet pgrep
	 * ********************************/
	int res;
	line=read_line(&res);
	//print("main\n");
	while (res) {
        num_of_done = 0;
        score = 0;
        // spust vsechny thready
        for (int i = 0; i < num_threads; i++)
            (*(zamky[i])).unlock();

        // cekej nej vsechny skonci
        (*all_done_lock).lock();

        // vypis
        if (score >= min_score)
            printf("%s\n",line);

		free(line); /* uvolnim pamet */
		line=read_line(&res);
	}
	//print("end\n");

	end = true;

	// spust vsechny thready - nech je dojet
    for (int i = 0; i < num_threads; i++)
        (*(zamky[i])).unlock();

	/**********************************
	 * Uvolneni pameti
	 * ********************************/

	/* provedeme join a uvolnime pamet threads */
	for(int i=0;i<num_threads;i++)
	{
		(*(threads[i])).join();
		delete threads[i];
	}

	delete score_lock;
	delete done_lock;
	delete all_done_lock;
	 //uvolnime pamet zamku
	for(int i=0;i<num_threads;i++){
		delete zamky[i];
	}

}
