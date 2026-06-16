#include "elevator_sim.h"

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

    printf("[%6d] Passenger %d arrives at floor %d -> floor %d, tolerance = %d\n",
        current_time, p->id, p->in_floor, p->out_floor,
        p->giveup_time - current_time);

    WaitQueue* wq = &wait_queues[p->in_floor];
    wq->queue[wq->rear++] = p;
    if (wq->rear >= 100) wq->rear = 0;

    if (p->out_floor > p->in_floor)
        elevator.call_up[p->in_floor] = 1;
    else
        elevator.call_down[p->in_floor] = 1;

    schedule_event(p->giveup_time - current_time, EV_GIVEUP, p);

    if (elevator.state == IDLE && !elevator.moving && elevator.door_state == DOOR_CLOSED)
        controller();

    int next_delay = generate_inter_time();
    schedule_event(next_delay, EV_PASSENGER_ARRIVE, NULL);
}

/* ---------- 放弃处理 ---------- */
void passenger_giveup(Passenger* p) {
    if (p->entered) return;

    WaitQueue* wq = &wait_queues[p->in_floor];
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
    printf("[%6d] Passenger %d gives up waiting (timeout)\n", current_time, p->id);
    free(p);
}

/* ---------- 事件分发 ---------- */
void process_event(Event* e) {
    switch (e->type) {
    case EV_PASSENGER_ARRIVE:
        generate_passenger();
        break;
    case EV_ELEV_ARRIVE:
        elevator_arrive(e->data.elev_arrive.floor);
        break;
    case EV_DOOR_OPEN:
        open_door();
        break;
    case EV_PERSON_EXIT:
        person_exit();
        break;
    case EV_PERSON_ENTER:
        person_enter();
        break;
    case EV_DOOR_CLOSE:
        close_door();
        break;
    case EV_GIVEUP:
        passenger_giveup(e->data.giveup.p);
        break;
    case EV_IDLE_BACK:
        idle_timeout();
        break;
    }
}

/* ---------- 主函数 ---------- */
int main() {
    init_system();

    int first_delay = random_range(MIN_INTER_TIME, MAX_INTER_TIME);
    schedule_event(first_delay, EV_PASSENGER_ARRIVE, NULL);

    while (!heap_empty() && current_time < SIM_TIME) {
        Event e = heap_pop();
        current_time = e.time;
        process_event(&e);
    }

    printf("\n========== Simulation finished ==========\n");
    printf("Total passengers: %d\n", total_passengers);
    printf("Completed: %d\n", completed_passengers);
    printf("Give up: %d\n", giveup_passengers);
    if (total_passengers > 0)
        printf("Completion rate: %.1f%%\n", (float)completed_passengers / total_passengers * 100);
    printf("Simulation time: %.1f seconds\n", current_time / 10.0);

    return 0;
}