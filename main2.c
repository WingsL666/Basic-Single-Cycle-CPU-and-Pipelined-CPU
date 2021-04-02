#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//Global for Fetch()
int pc = 0;
int next_pc = 0;
char** ins_memory;

//Global for Decode()
char **char_registers = (char *[]) {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3","t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};


char **fetch(int pc) {
    
    //FILE *fsize = fopen("sample_binary.txt","r");
    char ch;
    int count = 100;
    int local_pc = pc;
    int index = 0;

    char** ins_memory= (char**)malloc(count*sizeof(char*)); // allocating size for ins_memory
	
    FILE* f;
	
    while ((f = fopen("sample_binary.txt", "r")) != NULL) {
        index = local_pc/4; //index of instruction in the file
        
        fseek(f,index*34,SEEK_CUR); // take instruction at current pointer
        char *ins = (char*)malloc(34*sizeof(char));
        if(fgets(ins,34,f)!=NULL){
            ins_memory[index] = (char*)malloc(34);
            strcpy(ins_memory[index],ins); //store new ins into array
			//printf("Current inst: %s\n",ins_memory[index]);
        }
        // incrementing pc
        local_pc += 4;
    }
	if(f != NULL){
		fclose(f); // close file
	}
    
    return ins_memory;
}

void decode(char* ins){
	int opcode = 0;
	printf("Machine code ins: %s\n", ins);
	for(int i = 0;i <= 5; i++){ //opcode 0-5 has 6 bits
		opcode = opcode + (*(ins+i)*twoToPower(5-i));//element 0-5 correspond to 2^5...2^0
	}
	
	//printf("Opcode: %d \n",opcode);
	
	//0->R format
	//2 or 3 ->J format
	//Otherwise, 1,4-62->I format
	
	if(opcode == 0){
		call_R_format(ins, char_registers);
	}
	else if(opcode == 2 || opcode == 3){
		call_J_format(ins, opcode);
	}
	else{
		call_I_format(ins, char_registers, opcode);
	}
}


int twoToPower(int power){//implement a for loop for powering
	int result = 1;
	for(int i = 1;i <= power; i++){
		result *= 2;
	}
	return result;
}

void call_R_format(int* code, char** reg_arr){
	printf("Instruction Type: R \n");
	
	int rs = 0,rt = 0,rd = 0, shamt = 0, function = 0;
	int counter_power = 0;
	char *r_operation;
	
	//move func in front to update to correct operation and print func code at the last  
	for(int i = 26;i <= 31; i++){ //func 26-31 has 5 bits
        function = function + (*(code+i)*twoToPower(5-counter_power));//TwoToPower(5-counter_power) gets 2^5...2^0
        counter_power++;
    }
	
    switch(function)
    {
        case 0:  // Sll
            r_operation = "sll";
            break;

        case 2:    // Srl
            r_operation = "srl";
            break;

        case 8:   // Jr
            r_operation = "jr";
            break;
        case 32:   // Add
            r_operation = "add";
            break;

        case 33:   // Addu
            r_operation = "addu";
            break;
        
        case 34:  // Sub
            r_operation = "sub";
            break;
       
        case 35:  // subu
            r_operation = "Subu";
            break;

        case 36:  // and
            r_operation = "and";
            break;
            
        case 37: // or
            r_operation = "or";
            break;
        
        case 39: // nor
            r_operation = "nor";
            break;
           
        case 42:  // slt
            r_operation = "slt";
            break;
        
        case 43:  // sltu
            r_operation = "sltu";
            break;
	}
	
	printf("Operation: %s \n",r_operation);
	
	counter_power = 0;
	for(int i = 6;i <= 10; i++){ //rs 6-10 has 5 bits
		rs = rs + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
    //printf("Rs: %s",char_registers[rs]);
	
	printf("Rs: %s", reg_arr[rs]);
	
	printf("(R%d",rs);
    printf(")\n");
	
	counter_power = 0;
	for(int i = 11;i <= 15; i++){ //rt 11-15 has 5 bits
		rt = rt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
	
    printf("Rt: %s",reg_arr[rt]);
    printf("(R%d",rt);
    printf(")\n");
    
	counter_power = 0;
	for(int i = 16;i <= 20; i++){ //rd 16-20 has 5 bits
		rd = rd + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
    printf("Rd: %s",reg_arr[rd]);
    printf("(R%d",rd);
    printf(")\n");
	
	
	counter_power = 0;
	for(int i = 21;i <= 25; i++){ //shmat 21-25 has 5 bits
		shamt = shamt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
	printf("Shamt: %d \n",shamt);
	
	printf("Funct: %d \n",function);
    
}



void call_J_format(int* code, int opcode){
	printf("Instruction Type: J \n");
	
	int target_address = 0;
	int counter_power = 0;
	char* j_operation;
	
	switch(opcode)
	{
       case 2:
           j_operation = "j";
           break;
           
       case 3:
           j_operation = "jal";
           break;
	}
	
    printf("Operation: %s \n",j_operation);
	
	
	for(int i = 6;i <= 31; i++){ //target 6-31 has 26 bits
		target_address = target_address + (*(code+i)*twoToPower(25-counter_power));//TwoToPower(25-counter_power) gets 2^25...2^0
		counter_power++;
	}
	printf("Jump address: %d \n",target_address);
	
}


void call_I_format(int* code, char** reg_arr, int opcode){
	printf("Instruction Type: I \n");
	
	int rs = 0, rt = 0, immediate = 0;
	int counter_power = 0;
	char* i_operation;
	
	switch(opcode)
	{

     case 4:   // beq
         i_operation = "beq";
         break;
         
         case 5:   // bne
         i_operation = "bne";
         break;
        
        case 8:   // addi
         i_operation = "addi";
         break;

         case 9:          // addiu
         i_operation = "addiu";
         break;
 
        case 10: // slti
         i_operation = "slti";
         break;
 
        case 11: //sltiu
         i_operation = "sltiu";
         break;
 
         
         case 12:   // andi
         i_operation = "andi";
         break;
 

         case 13: // ori
         i_operation = "ori";
                     break;
 
  
         case 15:  // lui
         i_operation = "lui";
         break;
 
         case 35:  // lw
         i_operation = "lw";
         break;
 

         case 36:   // Lbu
         i_operation = "lbu";
         break;
 
        case 37: // lhu
         i_operation = "lhu";
         break;
 
        case 40:  //sb
         i_operation = "sb";
         break;
 
        case 41: //sh
         i_operation = "sh";
         break;
 
        case 43: //sw
         i_operation = "sw";
         break;
 
        case 48: //ll
         i_operation = "ll";
         break;
 
        case 56:
         i_operation = "sc";
         break;
 
       }
	   
	printf("Operation: %s \n",i_operation);
	
	
	for(int i = 6;i <= 10; i++){ //rs 6-10 has 5 bits
		rs = rs + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
	
	printf("Rs: %s", reg_arr[rs]);
	printf("(R%d)\n",rs);
	
	
	counter_power = 0;
	for(int i = 11;i <= 15; i++){ //rt 11-15 has 5 bits
		rt = rt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
		counter_power++;
	}
	
	printf("Rt: %s", reg_arr[rt]);
	printf("(R%d)\n",rs);
	
	counter_power = 0;
	for(int i = 16;i <= 31; i++){ //imm 16-31 has 16 bits
		immediate = immediate + (*(code+i)*twoToPower(15-counter_power));//TwoToPower(15-counter_power) gets 2^15...2^0
		counter_power++;
	}
	
	printf("Immediate: %d\n",immediate);
    
   }


int main(){
    ins_memory = fetch(pc);
	
	for(int i = 0; i < 8; i++){
		decode(*(ins_memory + i));
	}
	
    return 0;
}
