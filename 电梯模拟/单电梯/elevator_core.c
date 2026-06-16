#include "elevator_sim.h"

/* 全局变量定义 */
Elevator elevator;
WaitQueue wait_queues[FLOORS];
int current_time = 0;
int total_passengers = 0;
int completed_passengers = 0;
int giveup_passengers = 0;

static Event* event_heap = NULL;
static int heap_size = 0;
static int heap_capacity = 0;

/* ---------- 堆操作 ---------- */
void heap_push(Event e) {
    if (heap_size >= heap_capacity) {
        heap_capacity = heap_capacity ? heap_capacity * 2 : 1000;
        event_heap = (Event*)realloc(event_heap, heap_capacity * sizeof(Event));
    }
    int i = heap_size++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (event_heap[parent].time <= e.time) break;
        event_heap[i] = event_heap[parent];
        i = parent;
    }
    event_heap[i] = e;
}

Event heap_pop(void) {
    Event top = event_heap[0];
    Event last = event_heap[--heap_size];
    int i = 0;
    while (i * 2 + 1 < heap_size) {
        int child = i * 2 + 1;
        if (child + 1 < heap_size && event_heap[child + 1].time < event_heap[child].time)
            child++;
        if (last.time <= event_heap[child].time) break;
        event_heap[i] = event_heap[child];
        i = child;
    }
    event_heap[i] = last;
    return top;
}

int heap_empty(void) {
    return heap_size == 0;
}

void schedule_event(int delay, EventType type, void* data) {
    Event e;
    e.time = current_time + delay;
    e.type = type;
    switch (type) {
    case EV_PASSENGER_ARRIVE:
        e.data.arrive.p = NULL;
        break;
    case EV_ELEV_ARRIVE:
        e.data.elev_arrive.floor = *(int*)data;
        free(data);
        break;
    case EV_DOOR_OPEN:
    case EV_PERSON_EXIT:
    case EV_PERSON_ENTER:
    case EV_DOOR_CLOSE:
    case EV_IDLE_BACK:
        e.data.door_open.elev_id = 0;
        break;
    case EV_GIVEUP:
        e.data.giveup.p = (Passenger*)data;
        break;
    }
    heap_push(e);
}

/* ---------- 初始化 ---------- */
void init_system(void) {
    srand((unsigned)time(NULL));
    current_time = 0;
    elevator.floor = 1;
    elevator.state = IDLE;
    elevator.door_state = DOOR_CLOSED;
    elevator.moving = 0;
    elevator.idle_start_time = 0;
    memset(elevator.call_up, 0, sizeof(elevator.call_up));
    memset(elevator.call_down, 0, sizeof(elevator.call_down));
    memset(elevator.call_car, 0, sizeof(elevator.call_car));
    elevator.passengers.top = -1;

    for (int i = 0; i < FLOORS; i++) {
        wait_queues[i].front = wait_queues[i].rear = 0;
    }

    heap_capacity = 1000;
    event_heap = (Event*)malloc(heap_capacity * sizeof(Event));
    heap_size = 0;
}

/* ---------- 电梯核心动作 ---------- */
void elevator_arrive(int floor) {
    elevator.floor = floor;
    elevator.moving = 0;
    printf("[%6d] Elevator arrives at floor %d\n", current_time, floor);

    int stop = 0;
    if (elevator.call_car[floor]) stop = 1;
    if (elevator.state == GOING_UP && elevator.call_up[floor]) stop = 1;
    if (elevator.state == GOING_DOWN && elevator.call_down[floor]) stop = 1;
    if (elevator.state == IDLE && (elevator.call_up[floor] || elevator.call_down[floor])) stop = 1;

    if (stop)
        schedule_event(DOOR_OPEN, EV_DOOR_OPEN, NULL);
    else
        continue_moving();
}

void open_door(void) {
    if (elevator.door_state == DOOR_OPENED) return;
    elevator.door_state = DOOR_OPENED;
    printf("[%6d] Elevator door opens\n", current_time);
    elevator.call_up[elevator.floor] = 0;
    elevator.call_down[elevator.floor] = 0;

    int has_exit = 0;
    for (int i = 0; i <= elevator.passengers.top; i++) {
        if (elevator.passengers.stack[i]->out_floor == elevator.floor) {
            has_exit = 1;
            break;
        }
    }
    if (has_exit) {
        schedule_event(EXIT_TIME, EV_PERSON_EXIT, NULL);
    }
    else {
        if (wait_queues[elevator.floor].front != wait_queues[elevator.floor].rear)
            schedule_event(ENTER_TIME, EV_PERSON_ENTER, NULL);
        else
            schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, NULL);
    }
}

void person_exit(void) {
    int found = -1;
    for (int i = elevator.passengers.top; i >= 0; i--) {
        if (elevator.passengers.stack[i]->out_floor == elevator.floor) {
            found = i;
            break;
        }
    }
    if (found >= 0) {
        Passenger* p = elevator.passengers.stack[found];
        for (int i = found; i < elevator.passengers.top; i++)
            elevator.passengers.stack[i] = elevator.passengers.stack[i + 1];
        elevator.passengers.top--;
        completed_passengers++;
        printf("[%6d] Passenger %d exits elevator (target floor %d)\n", current_time, p->id, p->out_floor);
        free(p);

        int still_has = 0;
        for (int i = 0; i <= elevator.passengers.top; i++) {
            if (elevator.passengers.stack[i]->out_floor == elevator.floor) {
                still_has = 1;
                break;
            }
        }
        if (still_has)
            schedule_event(EXIT_TIME, EV_PERSON_EXIT, NULL);
        else if (wait_queues[elevator.floor].front != wait_queues[elevator.floor].rear)
            schedule_event(ENTER_TIME, EV_PERSON_ENTER, NULL);
        else
            schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, NULL);
    }
}

void person_enter(void) {
    WaitQueue* wq = &wait_queues[elevator.floor];
    if (wq->front == wq->rear) {
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, NULL);
        return;
    }
    if (elevator.passengers.top >= CAPACITY - 1) {
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, NULL);
        return;
    }
    Passenger* p = wq->queue[wq->front++];
    if (wq->front >= 100) wq->front = 0;
    if (p->entered) {
        return;
    }
    p->entered = 1;
    elevator.passengers.stack[++elevator.passengers.top] = p;
    elevator.call_car[p->out_floor] = 1;
    printf("[%6d] Passenger %d enters elevator, target floor %d\n", current_time, p->id, p->out_floor);

    if (wq->front != wq->rear && elevator.passengers.top < CAPACITY - 1)
        schedule_event(ENTER_TIME, EV_PERSON_ENTER, NULL);
    else
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, NULL);
}

void close_door(void) {
    elevator.door_state = DOOR_CLOSED;
    printf("[%6d] Elevator door closes\n", current_time);
    elevator.call_car[elevator.floor] = 0;
    controller();
}

void controller(void) {
    int any_request = 0;
    for (int f = 0; f < FLOORS; f++) {
        if (elevator.call_up[f] || elevator.call_down[f] || elevator.call_car[f]) {
            any_request = 1;
            break;
        }
    }
    if (!any_request) {
        elevator.state = IDLE;
        elevator.idle_start_time = current_time;
        if (elevator.floor != 1)
            schedule_event(IDLE_TIMEOUT, EV_IDLE_BACK, NULL);
        return;
    }

    if (elevator.state == IDLE) {
        int best_floor = -1, best_dist = 999;
        for (int f = 0; f < FLOORS; f++) {
            if (elevator.call_up[f] || elevator.call_down[f] || elevator.call_car[f]) {
                int dist = abs(f - elevator.floor);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_floor = f;
                }
            }
        }
        if (best_floor != -1) {
            if (best_floor > elevator.floor) elevator.state = GOING_UP;
            else if (best_floor < elevator.floor) elevator.state = GOING_DOWN;
            else {
                schedule_event(DOOR_OPEN, EV_DOOR_OPEN, NULL);
                return;
            }
        }
    }
    continue_moving();
}

void continue_moving(void) {
    if (elevator.state == GOING_UP) {
        for (int f = elevator.floor + 1; f < FLOORS; f++) {
            if (elevator.call_up[f] || elevator.call_car[f] || (f == 1 && elevator.call_down[f])) {
                move_to_floor(f);
                return;
            }
        }
        elevator.state = GOING_DOWN;
        for (int f = elevator.floor - 1; f >= 0; f--) {
            if (elevator.call_down[f] || elevator.call_car[f] || (f == 1 && elevator.call_up[f])) {
                move_to_floor(f);
                return;
            }
        }
        elevator.state = IDLE;
        return;
    }
    else if (elevator.state == GOING_DOWN) {
        for (int f = elevator.floor - 1; f >= 0; f--) {
            if (elevator.call_down[f] || elevator.call_car[f] || (f == 1 && elevator.call_up[f])) {
                move_to_floor(f);
                return;
            }
        }
        elevator.state = GOING_UP;
        for (int f = elevator.floor + 1; f < FLOORS; f++) {
            if (elevator.call_up[f] || elevator.call_car[f] || (f == 1 && elevator.call_down[f])) {
                move_to_floor(f);
                return;
            }
        }
        elevator.state = IDLE;
        return;
    }
}

void move_to_floor(int target) {
    if (target == elevator.floor) {
        schedule_event(DOOR_OPEN, EV_DOOR_OPEN, NULL);
        return;
    }
    int travel = (target > elevator.floor) ? MOVE_UP : MOVE_DOWN;
    elevator.moving = 1;
    printf("[%6d] Elevator moving to floor %d\n", current_time, target);
    int* pfloor = (int*)malloc(sizeof(int));
    *pfloor = target;
    schedule_event(travel, EV_ELEV_ARRIVE, pfloor);
}

void idle_timeout(void) {
    if (elevator.state == IDLE && elevator.floor != 1 && !elevator.moving && elevator.door_state == DOOR_CLOSED) {
        printf("[%6d] Elevator idle timeout, return to floor 1\n", current_time);
        move_to_floor(1);
    }
}