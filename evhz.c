/* Copyright (C) 2016 Ian Kelling */

/* Licensed under the Apache License, Version 2.0 (the "License"); */
/* you may not use this file except in compliance with the License. */
/* You may obtain a copy of the License at */

/*     http://www.apache.org/licenses/LICENSE-2.0 */

/* Unless required by applicable law or agreed to in writing, software */
/* distributed under the License is distributed on an "AS IS" BASIS, */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/* See the License for the specific language governing permissions and */
/* limitations under the License. */
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>

#define EVENTS 200
#define HZ_LIST 64

typedef struct event_s {
    int fd;
    int hz[HZ_LIST];
    int count;
    int avghz;
    unsigned long long prev_time;
    char name[128];
} event_t;

int quit = 0;

void sigint() {
    quit = 1;
}

int main(int argc, char *argv[]) {
    int optch;
    int i;
    event_t events[EVENTS];
    int verbose = 1;
    int max_event = 0;

    while((optch = getopt(argc, argv, "hn")) != -1) {
        switch(optch) {
        case('h'):
            printf("Usage: %s [-n|-h]\n", argv[0]);
            printf("-n     nonverbose\n");
            printf("-h     help\n");
            return 0;
            break;
        case('n'):
            verbose = 0;
            break;
        }
    }

    if(geteuid() != 0) {
        printf("%s must be used as superuser\n", argv[0]);
        return 1;
    }

    signal(SIGINT, sigint);

    printf("Press CTRL-C to exit.\n\n");

    memset(events, 0, sizeof(events));

    // List input devices
    for(i = 0; i < EVENTS; i++) {
        // 20 is needed for 3 digits of event devs under the expected format, but
        // just give it some extra in case.
        char device[30];

        sprintf(device, "/dev/input/event%i", i);
        events[i].fd = open(device, O_RDONLY);
		
        if(events[i].fd != -1) {
            max_event = i;
            ioctl(events[i].fd, EVIOCGNAME(sizeof(events[i].name)), events[i].name);
            if(verbose) printf("event%i: %s\n", i, events[i].name);
        }
    }

    while(!quit) {
        fd_set set;

        FD_ZERO(&set);

        for(i = 0; i <= max_event; i++) {
            if(events[i].fd != -1) {
                FD_SET(events[i].fd, &set);
            }
        }

        if(select(FD_SETSIZE, &set, NULL, NULL, NULL) <= 0) {
            continue;
        }

        int bytes;
        struct input_event event;

        for(i = 0; i <= max_event; i++) {
            if(events[i].fd == -1 || !FD_ISSET(events[i].fd, &set)) {
                continue;
            }

            bytes = read(events[i].fd, &event, sizeof(event));

            if(bytes != sizeof(event)) {
                continue;
            }

            if(event.type == EV_REL || event.type == EV_ABS) {
                unsigned long long time, timediff;
                unsigned hz = 0;

                time = (unsigned long long)event.time.tv_sec * 1000ULL;
                time += (unsigned long long)event.time.tv_usec / 1000ULL;

                timediff = time - events[i].prev_time;

                if(timediff != 0)
                    hz = 1000ULL / timediff;

                if(hz > 0) {
                    unsigned j, maxavg;

                    events[i].count++;
                    events[i].hz[events[i].count & (HZ_LIST - 1)] = hz;

                    events[i].avghz = 0;

                    maxavg = (events[i].count > HZ_LIST) ? HZ_LIST : events[i].count;

                    for(j = 0; j < maxavg; j++) {
                        events[i].avghz += events[i].hz[j];
                    }

                    events[i].avghz /= maxavg;

                    if(verbose) printf("%s: Latest % 5iHz, Average % 5iHz\n", events[i].name, hz, events[i].avghz);
                }

                events[i].prev_time = time;
            }
        }
    }

    for(i = 0; i < max_event; i++) {
        if(events[i].fd != -1) {
            if (events[i].avghz != 0) {
                printf("\nAverage for %s: % 5iHz\n", events[i].name, events[i].avghz);
            }
            close(events[i].fd);
        }
    }

    return 0;
}
