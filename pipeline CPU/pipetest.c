#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// #define FILENAME "sample_binary.txt"

//define boolean type
typedef enum {
    false, true
}
bool;

//Global for Fetch()
int pc = 0;
int next_pc = 0;

//Global for Decode()
int jump_target = 0;
int registerfile_rs_index, registerfile_rt_index, registerfile_rd_index;
int global_immediate;
int* sign_extended;

//9 control signals
int jump = 0;
int RegDst;
int ALUSrc;
int MemtoReg;
int RegWrite;
int MemRead;
int MemWrite;
int branch = 0;
int InstType;
int alu_op;

//Global for Execute()
int alu_zero = 0;
int branch_target;
int global_rd_value;
int d_mem_entry_address = 0;

//Global for mem()
int d_mem_entry_value; //store the data memory value get from lw

//Register name
char **char_registers = (char *[]) {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3","t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

//Clock cycle in WB()
int total_clock_cycles = 1;


//Pipeline stage registers: work as buffers to store all variables
int if_stage; //indicate if is in Fetch() stage or not with 1 as true, 0 as false
char* if_id_ins;//store machine code in if_id buffer
int* if_id;
int* id_ex;
int* ex_mem;
int* mem_wb;


//Set flush to true in main()->while(free) pipe-> wb() stage if branch is taken
bool flush = false;


char* fetch(char** ins_memory) {
    
    if(ex_mem[12] == 1 && ex_mem[15] == 1 && flush == false){//if branch = 1 and alu_zero = 1, then branch is taken
        //printf("Assume branch is not Taken, keep fetching next_pc\n");
        //printf("fetch(): pc is updated to %d to next_pc get from if_id\n", pc);
        pc = if_id[3];
        
        printf("pc is updated to 0x%x \n", pc);
    }
    else if(mem_wb[12] == 1 && mem_wb[15] == 1 && flush == true){//if branch = 1 and alu_zero = 1, then branch is taken
        //flush is done in wb(), now just fetch the new Taken branch
        //printf("Branch is Taken\n");
        //printf("fetch(): pc is updated to %d to branch_target\n", pc);
        pc = mem_wb[16];
        
        printf("pc is updated to 0x%x to branch_target\n", pc);
        
        //clean up the mem_wb buffer
        for(int i = 0; i < 25; i++){
            mem_wb[i] = 0;
        }
        
        //reset flush to false after fetching the branch tar ins
        flush = false;
    }
    else if(id_ex[9] == 1){ //jump signal is 1
        pc = id_ex[4]; // update pc to jump_target
        printf("fetch(): pc is updated to 0x%x to jump_target\n", pc);
    }
    else if(total_clock_cycles == 1){
        //printf("fetch(): pc is updated to 0\n");
        pc = 0;
        printf("pc is updated to 0x%x \n", pc+4);
    }
    else if(total_clock_cycles <= 3){
        pc = if_id[3]; //update pc to next_pc
        printf("pc is updated to 0x%x \n", pc+4);
    }
    else{
        //printf("fetch(): pc is updated to %d to next_pc get from if_id\n", pc);
        pc = if_id[3]; //update pc to next_pc
        printf("pc is updated to 0x%x\n", pc);
    }
    //no else since the first cycle always fetch #0 bc pc is 0 as defult
    
    char* instruction;
    
    int ins_index = pc / 4;
    //printf("pc ins index is %d \n", ins_index);
    
    instruction = ins_memory[ins_index];
    
    //pc = next_pc;
    //printf("pc is modified to next_pc: %d \n", pc);
    
    //store pc values
    if_id[2] = pc;
    if_id[3] = pc + 4;
    
    return instruction;
}

int twoToPower(int power){//implement a for loop for powering, Use in call_?_format()
    int result = 1;
    
    for(int i = 1;i <= power; i++){
        result *= 2;
    }
    
    return result;
}



void printArr(int *a, int size){ //print 2D array
    
    //printf("printArr: ");
    for(int i = 0 ; i < size;i++){
        printf("%d",*(a+i));
    }
    
    printf("\n");
}

void printArrWithSpace(int *a, int size){ //print 2D array with space between each element
    
    //printf("printArr: ");
    for(int i = 0 ; i < size;i++){
        printf("%d ",*(a+i));
    }
    
    printf("\n");
}



//Not zero extension, sign_extension extend sign base on left most bit here

void sign_extension(int sign_bit, int* code, int* sign_extended){ //Use in Decode()
    
    for(int i = 16;i <= 31; i++){ //imm 16-31 has 16 bits
        *(sign_extended+i) = *(code+i);
    }
    
    if(sign_bit == 1){
        
        for(int i = 0;i <= 15; i++){ //imm 16-31 has 16 bits
            *(sign_extended+i) = 1;
        }
        
        //printf("Sign extended code:");
        //printArr(sign_extended, 32);
    }
    
    else{
        
        for(int i = 0;i <= 15; i++){ //imm 16-31 has 16 bits
            *(sign_extended+i) = 0;
        }
        
        //printf("Sign extended code:");
        //printArr(sign_extended, 32);
    }
}

int convertNegBinaryToDecimal(int* code){ //Use in Decode()
    
    int immediate = 0, counter_power = 0;
    
    //use 2's complement to convert imm binary to negative decimal
    //Flip the 16 bits for imm and store it in flip_imm
    
    int* flip_imm = (int*)malloc(16*sizeof(int));
    
    for(int i = 16;i <= 31; i++){ //imm 16-31 has 16 bits
        
        if(*(code+i) == 1){
            
            //-16 to start at index 0 in flip_imm
            *(flip_imm + i - 16) = 0;
        }
        
        else{
            *(flip_imm + i - 16) = 1;
        }
    }
    
    printf("After Flip 16 bits:");
    printArr(flip_imm, 16);
    counter_power = 0;
    
    for(int i = 0;i <= 15; i++){ //imm 16-31 has 16 bits
        
        immediate = immediate + (*(flip_imm+i)*twoToPower(15-counter_power));//TwoToPower(15-counter_power) gets 2^15...2^0
        counter_power++;
    }
    
    //add 1 to the flip bits for 2's complement
    immediate += 1;
    
    //make the immediate negative
    immediate *= -1;
    
    return immediate;
}

//Use in Decode()->call_J_format for jump_target
//but also can be use in anywhere in program

int* convertDecimalto32bitBinary(int deciaml){
    
    int* arr = (int*) malloc(32*sizeof(int));
    int n = deciaml;
    
    for(int i = 0;n > 0;i++){
        
        arr[31-i] = n % 2;
        n = n/2;
    }
    
    printf("Convert the current pc = %d to binary:",pc);
    printArr(arr,32);
    
    return arr;
}

int getFirst4bitReturnAsDecimal(int* binary32){ //get the first 4 bits in pc and convert it to decimal
    
    int dec = 0, counter_power = 0;
    
    for(int i = 0; i < 4;i++){
        dec = dec + (*(binary32+i)*twoToPower(31-i));//TwoToPower(31-counter_power) gets 2^31...2^28
    }
    
    return dec;
}

void call_R_format(int* code, char** reg_arr){ //Use in Decode(),combine ControlUnit()
    
    //printf("Instruction Type: R \n");
    
    int rs = 0,rt = 0,rd = 0, shamt = 0, function = 0;
    int counter_power = 0;
    char *r_operation;
    
    //move func in front to update to correct operation and print func code at the last
    for(int i = 26;i <= 31; i++){ //func 26-31 has 6 bits
        
        function = function + (*(code+i)*twoToPower(5-counter_power));//TwoToPower(5-counter_power) gets 2^5...2^0
        counter_power++;
    }
    
    //set control unit signals
    jump = 0;
    RegDst = 1;
    ALUSrc = 0;
    MemtoReg = 0;
    RegWrite = 1;
    MemRead = 0;
    MemWrite = 0;
    branch = 0;
    InstType = 10;
    
    switch(function)
    {
        case 32:   // Add
            r_operation = "add";
            alu_op = 10;
            break;
            
        case 34:  // Sub
            r_operation = "sub";
            alu_op = 110;
            break;
            
        case 36:  // and
            r_operation = "and";
            alu_op = 0;
            break;
            
        case 37: // or
            r_operation = "or";
            alu_op = 1;
            break;
            
        case 39: // nor
            r_operation = "nor";
            alu_op = 1100;
            break;
            
        case 42:  // slt
            r_operation = "slt";
            alu_op = 111;
            break;
    }
    
    //printf("Operation: %s \n",r_operation);
    counter_power = 0;
    
    for(int i = 6;i <= 10; i++){ //rs 6-10 has 5 bits
        
        rs = rs + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Rs: %s",char_registers[rs]);
    //printf("Rs: %s", reg_arr[rs]);
    //printf("(R%d",rs);
    //printf(")\n");
    
    //record the rs index to global
    registerfile_rs_index = rs;
    counter_power = 0;
    
    for(int i = 11;i <= 15; i++){ //rt 11-15 has 5 bits
        
        rt = rt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Rt: %s",reg_arr[rt]);
    //printf("(R%d",rt);
    //printf(")\n");
    
    //record the rt index to global
    registerfile_rt_index = rt;
    counter_power = 0;
    
    for(int i = 16;i <= 20; i++){ //rd 16-20 has 5 bits
        
        rd = rd + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Rd: %s",reg_arr[rd]);
    //printf("(R%d",rd);
    //printf(")\n");
    
    //record the rd index to global
    registerfile_rd_index = rd;
    counter_power = 0;
    
    for(int i = 21;i <= 25; i++){ //shmat 21-25 has 5 bits
        
        shamt = shamt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Shamt: %d \n",shamt);
    //record the shmat to global
    //global_shamt = shamt;
    //printf("Funct: %d \n",function);
    
    
    
    //Store data to id_ex buffer to be use in stage 3(aka exe stage)
    id_ex[5] = registerfile_rs_index;
    id_ex[6] = registerfile_rt_index;
    id_ex[7] = registerfile_rd_index;
    
    id_ex[9] = jump;
    id_ex[10] = RegWrite;
    id_ex[11] = MemWrite;
    id_ex[12] = branch;
    id_ex[13] = InstType;
    id_ex[14] = alu_op;
    
    //printf("id_ex in call_R_format:");
    //printArrWithSpace(id_ex, 20);
}



void call_J_format(int* code, int opcode){ //Use in Decode(),combine ControlUnit()
    
    //printf("Instruction Type: J \n");

    int target_address = 0;
    
    int counter_power = 0;
    
    char* j_operation;

    switch(opcode)
        
    {
            
        case 2:
            
            j_operation = "j";
            
            jump = 1;
            
            RegWrite = 0;
            
            MemRead = 0;
            
            MemWrite = 0;
            
            branch = 0;
            
            break;
            
    }
    
    
    
    //printf("Operation: %s \n",j_operation);
    
    for(int i = 6;i <= 31; i++){ //target 6-31 has 26 bits
        
        target_address = target_address + (*(code+i)*twoToPower(25-counter_power));//TwoToPower(25-counter_power) gets 2^25...2^0
        
        counter_power++;
    }
    
    //printf("Jump address: %d \n",target_address);
    
    
    
    
    
    //First shift-left-2 and then merge first 4 bits from pc to get target_address
    
    jump_target = target_address*4;
    
    
    
    int* pc_binary = (int*) malloc(32*sizeof(int));
    
    pc_binary = convertDecimalto32bitBinary(pc);
    
    
    
    int merge4 = getFirst4bitReturnAsDecimal(pc_binary);
    
    //printf("merge4:%d\n",merge4);
    
    jump_target += merge4;
        
    
    //Global for Decode() variables dictionary that will be store in buffer
    id_ex[4] = jump_target;
    
    id_ex[9] = jump;
    id_ex[10] = RegWrite;
    id_ex[11] = MemWrite;
    id_ex[12] = branch;
    
    //printf("id_ex in call_J_format:");
    //printArrWithSpace(id_ex, 20);
}





void call_I_format(int* code, char** reg_arr, int opcode, int* sign_extended){ //Use in Decode(), combine ControlUnit()
    
    //printf("Instruction Type: I \n");
    int rs = 0, rt = 0, immediate = 0;
    int counter_power = 0;
    char* i_operation;
    
    switch(opcode)
    {
        case 4:   // beq
            i_operation = "beq";
            jump = 0;
            ALUSrc = 0;
            RegWrite = 0;
            MemRead = 0;
            MemWrite = 0;
            branch = 1;
            InstType = 01;
            
            alu_op = 110;
            break;
            
        case 35:  // lw
            i_operation = "lw";
            jump = 0;
            RegDst = 0;
            ALUSrc = 1;
            MemtoReg = 1;
            RegWrite = 1;
            MemRead = 1;
            MemWrite = 0;
            branch = 0;
            InstType = 0;
            
            alu_op = 10;
            break;
            
        case 43: //sw
            i_operation = "sw";
            jump = 0;
            ALUSrc = 1;
            RegWrite = 0;
            MemRead = 0;
            MemWrite = 1;
            branch = 0;
            InstType = 0;
            
            alu_op = 10;
            break;
    }
    //printf("Operation: %s \n",i_operation);
    for(int i = 6;i <= 10; i++){ //rs 6-10 has 5 bits
        
        rs = rs + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Rs: %s", reg_arr[rs]);
    //printf("(R%d)\n",rs);
    
    //record the rs index to global
    registerfile_rs_index = rs;
    
    counter_power = 0;
    
    for(int i = 11;i <= 15; i++){ //rt 11-15 has 5 bits
        
        rt = rt + (*(code+i)*twoToPower(4-counter_power));//TwoToPower(4-counter_power) gets 2^4...2^0
        counter_power++;
    }
    
    //printf("Rt: %s", reg_arr[rt]);
    //printf("(R%d)\n",rt);
    
    //record the rt index to global
    registerfile_rt_index = rt;
    
    //assume if immediate offet is positive
    counter_power = 0;
    
    for(int i = 16;i <= 31; i++){ //imm 16-31 has 16 bits
        
        immediate = immediate + (*(code+i)*twoToPower(15-counter_power));//TwoToPower(15-counter_power) gets 2^15...2^0
        counter_power++;
    }
    
    //strcmp(...,...) == 0 means the two string are the same
    if(strcmp(i_operation,"lw") == 0 || strcmp(i_operation,"sw") == 0){
        
        //printf("leftmost bit: %d\n", *(code+16));
        
        //16 is the leftmost bit of the immediate
        
        if(*(code+16) == 1){
            //printf("Negative offset\n");
            sign_extension(*(code+16),code, sign_extended);//only lw and sw need sign-extension
            //convertNegBinaryToDecimal using 2's complement
            immediate = convertNegBinaryToDecimal(code);
            //printf("Immediate: %d\n",immediate);
        }
        
        else{
            //printf("Positive offset\n");
            sign_extension(*(code+16),code, sign_extended);
            //printf("Immediate: %d\n",immediate);
        }
    }
    
    else{ // i_operation is beq
        
        if(*(code+16) == 1){
            //printf("Negative offset\n");
            //convertNegBinaryToDecimal using 2's complement
            immediate = convertNegBinaryToDecimal(code);
            //printf("Immediate: %d\n",immediate);
        }
        
        else{
            //printf("Positive offset\n");
            //printf("Immediate: %d\n",immediate);
        }
    }
    
    //record immediate to global
    global_immediate = immediate;
    
    //printf("call_I_format(): global_immediate : %d\n", global_immediate);
    
    
    
    //Global for Decode() variables dictionary that will be store in buffer
    /*
    4: int jump_target = 0;
    
    5: int registerfile_rs_index;
    6: int registerfile_rt_index;
    7: int registerfile_rd_index;
    
    8: int global_immediate;
    //control signals
    9: int jump = 0;
    10: int RegWrite;
    11: int MemWrite;
    12: int branch = 0;
    13: int InstType;
    14: int alu_op;
    */
    
    
    id_ex[5] = registerfile_rs_index;
    id_ex[6] = registerfile_rt_index;
    
    id_ex[8] = global_immediate;
    
    id_ex[9] = jump;
    id_ex[10] = RegWrite;
    id_ex[11] = MemWrite;
    id_ex[12] = branch;
    id_ex[13] = InstType;
    id_ex[14] = alu_op;
    
    //printf("id_ex in call_I_format:");
    //printArrWithSpace(id_ex, 20);
}








void decode(char* ins, int* sign_extended){ //ControlUnit() is integrediate in decode()
    
    //Get data from if_id buffer
    pc = if_id[2];
    next_pc = if_id[3];
    
    
    int opcode = 0;
    int* machineCode = (int*) malloc(32*sizeof(int));
    //printf("Machine code ins: %s\n", ins);
    
    //Transfer User input char string into array of integers machineCode
    for(int i = 0;i < 32; i++){
        //since ASCII code for '0' is 48 and '1' is 49, use -48 to get the correct integer
        *(machineCode+i) = *(ins+i) - 48;
    }
    
    for(int i = 0;i <= 5; i++){ //opcode 0-5 has 6 bits
        opcode = opcode + (*(machineCode+i)*twoToPower(5-i));//element 0-5 correspond to 2^5...2^0
    }
    
    //printf("Opcode: %d \n",opcode);
    //0->R format
    //2 or 3 ->J format
    //Otherwise, 1,4-62->I format
    
    if(opcode == 0){
        call_R_format(machineCode, char_registers);//also store data to id_ex
    }
    
    else if(opcode == 2 || opcode == 3){
        call_J_format(machineCode, opcode);//also store data to id_ex
    }
    
    else{
        call_I_format(machineCode, char_registers, opcode, sign_extended);//also store data to id_ex
    }
    
    
    //store data to id_ex
    id_ex[2] = pc;
    if_id[3] = pc + 4;
    
    
}







void execute(int* registerfile){
    
    //Get data from id_ex buffer
    pc = id_ex[2];
    next_pc = id_ex[3];
    jump_target = id_ex[4];
    
    registerfile_rs_index = id_ex[5];
    registerfile_rt_index = id_ex[6];
    registerfile_rd_index = id_ex[7];
    
    global_immediate = id_ex[8];
    
    jump = id_ex[9];
    RegWrite = id_ex[10];
    MemWrite = id_ex[11];
    branch = id_ex[12];
    InstType = id_ex[13];
    alu_op = id_ex[14];
    //
    
    
    int rs_value,rt_value,rd_value;
    
    //Run ALU Operation
    
    if(InstType == 10){ //R type
        
        rs_value = registerfile[registerfile_rs_index];
        
        rt_value = registerfile[registerfile_rt_index];

        switch(alu_op)
        {
            case 10:   // Add
                
                rd_value = rs_value + rt_value;
                
                break;

            case 110:  // Sub
                
                rd_value = rs_value - rt_value;
                
                break;

            case 0:  // and
                
                rd_value = rs_value & rt_value; //binary AND operator
                
                break;
 
            case 1: // or
                
                rd_value = rs_value | rt_value; //binary OR operator
                
                break;

            case 1100: // nor
                
                rd_value = ~(rs_value | rt_value);
                
                break;

            case 111:  // slt
                
                rd_value = rs_value - rt_value;
                
                if(rd_value < 0){
                    
                    rd_value = 1; //aka means slt is true
                    
                }
                
                else{
                    
                    rd_value = 0; //aka means slt is false
                    
                }
                
                break;
        }
        
        
        
        global_rd_value = rd_value; //store this to global for write back later
        
        //printf("exe():global_rd_value for R type: %d\n", global_rd_value);
        
    }
    
    else if(InstType == 0){ // lw or sw
        rs_value = registerfile[registerfile_rs_index];
        
        switch(alu_op)
        {
            case 10:   // Add
                d_mem_entry_address = rs_value + global_immediate; //store the data entry address to global
                
                //printf("exe():d_mem_entry_address for lw sw: %d, rs_value: %d, global_immediate: %d\n", d_mem_entry_address, rs_value, global_immediate);
                
                break;
        }
    }
    
    else if(branch == 1){ // beq
        rs_value = registerfile[registerfile_rs_index];
        rt_value = registerfile[registerfile_rt_index];
          
        switch(alu_op)
            
        {
                
            case 110:  // Sub
                //printf("Enter 110 with rs = %d and rt = %d\n", rs_value, rt_value);
                
                rd_value = rs_value - rt_value;
                
                if(rd_value == 0){
                    
                    alu_zero = 1; // means rd is equal to zero is true aka rs and rt are equal
          
                }
                
                else{
                    
                    alu_zero = 0; // means rd is equal to zero is false aka rs and rt are not equal
                    
                }
                
                break;
                
        }
        
        //Calculate branch_target address
        branch_target = 4 * global_immediate; //to shift-left-2 of the sign-extended offset input
        
        //branch_target = pc + 4 + branch_target; // add pc + 4 to it since branch start counting from next line
        branch_target = pc + 4 + branch_target;
        //printf("Calculate branch_target in exe() and get %d\n\n\n", branch_target);
        //printf("\n");

    }
    
    
    
    //Pass data to ex_mem buffer
    ex_mem[2] = pc;
    ex_mem[3] = pc + 4;
    ex_mem[4] = jump_target;
    
    ex_mem[5] = registerfile_rs_index;
    ex_mem[6] = registerfile_rt_index;
    ex_mem[7] = registerfile_rd_index;
    
    ex_mem[8] = global_immediate;
    
    ex_mem[9] = jump;
    ex_mem[10] = RegWrite;
    ex_mem[11] = MemWrite;
    ex_mem[12] = branch;
    ex_mem[13] = InstType;
    ex_mem[14] = alu_op;
    
    
    ex_mem[15] = alu_zero;
    ex_mem[16] = branch_target;
    ex_mem[17] = global_rd_value;
    ex_mem[18] = d_mem_entry_address;
    //
    
    
}









void mem(int* data_memory, int* registerfile){
    
    //Get data from ex_mem buffer
    pc = ex_mem[2];
    next_pc = ex_mem[3];
    jump_target = ex_mem[4];
    
    registerfile_rs_index = ex_mem[5];
    registerfile_rt_index = ex_mem[6];
    registerfile_rd_index = ex_mem[7];
    
    global_immediate = ex_mem[8];
    
    jump = ex_mem[9];
    RegWrite = ex_mem[10];
    MemWrite = ex_mem[11];
    branch = ex_mem[12];
    InstType = ex_mem[13];
    alu_op = ex_mem[14];
    
    
    alu_zero = ex_mem[15];
    branch_target = ex_mem[16];
    global_rd_value = ex_mem[17];
    d_mem_entry_address = ex_mem[18];
    //
    
    
    //printArrWithSpace(registerfile,32);
    //printArrWithSpace(data_memory,32);
    
    if(InstType == 0){
        //printf("check InstType == 0\n");
        switch(MemWrite)
            
        {
                
            case 0:  // lw
                //printf("check MemWrite case 0, d_mem_entry_address: %d \n", d_mem_entry_address);
                
                //divide by 4 to get the real index in data_memory
                d_mem_entry_value = data_memory[d_mem_entry_address/4]; //load the memory to global var
                
                //printf("prepare load value:%d \n", d_mem_entry_value);
                
                break;
                
                
                
            case 1:  // sw
                //printf("check MemWrite case 1\n");
                
                data_memory[d_mem_entry_address] = registerfile[registerfile_rt_index]; //store value in rt to data memory
                
                printf("memory 0x%x is modified to 0x%x by sw\n", d_mem_entry_address, data_memory[d_mem_entry_address]);
                
                break;
                
        }
    }
    /*
    else if(branch == 1 && alu_zero == 1){ //both branch condition satisfy
        //printf("mem()!!!!!alu_zero is: %d and branch is %d\n", alu_zero, branch);
    }*/
    
    
    //Store data to mem_wb buffer
    mem_wb[2] = pc;
    mem_wb[3] = pc + 4;
    mem_wb[6] = registerfile_rt_index;
    mem_wb[7] = registerfile_rd_index;
    
    mem_wb[10] = RegWrite;
    mem_wb[12] = branch;
    mem_wb[13] = InstType;
    
    mem_wb[15] = alu_zero;
    mem_wb[16] = branch_target;
    
    mem_wb[17] = global_rd_value;
    mem_wb[18] = d_mem_entry_address;
    
    mem_wb[19] = d_mem_entry_value;
    //
    
    
}

void writeBack(int* registerfile){
    
    //Get data from mem_wb buffer
    pc = mem_wb[2];
    next_pc = mem_wb[3];
    registerfile_rt_index = mem_wb[6];
    registerfile_rd_index = mem_wb[7];
    
    RegWrite = mem_wb[10];
    branch = mem_wb[12];
    InstType = mem_wb[13];
    
    alu_zero = mem_wb[15];
    
    branch_target = mem_wb[16];
    global_rd_value = mem_wb[17];
    d_mem_entry_address = mem_wb[18];
    d_mem_entry_value = mem_wb[19];
    //
    
    
    if(RegWrite == 1){ //write to register is true
        //printf("RegWrite == 1\n");
        
        switch(InstType)
            
        {
                
            case 10:   // R type
                //printf("writeBack() R type: registerfile_rd_index: %d, global_rd_value: %d\n",registerfile_rd_index, global_rd_value);
                
                registerfile[registerfile_rd_index] = global_rd_value; // write back to rd
                
                printf("$%s is modified to 0x%x \n", char_registers[registerfile_rd_index], registerfile[registerfile_rd_index]);
                
                break;
                
                
                
            case 0: // lw
                //printf("writeBack() lw: registerfile_rd_index: %d, d_mem_entry_value: %d\n",registerfile_rt_index, d_mem_entry_value);
                
                registerfile[registerfile_rt_index] = d_mem_entry_value;
                
                printf("$%s is modified to 0x%x \n", char_registers[registerfile_rt_index], registerfile[registerfile_rt_index]);
                
                break;
                
        }
        
    }
    
    
}

int main(){
    
    int ins_index = 0;
    int totalNumofIns = 0;
    char* instruction;
    
    char *FILENAME = malloc(256+1);
        
        printf("Enter filename:\n");
        scanf("%s", FILENAME);
    
    
    int stage = 0;
    
    //value that store inside the Registerfile
    int* registerfile = (int*) malloc(32*sizeof(int));
    
    for(int i = 0; i < 32; i++){
        *(registerfile+i) = 0;
    }
    
    //printArrWithSpace(registerfile,32);
    //initialize data memory
    int* data_memory = (int*) malloc(32*sizeof(int));
    
    for(int i = 0; i < 32; i++){
        *(data_memory+i) = 0;
    }
    
    //printArrWithSpace(data_memory,32);
    
    //array to store sign-extended offet
    sign_extended = (int*) malloc(32*sizeof(int));
    
    
    
    
    //assign spaces to pipeline registers(buffers)
    if_id = (int*) malloc(25*sizeof(int));
    id_ex = (int*) malloc(25*sizeof(int));
    ex_mem = (int*) malloc(25*sizeof(int));
    mem_wb = (int*) malloc(25*sizeof(int));
    
    
    
    
    //Note!!!!!!!!process below is very sensitive about number of \n within the txt file
    //get the total number of instruction to create instruction cache array:
    
    FILE *fp;
    char ch;
    
    //open file in read more
    fp=fopen(FILENAME,"r");
    
    if(fp==NULL) {
        printf("File \"%s\" does not exist!!!\n",FILENAME);
        return -1;
    }
    
    //read character by character and check for new line
    while((ch=fgetc(fp))!=EOF) {
        if(ch=='\n')
            totalNumofIns++;
    }
    totalNumofIns++;
    
    //close the file
    if(fp != NULL){
        fclose(fp); // close file
    }
    
    //print number of lines
    printf("Total number of lines are: %d\n",totalNumofIns);
    
    //create instruction cache array:
    char** ins_memory= (char**)malloc(totalNumofIns*sizeof(char*)); // allocating size for ins_memory
    
    //initialze instruction memory:
    FILE* f;
    
    while ((f = fopen(FILENAME, "r")) != NULL) {
        
        fseek(f,ins_index*34,SEEK_CUR); // take instruction at current pointer
        char *ins = (char*)malloc(33*sizeof(char));
        
        if(fgets(ins,34,f)!=NULL){
            ins_memory[ins_index] = (char*)malloc(33);
            strcpy(ins_memory[ins_index],ins); //store new ins into array
            //printf("Current inst: %s\n",ins_memory[ins_index]);
            ins_index++;
        }
    }
    
    if(f != NULL){
        fclose(f); // close file
    }
    
    
    
    //Initialize registerfile and d_mem with given sample_binary
    
    registerfile[9] = 32;
    registerfile[10] = 5;
    registerfile[16] = 112;
    
    data_memory[28] = 5;
    data_memory[29] = 16;
    
    /*Data store in each element of a buffer:
    0 : buffer is empty or not(0 means buffer is empty, 1-5 indicate different stages)
    1 : instruction index in ins_memory
    
    //Fetch()
    2: int pc = 0;
    3: int next_pc;
    //Global for Decode()
    4: int jump_target = 0;
    
    5: int registerfile_rs_index;
    6: int registerfile_rt_index;
    7: int registerfile_rd_index;
    
    8: int global_immediate;
    //control signals
    9: int jump = 0;
    10: int RegWrite;
    11: int MemWrite;
    12: int branch = 0;
    13: int InstType;
    14: int alu_op;
    //Global for Execute()
    15: int alu_zero = 0;
    16: int branch_target;
    17: int global_rd_value;
    18: int d_mem_entry_address = 0;
    //Global for mem()
    19: int d_mem_entry_value; //store the data memory value get from lw
    
    */
    
    int if_nop_counter = 0;
    int id_nop_counter = 0;
    
    bool is_in_nop = false;
    
    if_stage = 1; //tell CPU to start fetching the first instruction
    
    
    //the rest of the clock cycles:
    while(free){
        printf("\n");
        printf("Clock Cycle: %d\n", total_clock_cycles);
        
        /*
        printf("1___________________1\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("1___________________1\n");
        */
        
        //Put individual if statements from 5 to 1, so CPU move on with order 4->5, 3->4...1->2
        if( mem_wb[0] == 5){ //check if the mem_wb buffer is empty or not, 0 is empty, 5 is not empty
            //printf("!!!!!!!!!!!!nop counter in wb(): %d\n", id_nop_counter);
            
            //printf("WB() data from mem_wb buffer and processing instruction #%d\n", mem_wb[1]);
            writeBack(registerfile);
            
            
            
            //Flushing if branch is Taken
            if(mem_wb[12] == 1 && mem_wb[15] == 1){//if branch = 1 and alu_zero = 1, then branch is taken
                //printf("~~~~~~~~~~wb(): branch is Taken to %d. Do flushing\n",pc);
                //Flush
                //printf("control hazard detected (flush 3 instructions)\n");
                for(int i = 0; i < 25; i++){
                    if_id[i] = 0;
                    id_ex[i] = 0;
                    ex_mem[i] = 0;
                }
                mem_wb[0] = 0; // turn off current stage wb() after finish wb
                ex_mem[0] = 0;  // turn off mem()
                id_ex[0] = 0; // turn off exe()
                if_id[0] = 0; // turn off decode()
                //only left fetch() on to fetch() the taken branch instruction by setting global var flush to true:
                flush = true;
            }
            
            mem_wb[0] = 0; // turn off current stage
            
            
            if(mem_wb[1] == totalNumofIns - 1){ //reach the last intruction's writeback, where totalNumofIns - 1 is the index of the last intruction of inout txt file
                break;
            }
            

            if(id_nop_counter == 2 && if_nop_counter == 2 && is_in_nop == true){ //indicate second nop is finish
                //printf("!!!!!!!!!!second nop finish here: \n");
                printf("data hazard detected (nop2)\n");
                id_ex[0] = 3; //turn on exe()
                ex_mem[0] = 0; //turn off mem()
                
                //printf("In WB() id_ex[0] is %d\n", id_ex[0]);
                //printf("In WB() if_nop_counter is %d\n", if_nop_counter);
                total_clock_cycles++;
                continue;
                
            }
            
            int is_in_nop_d = 0;
            if(is_in_nop == true){
                is_in_nop_d = 1;
            }
            //printf("!!!!!!!!!!!!nop counter in wb(): %d with is_in_nop as %d\n", id_nop_counter, is_in_nop_d);
            
            
        }
        
        /*
        printf("2___________________2\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("2___________________2\n");
        */
        
        
        if( ex_mem[0] == 4){ //check if the ex_mem buffer is empty or not, 0 is empty, 4 is not empty
            //printf("!!!!!!!!!!!!nop counter in mem(): %d\n", id_nop_counter);
            
            int is_in_nop_d = 0;
            if(is_in_nop == true){
                is_in_nop_d = 1;
            }
            //printf("!!!!!!!!!!!!nop counter in mem(): %d with is_in_nop as %d\n", id_nop_counter, is_in_nop_d);
            
            if(is_in_nop == true){
                printf("data hazard detected (nop1)\n");
            }
            
            
            //printf("Mem() data from ex_mem buffer and processing instruction #%d\n", ex_mem[1]);
            mem(data_memory, registerfile);
                
            ex_mem[0] = 0;
            mem_wb[0] = 5;
                
            mem_wb[1] = ex_mem[1];
            
            
        }
        /*
        printf("3___________________3\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("3___________________3\n");
        */
        
        if(id_ex[0] == 3){ //check if the id_ex buffer is empty or not, 0 is empty, 3 is not empty
            
            int is_in_nop_d = 0;
            if(is_in_nop == true){
                is_in_nop_d = 1;
            }
            //printf("!!!!!!!!!!!!nop counter in exe(): %d with is_in_nop as %d\n", id_nop_counter, is_in_nop_d);
            
            //Global for Decode()
            /*
            5: int registerfile_rs_index;
            6: int registerfile_rt_index;
            7: int registerfile_rd_index;
            
            8: int global_immediate;
            //control signals
            9: int jump = 0;
            10: int RegWrite;
            11: int MemWrite;
            12: int branch = 0;
            13: int InstType;
            14: int alu_op;
            */
            
            /*Debugger
            printf("___________________\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("___________________\n");
            */
            
            
            if(is_in_nop == false){// if not in a nop process
            
                if(ex_mem[9] == 0 && ex_mem[12] == 0){//if current ins is not a jump or branch
                
                    if(ex_mem[13] == 10){ //if ins is R type
                    
                        //check if next ins rs or rt use its rd
                        if(id_ex[5] == ex_mem[7] || id_ex[6] == ex_mem[7]){
                            //if true, then nop ID() and IF() below by setting stage # to 6
                            if_nop_counter = 1;
                            id_nop_counter = 1;
                            is_in_nop = true;
                            //printf("data hazard detected\n");
                        }
                        
                    }
                    else if(ex_mem[13] == 0 && ex_mem[11] == 0){ //if ins is lw
                        
                        //check if next ins rs or rt use its rt
                        if(id_ex[5] == ex_mem[6] || id_ex[6] == ex_mem[6]){
                            //if true, then nop ID() and IF() below by setting stage # to 6
                            if_nop_counter = 1;
                            id_nop_counter = 1;
                            is_in_nop = true;
                            //printf("data hazard detected\n");
                        }
                    }
                
                }
            }
            else if(is_in_nop == true){ //real end of nop process
                id_nop_counter = 0;
                if_nop_counter = 0;
                is_in_nop = false;
                
                //printf("is nooooooooooooooooo nop anymore\n");
            }
            
            /*
            printf("???????????????\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("???????????????\n");
            */
                        
            if(if_nop_counter == 0 && id_nop_counter == 0 && is_in_nop == false){
                //printf("Exe() data from id_ex buffer and processing instruction #%d\n", id_ex[1]);
                execute(registerfile);
                
                id_ex[0] = 0;
                
                ex_mem[0] = 4;
                ex_mem[1] = id_ex[1];
            }
            else if(is_in_nop == true){ // if it need to stall
                //printf("!!!!!!!!first nop start here: \n");
                printf("data hazard detected (nop1)\n");
                id_ex[0] = 0; //so current ins don't move on
            
                //first nop only run mem() and wb() later
                ex_mem[0] = 4; //first nop
                mem_wb[0] = 5; //first nop
                
                if_nop_counter = 2; ////start first nop indicate with 1 using "continue;" below
                id_nop_counter = 2; //first nop
                total_clock_cycles++;
                continue; //go to first nop
            }
            /*
            printf("!!!!!!!!!!!!!!!!!!\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("!!!!!!!!!!!!!!!!!!\n");*/
            
            
            /*
            printf("/////////////////\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("////////////////\n"); */
            
        }
        /*
        printf("4___________________4\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("4___________________4\n");
        */
        
        
        
        if( if_id[0] == 2){ //check if the if_id buffer is empty or not, 0 is empty, 2 is not empty
            //printf("Decode() data from if_id buffer and processing instruction #%d\n", if_id[1]);
            //printf("code: %s\n", if_id_ins);
            
            decode(if_id_ins, sign_extended);
            
            
            
            
            //Print statement for detecting flushing
            if(mem_wb[12] == 1 && mem_wb[15] == 1){//if branch = 1 and alu_zero = 1, then branch is taken
                printf("control hazard detected (flush 3 instructions)\n");
            }
            //Print statement for detect if there are any data hazard: (nop will be apply in id_ex[0] == 3 aka exe() stage)
            else if(ex_mem[9] == 0 && ex_mem[12] == 0){//if current ins is not a jump or branch
                
                if(ex_mem[13] == 10){ //if ins is R type
                    
                    //check if next ins rs or rt use its rd
                    if(id_ex[5] == ex_mem[7] || id_ex[6] == ex_mem[7]){
                        printf("data hazard detected\n");
                    }
                        
                }
                else if(ex_mem[13] == 0 && ex_mem[11] == 0){ //if ins is lw
                        
                    //check if next ins rs or rt use its rt
                    if(id_ex[5] == ex_mem[6] || id_ex[6] == ex_mem[6]){
                            
                        printf("data hazard detected\n");
                    }
                }
            }
            
            
            
            
            if_id[0] = 0; //done with transferring data in buffer if_id to id_ex, buffer if_id is now consider empty
            id_ex[0] = 3; // id_ex is now fill with data
            
            id_ex[1] = if_id[1]; //pass the ins index to next stage
            
        }
        
        /*
        printf("5___________________5\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("5___________________5\n");
        */
        
        
        if(if_stage == 1){
            if((pc+4)/4 < totalNumofIns){ //if next_pc/4 is not greater than totalNumofIns
                if_id_ins = fetch(ins_memory);
                //printf("Fetch() instruction #%d\n", pc/4);
                if_id[0] = 2; //send current ins to stage 2
                if_id[1] = pc/4; //pass the ins index to stage 2 also
                
            }
            else{//aka if reach the end of all instruction
                if_stage == 0; //stop fetching by setting if_stage to 0
            }
        }
        
        
        /*
        printf("6___________________6\n");
            printArrWithSpace(if_id, 20);
            printArrWithSpace(id_ex, 20);
            printArrWithSpace(ex_mem, 20);
            printArrWithSpace(mem_wb, 20);
            printf("6___________________6\n");
        */
        
        if(is_in_nop == false){
            total_clock_cycles++;
        }
    }
    
    
    
    printf("program terminated: \n");
    printf("total execution time is %d cycles \n", total_clock_cycles);
    
    return 0;
    
}
