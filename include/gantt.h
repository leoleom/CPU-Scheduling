#ifndef GANTT_H
#define GANTT_H


void gantt_init(int size);
void gantt_add(int time, char pid);
void gantt_print(int n);
void gantt_free();
#endif

