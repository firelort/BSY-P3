#include <wiringPi.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#define PINECHO 21
#define PINTRIG 22
#define PINBEWEGUNG 23


#define PINTON 25
#define ROT 0
#define GELB 2
#define GREEN 3

void *threadOne();
void *threadTwo();
void *threadThree();
void *threadFour();

sem_t semA, semB, messdatenZugriff, semC, semD;

double distance = 0;

int main() {
    static pthread_t threads[4] = {0};
    sem_init(&semA,0,0);
    sem_init(&semB,0,0);
    sem_init(&messdatenZugriff,0,0);
    sem_init(&semC,0,0);
    sem_init(&semD,0,0);

    sem_post(&messdatenZugriff);

    wiringPiSetup();

    pinMode(ROT, OUTPUT);
    pinMode(GELB, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(PINTRIG, OUTPUT);

    pinMode(PINECHO, INPUT);
    pinMode(PINBEWEGUNG, INPUT);


    pullUpDnControl(PINTRIG,PUD_OFF);
    pullUpDnControl(PINECHO,PUD_OFF);
    pullUpDnControl(23,PUD_OFF);

    softToneCreate(25);

    distance = 5;

    pthread_create(&threads[0], NULL, threadOne, NULL);
    pthread_create(&threads[1], NULL, threadTwo, NULL);
    pthread_create(&threads[2], NULL, threadThree, NULL);
    pthread_create(&threads[3], NULL, threadFour, NULL);

    sleep(15);

    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
    pthread_cancel(threads[2]);
    pthread_cancel(threads[3]);

    digitalWrite(ROT, 0);
    digitalWrite(GELB, 0);
    digitalWrite(GREEN, 0);
    digitalWrite(PINECHO, 0);
    digitalWrite(PINTRIG, 0);
    digitalWrite(23, 0);

    softToneWrite(PINTON,0);

    sem_destroy(&semA);
    sem_destroy(&semB);
    sem_destroy(&messdatenZugriff);
    sem_destroy(&semC);
    sem_destroy(&semD);

    return 0;
}

void *threadOne() {
    while (1) {
        if (digitalRead(23) == 1) {
		sem_post(&semB);
		sem_wait(&semA);
		sem_wait(&semA);
        }
    }

}

void *threadTwo() {
    double myvar = 0.0;
    double timediff;
    struct timeval start, ende;
    double sec, usec;
    while (1) {

        sem_wait(&semB);
        //Messung der Daten
        digitalWrite(PINTRIG, 1);
        delay(10);
        digitalWrite(PINTRIG, 0);

        if (gettimeofday(&start,(struct timezone*)0)) {
            printf("error\n");
            exit(1);
        }
        while (digitalRead(PINECHO) == 0) {
            if (gettimeofday(&start,(struct timezone *)0)) {
                printf("error\n");
                exit(1);
            }
        }
        while (digitalRead(PINECHO) == 1) {
            if (gettimeofday(&ende, (struct timezone *)0)) {
                printf("error\n");
                exit(1);
            }
        }

        sec = ende.tv_sec - start.tv_sec;
        usec = ende.tv_usec - start.tv_usec;

        timediff =(double) sec + (double) (usec / 1000000.0);
        myvar = (timediff * 34300) / 2;
        distance = myvar;

        sem_post(&semC);
        sem_post(&semD);
    }
}

void *threadThree() {
    double myvar = 0;
    while (1) {
        sem_wait(&semC);
        myvar = distance;
        digitalWrite(ROT,0);
        digitalWrite(GELB,0);
        digitalWrite(GREEN,0);
        if (myvar < 10)
            digitalWrite(ROT,1);
        else if (myvar < 60)
            digitalWrite(GELB,1);
        else
            digitalWrite(GREEN,1);
        sem_post(&semA);
    }
}

void *threadFour() {
    double myvar;
    while (1) {
        sem_wait(&semD);
        myvar = distance;
        if (myvar < 10) {
            softToneWrite(PINTON, 100);
            delay(300);
            softToneWrite(PINTON, 0);
        }
        sem_post(&semA);
    }
}
