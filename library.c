#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define ROOM_CAPACITY 4  //room seats 
#define ROOM_COUNT 10  //total room count
#define MAX_STUDENTS 100  //maximum number of students 

void *roomkeeper(void *);
void *student(void *);
void randwait();

sem_t room_semaphores[ROOM_COUNT];  //room semaphores
sem_t library_waiting;  //students waiting in library
sem_t mutex;  //mutex lock


int rooms[ROOM_COUNT];

int students[MAX_STUDENTS];
int r[ROOM_COUNT];
int roomkeeperStats[ROOM_COUNT];

int roomID = 0;

int main(int argc, char* argv[])
{
	//room threads 10
	pthread_t rtid[ROOM_COUNT];
	//student threads 100
	pthread_t stid[MAX_STUDENTS];

	int i;
	//room threads initialization counting from 4
	for(i = 0;i<ROOM_COUNT;i++){
		sem_init(room_semaphores+i, 0 , ROOM_CAPACITY);
	}
	
	sem_init(&mutex, 0, 1);
	//waiting room semaphore, all students can wait
	sem_init(&library_waiting, 0, 0);

	//array to count students in rooms
	for(i = 0; i < ROOM_COUNT; i++){
		rooms[i] = 0;
	}

	//room id array
	for(i = 0; i < ROOM_COUNT; i++){
		r[i] = i;
	}
	//student id array
	for(i = 0; i < MAX_STUDENTS; i++){
		students[i] = i;
	}
	for(i = 0; i < ROOM_COUNT; i++){
		roomkeeperStats[i] = 0;  // 0 = clean, 1= anounce, 2 = sleep
	}
	//room thread create
	for(i = 0; i < ROOM_COUNT; i++){
		pthread_create(&rtid[i], NULL, roomkeeper, (void *)&r[i]);
	}
	
	//student threads create
	for(i = 0; i < MAX_STUDENTS; i++){
		pthread_create(&stid[i], NULL, student, (void *)&students[i]);
		randwait(); //random create
	}

	//student threads join
	for(i = 0;i < MAX_STUDENTS;i++){
		pthread_join(stid[i],NULL);
	}
	//room threads join
	for(i = 0;i < ROOM_COUNT; i++){
		pthread_join(rtid[i], NULL);
	}
	

    return 0;
}

void *roomkeeper(void *roomkeeperID){

	int id = *(int *)roomkeeperID;  //current room/roomkeeper id
	int i;
	int value;  //value inside the semaphore
	sem_wait(&mutex);
	while(1)
	{
		sem_post(&library_waiting);   //get student from library waiting room
		sem_getvalue(&room_semaphores[id], &value);
		if(rooms[id] == 0){  //if room empty = clean
			printf("Roomkeeper %d is cleaning the room %d\n", id, id);
			sem_post(&mutex);
			break;
		}
		else if(value < ROOM_CAPACITY && rooms[id] < ROOM_CAPACITY){  //if there is space = anounce
			printf("Room %d has %d seats left!\n", id, (ROOM_CAPACITY - value));
			sem_post(&mutex);
			break;
		}
		else{ 
			sem_post(&mutex);
		}
		
		
	}

	return NULL;
}
void *student(void *studentID){


	int id = *(int *)studentID;
	

	int i;
	int roomCapacity;

	//enter the library waiting room
	sem_post(&library_waiting);
	
	sem_wait(&mutex);
	printf("Student %d entered the library.\n", id);

	while(1)
	{
        //loop for finding a room to enter
		for(i = 0; i < ROOM_COUNT;i++)
		{
			sem_getvalue(&room_semaphores[i], &roomCapacity);

			if(roomCapacity == ROOM_CAPACITY){  //if room empty
				roomID = i;
				roomkeeperStats[roomID] = 1;
                printf("--------ENTERING ROOM %d--------\n",i);
				printf("Room %d has %d seats left!\n", i, roomCapacity);
				sem_post(&mutex);
				break;
			} 
            //if room has seats
			else if(roomCapacity < ROOM_CAPACITY){
				roomID = i;
				roomkeeperStats[roomID] = 1;
				printf("Room %d has %d seats left!\n", i, roomCapacity);
				sem_post(&mutex);
				break;
				
			}

			//room full find another
			else if(rooms[roomID] == 0){
				roomkeeperStats[roomID] = 2;
				continue;	
			}

		}

        //put student in the room
		if(rooms[roomID] < ROOM_CAPACITY)
		{
			sem_wait(&room_semaphores[roomID]);
			rooms[roomID] += 1;
			printf("Student %d in the room %d\n", id, roomID);
			sem_post(&mutex);
			break;
		}
		else if(rooms[roomID] == ROOM_CAPACITY){
			randwait();
			sem_post(&mutex);
			sem_post(&room_semaphores[roomID]);
			sem_post(&room_semaphores[roomID]);
			sem_post(&room_semaphores[roomID]);
			sem_post(&room_semaphores[roomID]);
			rooms[roomID] = 0;
			
			printf("Room %d is empty now, Roomkeeper %d is cleaning the room.\n", roomID, roomID);
            printf("--------LEAVING ROOM %d--------\n", roomID);
        	break;
		}
	}
	
	
	return NULL;
	
	
}
void randwait(){
	int len = 4;
	sleep(rand() % len);
}
