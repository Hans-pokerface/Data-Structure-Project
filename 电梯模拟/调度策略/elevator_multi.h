#ifndef ELEVATOR_MULTI_H
#define ELEVATOR_MULTI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== ���� ========== */
#define FLOORS         5
#define MAX_ELEVATORS  10   
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
#define UPToleranceBase 200
#define DOWNToleranceBase 160

#define GOING_UP       1
#define GOING_DOWN     2
#define IDLE           3
#define DOOR_OPENED    1
#define DOOR_CLOSED    0

#define DIR_UP         1
#define DIR_DOWN      -1
typedef struct {
    int appear_time;
    int tolerance;
    int in_floor;
    int out_floor;
} PassengerData;
#define DATASET_NUM 10
#define MAX_DATA_PASSENGERS 10000

extern PassengerData p_data[DATASET_NUM][MAX_DATA_PASSENGERS];
extern int p_count[DATASET_NUM];
/* ========== �¼����� ========== */
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

/* ========== ���ݽṹ ========== */
typedef struct Passenger {
    int id;
    int in_floor;
    int out_floor;
    int arrive_time;
    int giveup_time;
    int entered;       //乘客是否进入电梯
    int assigned_elev; //对于多梯模拟，存储欲分配的电梯编号
} Passenger;

// �����ڳ˿�ջ (����ȳ�)
typedef struct {
    Passenger* stack[CAPACITY];
    int top;
} ElevatorStack;

// ÿ��ȴ����� (ѭ�����У��Ƚ��ȳ�)
typedef struct {
    Passenger* queue[100];
    int front;
    int rear;
} WaitQueue;

// ����
typedef struct {
    int id;
    int floor;
    int state;          
    int door_state;
    int call_up[FLOORS];
    int call_down[FLOORS];
    int call_car[FLOORS];
    ElevatorStack passengers;
    int moving;
    int idle_start_time;
    WaitQueue wait_queues[FLOORS]; 
} Elevator;

// �¼�
typedef struct {
    int time;
    EventType type;
    int elev_id;   
    union {
        struct { Passenger* p; } giveup;
        struct { int floor; } elev_arrive;
    } data;
} Event;

/* ========== ȫ�ֱ������� ========== */
extern Elevator elevators[MAX_ELEVATORS];
extern int num_elevators;
extern int floor_up[FLOORS];
extern int floor_down[FLOORS];
extern int current_time;
extern int total_passengers;
extern int completed_passengers;
extern int giveup_passengers;
extern double loss;

/* ========== �������� ========== */
// ��ʼ��
void init_system(int elev_count);

// �Ѳ���
void heap_push(Event e);
Event heap_pop(void);
int heap_empty(void);
void schedule_event(int delay, EventType type, int elev_id, void* data);

// �˿����
void generate_passenger(void);
void passenger_giveup(Passenger* p);
Passenger* create_passenger(void);
int random_range(int min, int max);
int generate_giveup_time(int in, int out);
int generate_inter_time(void);

// ���ݺ����߼�
void process_event(Event* e);
void elevator_arrive(int elev_id, int floor);
void open_door(int elev_id);
void person_exit(int elev_id);
void person_enter(int elev_id);
void close_door(int elev_id);
void controller(int elev_id);
void continue_moving(int elev_id);
void move_to_floor(int elev_id, int target);
void idle_timeout(int elev_id);

// ��������Ϊָ��¥�������������ŵ���
typedef struct {
    double w_dist;
    double w_same_dir;
    double w_idle;
    double w_door_open;
    double w_load;
} DispatchWeights;
int dispatch_elevator(int floor, int direction, DispatchWeights *w);

#endif