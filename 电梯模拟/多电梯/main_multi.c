#include "elevator_multi.h"

/* ---------- 辅助函数 ---------- */
int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

int generate_giveup_time(int in, int out) {
    int span = abs(in - out);
    int base = (in > out) ? DOWNToleranceBase : UPToleranceBase;
    int extra = rand() % (span * 20 + 1);
    int time = base * span + extra;
    if (time < 20) time = 20;
    return time;
}

int generate_inter_time(void) {
    return random_range(MIN_INTER_TIME, MAX_INTER_TIME);
}

Passenger* create_passenger(void) {
    static int next_id = 0;
    Passenger* p = (Passenger*)malloc(sizeof(Passenger));
    p->id = next_id++;
    p->entered = 0;
    p->assigned_elev = -1;

    double r = (double)rand() / RAND_MAX;
    if (r < PROB_FLOOR1 / 2) {
        p->in_floor = 1;
        do { p->out_floor = rand() % FLOORS; } while (p->out_floor == 1);
    }
    else if (r < PROB_FLOOR1) {
        p->out_floor = 1;
        do { p->in_floor = rand() % FLOORS; } while (p->in_floor == 1);
    }
    else {
        do {
            p->in_floor = rand() % FLOORS;
            p->out_floor = rand() % FLOORS;
        } while (p->in_floor == p->out_floor);
    }

    p->arrive_time = current_time;
    p->giveup_time = current_time + generate_giveup_time(p->in_floor, p->out_floor);
    return p;
}

/* ---------- 乘客到达处理 ---------- */
void generate_passenger(void) {
    Passenger* p = create_passenger();
    total_passengers++;

    int direction = (p->out_floor > p->in_floor) ? DIR_UP : DIR_DOWN;
    int elev_id = dispatch_elevator(p->in_floor, direction);
    if (elev_id == -1) elev_id = 0; 
    p->assigned_elev = elev_id;

    Elevator* e = &elevators[elev_id];
    WaitQueue* wq = &e->wait_queues[p->in_floor];
    wq->queue[wq->rear++] = p;
    if (wq->rear >= 100) wq->rear = 0;

    if (direction == DIR_UP)
        e->call_up[p->in_floor] = 1;
    else
        e->call_down[p->in_floor] = 1;

    printf("[%6d] Passenger %d arrives at floor %d -> floor %d, tolerance = %d, assigned to elevator %d\n",
        current_time, p->id, p->in_floor, p->out_floor,
        p->giveup_time - current_time, elev_id);

    schedule_event(p->giveup_time - current_time, EV_GIVEUP, -1, p);

    if (e->state == IDLE && !e->moving && e->door_state == DOOR_CLOSED)
        controller(elev_id);

    int next_delay = generate_inter_time();
    schedule_event(next_delay, EV_PASSENGER_ARRIVE, -1, NULL);
}

/* ---------- 放弃处理 ---------- */
void passenger_giveup(Passenger* p) {
    if (p->entered) return;
    int elev_id = p->assigned_elev;
    if (elev_id < 0 || elev_id >= num_elevators) return;

    Elevator* e = &elevators[elev_id];
    WaitQueue* wq = &e->wait_queues[p->in_floor];
    int idx = wq->front;
    int found = 0;
    while (idx != wq->rear) {
        if (wq->queue[idx] == p) {
            found = 1;
            break;
        }
        idx++;
        if (idx >= 100) idx = 0;
    }
    if (!found) return;

    Passenger* new_queue[100];
    int new_front = 0, new_rear = 0;
    idx = wq->front;
    while (idx != wq->rear) {
        if (wq->queue[idx] != p) {
            new_queue[new_rear++] = wq->queue[idx];
            if (new_rear >= 100) new_rear = 0;
        }
        idx++;
        if (idx >= 100) idx = 0;
    }
    memcpy(wq->queue, new_queue, sizeof(Passenger*) * 100);
    wq->front = 0;
    wq->rear = new_rear;

    giveup_passengers++;
    printf("[%6d] Passenger %d gives up waiting (timeout) for elevator %d\n",
        current_time, p->id, elev_id);
    free(p);
}

/* ---------- 事件分发 ---------- */
void process_event(Event* e) {
    switch (e->type) {
    case EV_PASSENGER_ARRIVE:
        generate_passenger();
        break;
    case EV_ELEV_ARRIVE:
        elevator_arrive(e->elev_id, e->data.elev_arrive.floor);
        break;
    case EV_DOOR_OPEN:
        open_door(e->elev_id);
        break;
    case EV_PERSON_EXIT:
        person_exit(e->elev_id);
        break;
    case EV_PERSON_ENTER:
        person_enter(e->elev_id);
        break;
    case EV_DOOR_CLOSE:
        close_door(e->elev_id);
        break;
    case EV_GIVEUP:
        passenger_giveup(e->data.giveup.p);
        break;
    case EV_IDLE_BACK:
        idle_timeout(e->elev_id);
        break;
    }
}

/* ---------- 主函数 ---------- */
int main(int argc, char* argv[]) {
    int elev_count = 3;
    if (argc > 1) elev_count = atoi(argv[1]);
    if (elev_count < 1) elev_count = 1;
    if (elev_count > MAX_ELEVATORS) elev_count = MAX_ELEVATORS;

    init_system(elev_count);

    int first_delay = random_range(MIN_INTER_TIME, MAX_INTER_TIME);
    schedule_event(first_delay, EV_PASSENGER_ARRIVE, -1, NULL);

    while (!heap_empty() && current_time < SIM_TIME) {
        Event e = heap_pop();
        current_time = e.time;
        process_event(&e);
    }

    printf("\n========== Multi-Elevator Simulation Finished ==========\n");
    printf("Number of elevators: %d\n", num_elevators);
    printf("Total passengers: %d\n", total_passengers);
    printf("Completed: %d\n", completed_passengers);
    printf("Give up: %d\n", giveup_passengers);
    if (total_passengers > 0)
        printf("Completion rate: %.1f%%\n", (float)completed_passengers / total_passengers * 100);
    printf("Simulation time: %.1f seconds\n", current_time / 10.0);

    return 0;
}