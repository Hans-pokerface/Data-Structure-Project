#include "elevator_multi.h"
DispatchWeights W = {
    10.0,
    -30.0,
    -100.0,
    200.0,
    5.0
};
double loss;
PassengerData p_data[DATASET_NUM][MAX_DATA_PASSENGERS];
int p_count[DATASET_NUM];
int set;
void gen_p(void)
{
    for (int k = 0; k < DATASET_NUM; k++) {
        srand(1112345 + k);
        int t = 0;
        int cnt = 0;
        while (t < SIM_TIME &&
               cnt < MAX_DATA_PASSENGERS) {
            t += generate_inter_time();
            if (t >= SIM_TIME)
                break;
            PassengerData *pd =
                &p_data[k][cnt];
            pd->appear_time = t;
            double r =
                (double)rand() / RAND_MAX;
            if (r < PROB_FLOOR1 / 2) {
                pd->in_floor = 1;
                do {
                    pd->out_floor =
                        rand() % FLOORS;
                } while (pd->out_floor == 1);
            }
            else if (r < PROB_FLOOR1) {
                pd->out_floor = 1;
                do {
                    pd->in_floor =
                        rand() % FLOORS;
                } while (pd->in_floor == 1);
            }
            else {
                do {
                    pd->in_floor =
                        rand() % FLOORS;
                    pd->out_floor =
                        rand() % FLOORS;
                } while (
                    pd->in_floor ==
                    pd->out_floor);
            }
            pd->tolerance = generate_giveup_time(pd->in_floor, pd->out_floor);
            cnt++;
        }
        p_count[k] = cnt;
    }
}
/* ---------- 工具函数 ---------- */
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
int next_id = 0;
Passenger* create_passenger(void) {
    Passenger* p = (Passenger*)malloc(sizeof(Passenger));
    p->id = next_id + 1;
    p->entered = 0;
    p->assigned_elev = -1;
    p->in_floor = p_data[set][next_id].in_floor;
    p->out_floor = p_data[set][next_id].out_floor;
    p->arrive_time = p_data[set][next_id].appear_time;
    p->giveup_time = p->arrive_time + p_data[set][next_id].tolerance;
    next_id++;
    /*double r = (double)rand() / RAND_MAX;
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
    p->giveup_time = current_time + generate_giveup_time(p->in_floor, p->out_floor);*/
    return p;
}

/* ---------- 乘客到达处理 ---------- */
void generate_passenger(void) {
    Passenger* p = create_passenger();
    total_passengers++;

    int direction = (p->out_floor > p->in_floor) ? DIR_UP : DIR_DOWN;
    int elev_id = dispatch_elevator(p->in_floor, direction, &W);
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

    //printf("[%6d] Passenger %d arrives at floor %d -> floor %d, tolerance = %d, assigned to elevator %d\n",
    //    current_time, p->id, p->in_floor, p->out_floor,
    //    p->giveup_time - current_time, elev_id);

    schedule_event(p->giveup_time - current_time, EV_GIVEUP, -1, p);

    if (e->state == IDLE && !e->moving && e->door_state == DOOR_CLOSED)
        controller(elev_id);

    //int next_delay = generate_inter_time();
    if (next_id < p_count[set]) {
    int delay = p_data[set][next_id].appear_time - current_time;
    if (delay < 0) delay = 0; 
    schedule_event(delay, EV_PASSENGER_ARRIVE, -1, NULL);
    }
}

/* ---------- 乘客放弃等待处理 ---------- */
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
    loss += (current_time - p->arrive_time + 500) * (current_time - p->arrive_time + 500) / 100.0;
    //printf("[%6d] Passenger %d gives up waiting (timeout) for elevator %d\n",
    //    current_time, p->id, elev_id);
    free(p);
}

/* ---------- 事件处理 ---------- */
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
DispatchWeights *generate_w(DispatchWeights *pre_w, double stride)
{
    DispatchWeights *new_w = (DispatchWeights *) malloc(sizeof(DispatchWeights));
    new_w->w_dist = pre_w->w_dist + ((double) rand() / RAND_MAX * 2.0 - 1.0) * stride;
    new_w->w_door_open = pre_w->w_door_open + ((double) rand() / RAND_MAX * 2.0 - 1.0) * stride;
    new_w->w_idle = pre_w->w_idle + ((double) rand() / RAND_MAX * 2.0 - 1.0) * stride;
    new_w->w_load = pre_w->w_load + ((double) rand() / RAND_MAX * 2.0 - 1.0) * stride;
    new_w->w_same_dir = pre_w->w_same_dir + ((double) rand() / RAND_MAX * 2.0 - 1.0) * stride;
    return new_w;
}
int ele = 0, total = 0, com = 0, giveup = 0;
void evaluate_weights(int elev_count)
{
    loss = 0;
    for (int j = 0; j < 10; j++) {
        total_passengers = completed_passengers = giveup_passengers = 0;
        current_time = 0;
                init_system(elev_count);
                set = j;
                next_id = 0;
                //int first_delay = random_range(MIN_INTER_TIME, MAX_INTER_TIME);
                schedule_event(p_data[set][0].appear_time, EV_PASSENGER_ARRIVE, -1, NULL);

                while (!heap_empty() && current_time < SIM_TIME) {
                    Event e = heap_pop();
                    current_time = e.time;
                    process_event(&e);
            }
            total += total_passengers;
            com += completed_passengers;
            giveup += giveup_passengers;
        }
}
/* ---------- 主函数 ---------- */
int main(int argc, char* argv++) {
    int elev_count = 3;
    if (argc > 1) elev_count = atoi(argv[1]);
    if (elev_count < 1) elev_count = 1;
    if (elev_count > MAX_ELEVATORS) elev_count = MAX_ELEVATORS;
    double best_loss = 9999999;
    double stride = 5.0;
    int times = 0;
    int best_total, best_com, best_giveup;
    DispatchWeights best_w = W;
    srand(54321);
    gen_p();
    evaluate_weights(elev_count);
    printf("\n========== First Simulation (10 samples) ==========\n");
    printf("Number of elevators: %d\n", num_elevators);
    printf("Total passengers: %d\n", total);
    printf("Completed: %d\n", com);
    printf("Give up: %d\n", giveup);
    if (total_passengers > 0)
        printf("Completion rate: %.1f%%\n", (float)com / total * 100);

    for (int i = 0; i < 2000; i++) {
        DispatchWeights *new_w = generate_w(&best_w, stride);
        W = *new_w;
        total = 0;
        com = 0;
        giveup = 0;
        evaluate_weights(elev_count);
        if (loss < best_loss) {
            times = i;
            best_loss = loss;
            best_w = *new_w;
            best_total = total;
            best_com = com;
            best_giveup = giveup;
        }
        if (i == times + 500) {
            printf("Training times: %d\n", times);
            break;
        }
        if (i % 100 == 0 && i > 0) {
            stride *= 0.8; 
        }
        if (i % 50 == 0) {
            printf("current weights: %.6f %.6f %.6f %.6f %.6f\n", best_w.w_dist, best_w.w_same_dir, best_w.w_idle, best_w.w_door_open, best_w.w_load);
            printf("current loss: %.6f\n", best_loss);
        }
    }
    printf("Final loss:%.6f\n", best_loss);
    printf("Final weights: %.6f %.6f %.6f %.6f %.6f\n", best_w.w_dist, best_w.w_same_dir, best_w.w_idle, best_w.w_door_open, best_w.w_load);
    loss = 0;
    W = best_w;
    total = com = giveup = 0;
    //evaluate_weights(elev_count);
    printf("\n========== Best Simulation (10 samples) ==========\n");
    printf("Number of elevators: %d\n", num_elevators);
    printf("Total passengers: %d\n", best_total);
    printf("Completed: %d\n", best_com);
    printf("Give up: %d\n", best_giveup);
    if (total_passengers > 0)
        printf("Completion rate: %.1f%%\n", (float)best_com / best_total * 100);
    return 0;
}