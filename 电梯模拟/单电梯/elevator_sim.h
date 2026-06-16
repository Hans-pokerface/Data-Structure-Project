#ifndef ELEVATOR_SIM_H
#define ELEVATOR_SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FLOORS         5        
#define CAPACITY       15     
#define SIM_TIME       5000     

#define DOOR_OPEN      20
#define DOOR_CLOSE     20
#define ENTER_TIME     25
#define EXIT_TIME      25
#define MOVE_UP        51
#define MOVE_DOWN      61
#define IDLE_TIMEOUT   300

#define PROB_FLOOR1    0.5      
#define MAX_INTER_TIME 200
#define MIN_INTER_TIME 10
#define UPToleranceBase 50
#define DOWNToleranceBase 40

#define GOING_UP       1
#define GOING_DOWN     2
#define IDLE           3

#define DOOR_OPENED    1
#define DOOR_CLOSED    0

typedef enum {
    EV_PASSENGER_ARRIVE,
    EV_ELEV_ARRIVE,
    EV_DOOR_OPEN,
    EV_PERSON_EXIT,
    EV_PERSON_ENTER,
    EV_DOOR_CLOSE,
    EV_GIVEUP,
    EV_IDLE_BACK
} EventType;

/* ==================== 数据结构 ==================== */
typedef struct Passenger {
    int id;
    int in_floor;
    int out_floor;
    int arrive_time;      
    int giveup_time;    
    int entered;        
} Passenger;

typedef struct {
    Passenger* stack[CAPACITY];
    int top;
} ElevatorStack;

typedef struct {
    Passenger* queue[100];
    int front;
    int rear;
} WaitQueue;


typedef struct {
    int floor;              
    int state;             
    int door_state;        
    int call_up[FLOORS];
    int call_down[FLOORS];
    int call_car[FLOORS];
    ElevatorStack passengers;
    int moving;         
    int idle_start_time;   
} Elevator;


typedef struct {
    int time;
    EventType type;
    union {
        struct { Passenger* p; } arrive;
        struct { int floor; } elev_arrive;
        struct { int elev_id; } door_open;
        struct { int elev_id; } person_exit;
        struct { int elev_id; } person_enter;
        struct { int elev_id; } door_close;
        struct { Passenger* p; } giveup;
        struct { int elev_id; } idle_back;
    } data;
} Event;

/* ==================== 全局变量声明 ==================== */
extern Elevator elevator;
extern WaitQueue wait_queues[FLOORS];
extern int current_time;
extern int total_passengers;
extern int completed_passengers;
extern int giveup_passengers;

/* ==================== 函数声明 ==================== */
void init_system(void);

void heap_push(Event e);
Event heap_pop(void);
int heap_empty(void);
void schedule_event(int delay, EventType type, void* data);

void generate_passenger(void);
void passenger_giveup(Passenger* p);

void process_event(Event* e);
void elevator_arrive(int floor);
void open_door(void);
void person_exit(void);
void person_enter(void);
void close_door(void);
void controller(void);
void continue_moving(void);
void move_to_floor(int target);
void idle_timeout(void);

int random_range(int min, int max);
int generate_giveup_time(int in, int out);
int generate_inter_time(void);
Passenger* create_passenger(void);

#endif