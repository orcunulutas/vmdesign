
// 1-21-40
// 1-27-35
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tim.h"

Inst program[] = {
    DEF_INST_PUSH(15),
    DEF_INST_PUSH(3),
    DEF_INST_MOD(),
    DEF_INST_PRINT(),
};

#define PROGRAM_SIZE (sizeof(program)/sizeof(program[0]))


int stack_size;

void push(Machine *machine, int value) {
    if (machine->stack_size >= MAX_STACK_SIZE) {
        fprintf(stderr,"ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size] =value;
    machine->stack_size++;
}


int pop(Machine *machine) {
    if (machine->stack_size <=0) {
        fprintf(stderr,"ERROR: Stack Underflow\n");
        exit(1);
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
}

void index_swap(Machine *machine, int i) {
    if (i >= machine->stack_size || i <=0) {
	fprintf(stderr,"ERROR: Stack Underflow on swap\n");
	exit(1);
    }
    int temp = machine->stack[i];
    machine->stack[i] = pop(machine);
    push(machine, temp);
}

void index_dup(Machine *machine, int i) {
    if (i >= machine->stack_size || i <=0) {
	fprintf(stderr,"ERROR: Stack Underflow on swap\n");
	exit(1);
    }
    push(machine, machine->stack[i]);
}

void print_stack(Machine *machine) {
    printf("----- STACK:\n");
    for (int i=machine->stack_size-1;i>=0;i--) {
        printf("%d\n",machine->stack[i]);
    }
    printf("----- STACK END\n");
}

void write_program_to_file (Machine *machine,char *file_path) {
    FILE *file = fopen(file_path,"wb");

    if (file==NULL) {
        fprintf(stderr,"ERROR: Could not write to file %s\n",file_path);
        exit(1);
    }
    

    fwrite(machine->instructions,
                    sizeof(machine->instructions[0]), 
                    PROGRAM_SIZE,
                    file);

    fclose(file);
}


Machine *read_program_from_file (char *file_path) {
    FILE *file =fopen(file_path,"rb");
    if (file==NULL) {
        fprintf(stderr,"ERROR: Could not read from file %s\n",file_path);
    }
    Inst *instructions = malloc(sizeof(Inst)*MAX_STACK_SIZE);
    Machine *machine = malloc (sizeof(Machine));

    fseek(file,0,SEEK_END);
    int length = ftell(file);
    fseek(file,0,SEEK_SET);
    fread(instructions, sizeof(instructions[0]),length/8,file);

    machine->program_size = length/8;
    machine->instructions = instructions;

    fclose(file);

    return machine;
} 

int main() {
    int a,b;
    Machine *loaded_machine = malloc(sizeof(Machine)*MAX_STACK_SIZE);
    loaded_machine->instructions = program;
    write_program_to_file(loaded_machine,"test.tim");
    loaded_machine = read_program_from_file("test.tim");
    for (size_t ip=0;ip<loaded_machine->program_size;ip++) {
        print_stack(loaded_machine);
	switch (loaded_machine->instructions[ip].type) {
	    case INST_NOP:
		    continue;
		break;
	    case INST_PUSH:
                push(loaded_machine,loaded_machine->instructions[ip].value);
                break;
            case INST_POP:
                pop(loaded_machine);
                break;
            case INST_ADD:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push(loaded_machine,a+b);
                break;
            case INST_SUB:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push(loaded_machine,b-a);
                break;
            case INST_DUP:
                a=pop(loaded_machine);
                push(loaded_machine,a);
                push(loaded_machine,a);
                break;
            case INST_INDUP:
                index_dup(loaded_machine,loaded_machine->instructions[ip].value);
		break;
            case INST_SWAP:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push(loaded_machine,a);
                push(loaded_machine,b);
                break;
            case INST_INSWAP:
                index_swap(loaded_machine,loaded_machine->instructions[ip].value);
		break;
            case INST_MUL:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push(loaded_machine,a*b);
                break;
	    case INST_CMPE:
		a=pop(loaded_machine);
		b=pop(loaded_machine);
		if (a==b) {
			push(loaded_machine,1);
		} else {
			push(loaded_machine,0);
		}
		break;
		
	    case INST_CMPNE:
		a=pop(loaded_machine);
		b=pop(loaded_machine);
		if (a!=b) {
			push(loaded_machine,1);
		} else {
			push(loaded_machine,0);
		}
		break;
	    case INST_CMPL:
		a=pop(loaded_machine);
		b=pop(loaded_machine);
		if (a<b) {
			push(loaded_machine,1);
		} else {
			push(loaded_machine,0);
		}
		break;
		
	    case INST_CMPG:
		a=pop(loaded_machine);
		b=pop(loaded_machine);
		if (a>b) {
			push(loaded_machine,1);
		} else {
			push(loaded_machine,0);
		}
		break;
	    case INST_CJMP:
		a=pop(loaded_machine);
		if (a==1) {
			ip=loaded_machine->instructions[ip].value-1;
			if (ip >= loaded_machine->program_size+1) {
				fprintf(stderr,"ERROR: Jumping out of Stack");
				exit(1);
			}
		}
		break;
	    case INST_JMP:
		ip = loaded_machine->instructions[ip].value-1;
		if (ip>loaded_machine->program_size+1) {
			ip=loaded_machine->instructions[ip].value-1;
		} else {
			continue;
		}
		if (ip >= loaded_machine->program_size) {
			fprintf(stderr,"ERROR: Jumping out of Stack");
			exit(1);
		}
		break;
	    case INST_ZJMP:
		a=pop(loaded_machine);
		if (a==0) {
			ip=loaded_machine->instructions[ip].value-1;
			if (ip >= loaded_machine->program_size+1) {
				fprintf(stderr,"ERROR: Jumping out of Stack");
				exit(1);
			}
		}
		break;
	    case INST_NZJMP:
		a=pop(loaded_machine);
		if (a!=0) {
			ip=loaded_machine->instructions[ip].value-1;
			if (ip >= loaded_machine->program_size+1) {
				fprintf(stderr,"ERROR: Jumping out of Stack");
				exit(1);
			}
		}
		break;
            case INST_DIV:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push (loaded_machine,a/b);
                break;
            case INST_MOD:
                a=pop(loaded_machine);
                b=pop(loaded_machine);
                push (loaded_machine,a%b);
                break;
            case INST_PRINT:
                printf("%d\n",pop(loaded_machine));
                break;
	    case INST_HALT:
		ip = loaded_machine->program_size;
		break;
        }
    }
    print_stack(loaded_machine);
    return 0;
}
