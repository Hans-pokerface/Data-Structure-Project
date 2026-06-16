#include "elevator_multi.h"

/* 全局变量定义 */
Elevator elevators[MAX_ELEVATORS];
int num_elevators = 3;
int floor_up[FLOORS] = { 0 };
int floor_down[FLOORS] = { 0 };
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
        heap_capacity = heap_capacity ? heap_capacity * 2 : 2000;
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

void schedule_event(int delay, EventType type, int elev_id, void* data) {
    Event e;
    e.time = current_time + delay;
    e.type = type;
    e.elev_id = elev_id;
    switch (type) {
    case EV_ELEV_ARRIVE:
        e.data.elev_arrive.floor = *(int*)data;
        free(data);
        break;
    case EV_GIVEUP:
        e.data.giveup.p = (Passenger*)data;
        break;
    default:
        e.data.elev_arrive.floor = 0;
        break;
    }
    heap_push(e);
}

/* ---------- 初始化 ---------- */
void init_system(int elev_count) {
    srand((unsigned)time(NULL));
    current_time = 0;
    num_elevators = elev_count > MAX_ELEVATORS ? MAX_ELEVATORS : elev_count;

    for (int i = 0; i < num_elevators; i++) {
        elevators[i].id = i;
        elevators[i].floor = 1;
        elevators[i].state = IDLE;
        elevators[i].door_state = DOOR_CLOSED;
        elevators[i].moving = 0;
        elevators[i].idle_start_time = 0;
        memset(elevators[i].call_up, 0, sizeof(elevators[i].call_up));
        memset(elevators[i].call_down, 0, sizeof(elevators[i].call_down));
        memset(elevators[i].call_car, 0, sizeof(elevators[i].call_car));
        elevators[i].passengers.top = -1;
        for (int f = 0; f < FLOORS; f++) {
            elevators[i].wait_queues[f].front = elevators[i].wait_queues[f].rear = 0;
        }
    }

    memset(floor_up, 0, sizeof(floor_up));
    memset(floor_down, 0, sizeof(floor_down));

    heap_capacity = 2000;
    event_heap = (Event*)malloc(heap_capacity * sizeof(Event));
    heap_size = 0;
}

/* ---------- 电梯核心动作 ---------- */
void elevator_arrive(int elev_id, int floor) {
    Elevator* e = &elevators[elev_id];
    e->floor = floor;
    e->moving = 0;
    printf("[%6d] Elevator %d arrives at floor %d\n", current_time, elev_id, floor);

    int stop = 0;
    if (e->call_car[floor]) stop = 1;
    if (e->state == GOING_UP && e->call_up[floor]) stop = 1;
    if (e->state == GOING_DOWN && e->call_down[floor]) stop = 1;
    if (e->state == IDLE && (e->call_up[floor] || e->call_down[floor])) stop = 1;

    if (stop)
        schedule_event(DOOR_OPEN, EV_DOOR_OPEN, elev_id, NULL);
    else
        continue_moving(elev_id);
}

void open_door(int elev_id) {
    Elevator* e = &elevators[elev_id];
    if (e->door_state == DOOR_OPENED) return;
    e->door_state = DOOR_OPENED;
    printf("[%6d] Elevator %d door opens\n", current_time, elev_id);
    e->call_up[e->floor] = 0;
    e->call_down[e->floor] = 0;

    int has_exit = 0;
    for (int i = 0; i <= e->passengers.top; i++) {
        if (e->passengers.stack[i]->out_floor == e->floor) {
            has_exit = 1;
            break;
        }
    }
    if (has_exit) {
        schedule_event(EXIT_TIME, EV_PERSON_EXIT, elev_id, NULL);
    }
    else {
        if (e->wait_queues[e->floor].front != e->wait_queues[e->floor].rear)
            schedule_event(ENTER_TIME, EV_PERSON_ENTER, elev_id, NULL);
        else
            schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, elev_id, NULL);
    }
}

void person_exit(int elev_id) {
    Elevator* e = &elevators[elev_id];
    int found = -1;
    for (int i = e->passengers.top; i >= 0; i--) {
        if (e->passengers.stack[i]->out_floor == e->floor) {
            found = i;
            break;
        }
    }
    if (found >= 0) {
        Passenger* p = e->passengers.stack[found];
        for (int i = found; i < e->passengers.top; i++)
            e->passengers.stack[i] = e->passengers.stack[i + 1];
        e->passengers.top--;
        completed_passengers++;
        printf("[%6d] Passenger %d exits elevator %d (target floor %d)\n",
            current_time, p->id, elev_id, p->out_floor);
        free(p);

        int still_has = 0;
        for (int i = 0; i <= e->passengers.top; i++) {
            if (e->passengers.stack[i]->out_floor == e->floor) {
                still_has = 1;
                break;
            }
        }
        if (still_has)
            schedule_event(EXIT_TIME, EV_PERSON_EXIT, elev_id, NULL);
        else if (e->wait_queues[e->floor].front != e->wait_queues[e->floor].rear)
            schedule_event(ENTER_TIME, EV_PERSON_ENTER, elev_id, NULL);
        else
            schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, elev_id, NULL);
    }
}

void person_enter(int elev_id) {
    Elevator* e = &elevators[elev_id];
    WaitQueue* wq = &e->wait_queues[e->floor];
    if (wq->front == wq->rear) {
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, elev_id, NULL);
        return;
    }
    if (e->passengers.top >= CAPACITY - 1) {
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, elev_id, NULL);
        return;
    }
    Passenger* p = wq->queue[wq->front++];
    if (wq->front >= 100) wq->front = 0;
    if (p->entered) return;
    p->entered = 1;
    e->passengers.stack[++e->passengers.top] = p;
    e->call_car[p->out_floor] = 1;
    printf("[%6d] Passenger %d enters elevator %d, target floor %d\n",
        current_time, p->id, elev_id, p->out_floor);

    if (wq->front != wq->rear && e->passengers.top < CAPACITY - 1)
        schedule_event(ENTER_TIME, EV_PERSON_ENTER, elev_id, NULL);
    else
        schedule_event(DOOR_CLOSE, EV_DOOR_CLOSE, elev_id, NULL);
}

void close_door(int elev_id) {
    Elevator* e = &elevators[elev_id];
    e->door_state = DOOR_CLOSED;
    printf("[%6d] Elevator %d door closes\n", current_time, elev_id);
    e->call_car[e->floor] = 0;
    controller(elev_id);
}

void controller(int elev_id) {
    Elevator* e = &elevators[elev_id];
    int any_request = 0;
    for (int f = 0; f < FLOORS; f++) {
        if (e->call_up[f] || e->call_down[f] || e->call_car[f]) {
            any_request = 1;
            break;
        }
    }
    if (!any_request) {
        e->state = IDLE;
        e->idle_start_time = current_time;
        if (e->floor != 1)
            schedule_event(IDLE_TIMEOUT, EV_IDLE_BACK, elev_id, NULL);
        return;
    }

    if (e->state == IDLE) {
        int best_floor = -1, best_dist = 999;
        for (int f = 0; f < FLOORS; f++) {
            if (e->call_up[f] || e->call_down[f] || e->call_car[f]) {
                int dist = abs(f - e->floor);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_floor = f;
                }
            }
        }
        if (best_floor != -1) {
            if (best_floor > e->floor) e->state = GOING_UP;
            else if (best_floor < e->floor) e->state = GOING_DOWN;
            else {
                schedule_event(DOOR_OPEN, EV_DOOR_OPEN, elev_id, NULL);
                return;
            }
        }
    }
    continue_moving(elev_id);
}

void continue_moving(int elev_id) {
    Elevator* e = &elevators[elev_id];
    if (e->state == GOING_UP) {
        for (int f = e->floor + 1; f < FLOORS; f++) {
            if (e->call_up[f] || e->call_car[f] || (f == 1 && e->call_down[f])) {
                move_to_floor(elev_id, f);
                return;
            }
        }
        e->state = GOING_DOWN;
        for (int f = e->floor - 1; f >= 0; f--) {
            if (e->call_down[f] || e->call_car[f] || (f == 1 && e->call_up[f])) {
                move_to_floor(elev_id, f);
                return;
            }
        }
        e->state = IDLE;
        return;
    }
    else if (e->state == GOING_DOWN) {
        for (int f = e->floor - 1; f >= 0; f--) {
            if (e->call_down[f] || e->call_car[f] || (f == 1 && e->call_up[f])) {
                move_to_floor(elev_id, f);
                return;
            }
        }
        e->state = GOING_UP;
        for (int f = e->floor + 1; f < FLOORS; f++) {
            if (e->call_up[f] || e->call_car[f] || (f == 1 && e->call_down[f])) {
                move_to_floor(elev_id, f);
                return;
            }
        }
        e->state = IDLE;
        return;
    }
}

void move_to_floor(int elev_id, int target) {
    Elevator* e = &elevators[elev_id];
    if (target == e->floor) {
        schedule_event(DOOR_OPEN, EV_DOOR_OPEN, elev_id, NULL);
        return;
    }
    int travel = (target > e->floor) ? MOVE_UP : MOVE_DOWN;
    e->moving = 1;
    printf("[%6d] Elevator %d moving to floor %d\n", current_time, elev_id, target);
    int* pfloor = (int*)malloc(sizeof(int));
    *pfloor = target;
    schedule_event(travel, EV_ELEV_ARRIVE, elev_id, pfloor);
}

void idle_timeout(int elev_id) {
    Elevator* e = &elevators[elev_id];
    if (e->state == IDLE && e->floor != 1 && !e->moving && e->door_state == DOOR_CLOSED) {
        printf("[%6d] Elevator %d idle timeout, return to floor 1\n", current_time, elev_id);
        move_to_floor(elev_id, 1);
    }
}

/* ---------- 调度器：为指定楼层和方向分配最优电梯 ---------- */
int dispatch_elevator(int floor, int direction) {
    int best_id = -1;
    int best_score = 999999;

    for (int i = 0; i < num_elevators; i++) {
        Elevator* e = &elevators[i];
        int score = 0;
        int dist = abs(e->floor - floor);
        score += dist * 10;
        if (e->state == GOING_UP && direction == DIR_UP && e->floor <= floor)
            score -= 30;
        else if (e->state == GOING_DOWN && direction == DIR_DOWN && e->floor >= floor)
            score -= 30;
        if (e->state == IDLE)
            score -= 100;
        if (e->door_state == DOOR_OPENED)
            score += 200;
        score += e->passengers.top * 5;

        if (score < best_score) {
            best_score = score;
            best_id = i;
        }
    }
    return best_id;
}