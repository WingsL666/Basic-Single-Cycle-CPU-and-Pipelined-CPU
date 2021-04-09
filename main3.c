#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define FILENAME "sample_binary_self_test.txt"

//Global for Fetch()
int pc = 0;
int next_pc = 0;
char** ins_memory;

//Global for Decode()
int jump_target = 0;

int jump;
int RegDst;
int ALUSrc;
int MemtoReg;
int RegWrite;
int MemRead;
int MemWrite;
int branch;
int InstType;

//Register name
char **char_registers = (char *[]) {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3","t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};



void fetch(char** ins_memory) {
	int updated_pc;
    next_pc = pc + 4;
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
        printf("Sign extended code:");
        printArr(sign_extended, 32);
    }
    else{
        for(int i = 0;i <= 15; i++){ //imm 16-31 has 16 bits
            *(sign_extended+i) = 0;
        }
        printf("Sign extended code:");
        printArr(sign_extended, 32);
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
		arr[i] = n % 2;    
		n = n/2;    
	}    
	printf("Convert %d to binary:",pc);
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



void call_R_format(int* code, char** reg_arr){ //Use in Decode()
    printf("Instruction Type: R \n");
    
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
            break;
            
        case 34:  // Sub
            r_operation = "sub";
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

void call_J_format(int* code, int opcode){ //Use in Decode()
    printf("Instruction Type: J \n");
    
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
    
    printf("Operation: %s \n",j_operation);
    
    
    for(int i = 6;i <= 31; i++){ //target 6-31 has 26 bits
        target_address = target_address + (*(code+i)*twoToPower(25-counter_power));//TwoToPower(25-counter_power) gets 2^25...2^0
        counter_power++;
    }
    printf("Jump address: %d \n",target_address);
	
	
	//First shift-left-2 and then merge first 4 bits from pc to get target_address
	jump_target = target_address*4;
	
	int* pc_binary = (int*) malloc(32*sizeof(int));
	pc_binary = convertDecimalto32bitBinary(pc);
	
	int merge4 = getFirst4bitReturnAsDecimal(pc_binary);
	printf("merge4:%d\n",merge4);
	
	jump_target += merge4;
    
}


void call_I_format(int* code, char** reg_arr, int opcode, int* sign_extended){ //Use in Decode()
    printf("Instruction Type: I \n");
    
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
			MemWrite = 1;
			branch = 1;
			InstType = 01;
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
    printf("(R%d)\n",rt);
    
    
    //assume if immediate offet is positive
    counter_power = 0;
    for(int i = 16;i <= 31; i++){ //imm 16-31 has 16 bits
        immediate = immediate + (*(code+i)*twoToPower(15-counter_power));//TwoToPower(15-counter_power) gets 2^15...2^0
        counter_power++;
    }
    //
    
    //strcmp(...,...) == 0 means the two string are the same
    if(strcmp(i_operation,"lw") == 0 || strcmp(i_operation,"sw") == 0){
        
        printf("leftmost bit: %d\n", *(code+16));
        
        //16 is the leftmost bit of the immediate
        if(*(code+16) == 1){
            printf("Negative offset\n");
            sign_extension(*(code+16),code, sign_extended);
            
            //convertNegBinaryToDecimal using 2's complement
            immediate = convertNegBinaryToDecimal(code);
            
            printf("Immediate: %d\n",immediate);
        }
        else{
            printf("Positive offset\n");
            sign_extension(*(code+16),code, sign_extended);
            printf("Immediate: %d\n",immediate);
        }
        
    }
    else{
        if(*(code+16) == 1){
            printf("Negative offset\n");
            
            //convertNegBinaryToDecimal using 2's complement
            immediate = convertNegBinaryToDecimal(code);
            
            printf("Immediate: %d\n",immediate);
        }
        else{
            printf("Positive offset\n");
            printf("Immediate: %d\n",immediate);
        }
        
    }
    
    
}








void decode(char* ins, int* sign_extended){
    int opcode = 0;
    int* machineCode = (int*) malloc(32*sizeof(int));
    printf("Machine code ins: %s\n", ins);
    
    
    /*
     
     if (operation == "add"){
     add = 0010;
     }
     else if  (operation == "subtract"){
     subtract = 0110;
     }
     else if  (operation == "or"){
     or = 0001;
     }
     else if  (operation == "and"){
     and = 0000;
     }
     else if  (operation == "slt"){
     slt = 0111;
     }
     else if  (operation == "nor"){
     nor = 1100;
     }
     */
    
    /*
     if (inst = Rtype) {
        jump = 0;
        RegDst = 1;
        ALUSrc = 0;
        MemtoReg = 0;
        RegWrite = 1;
        MemRead = 0;
        MemWrite = 0;
        branch = 0;
        InstType = 10;
    }
     else if ( inst = lw)
        {
        jump = 0;
        RegDst = 0;
        ALUSrc = 1;
        MemtoReg = 1;
        RegWrite = 1;
        MemRead = 1;
        MemWrite = 0;
        branch = 0;
        InstType = 0;
        }
    else if ( inst = sw)
        {
        jump = 0;
        ALUSrc = 1;
        RegWrite = 0;
        MemRead = 0;
        MemWrite = 1;
        branch = 0;
        InstType = 0;
        }
     else if ( inst = beq)
        {
        jump = 0;
        ALUSrc = 0;
        RegWrite = 0;
        MemRead = 0;
        MemWrite = 1;
        branch = 1;
        InstType = 01;
        }
    else if ( inst = beq)
        {
        jump = 0;
        ALUSrc = 0;
        RegWrite = 0;
        MemRead = 0;
        MemWrite = 1;
        branch = 1;
        InstType = 01;
        }
    else if ( inst = jump)
        {
        jump = 1;
        RegWrite = 0;
        MemRead = 0;
        MemWrite = 0;
        branch = 0;
        }
    }
     */
    
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
        call_R_format(machineCode, char_registers);
    }
    else if(opcode == 2 || opcode == 3){
        call_J_format(machineCode, opcode);
    }
    else{
        call_I_format(machineCode, char_registers, opcode, sign_extended);
    }
}




int main(){
    int ins_index = 0;
    int totalNumofIns = 0;
    
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
    //close the file
    if(fp != NULL){
        fclose(fp); // close file
    }
    //print number of lines
    printf("Total number of lines are: %d\n",totalNumofIns);
    
    
    //create instruction cache array:
    char** ins_memory= (char**)malloc(totalNumofIns*sizeof(char*)); // allocating size for ins_memory
    
    FILE* f;
    
    while ((f = fopen(FILENAME, "r")) != NULL) {
        
        fseek(f,ins_index*34,SEEK_CUR); // take instruction at current pointer
        char *ins = (char*)malloc(34*sizeof(char));
        
        if(fgets(ins,34,f)!=NULL){
            ins_memory[ins_index] = (char*)malloc(34);
            strcpy(ins_memory[ins_index],ins); //store new ins into array
            //printf("Current inst: %s\n",ins_memory[ins_index]);
            ins_index++;
        }
        
    }
    if(f != NULL){
        fclose(f); // close file
    }
    
    
    
    //value that store inside the Registerfile
    int* registerfile = (int*) malloc(32*sizeof(int));
	for(int i = 0; i < 32; i++){
		*(registerfile+i) = 0;
	}		
	printArrWithSpace(registerfile,32);
    
	
    //array to store sign-extended offet
    int* sign_extended = (int*) malloc(32*sizeof(int));
	
	
	//test convertDecimalto32bitBinary() and getFirst4bitReturnAsDecimal() for getting jump_target
	/*
	int* pc_binary = (int*) malloc(32*sizeof(int));
	pc_binary = convertDecimalto32bitBinary(5800);
	
	int merge4 = getFirst4bitReturnAsDecimal(pc_binary);
	printf("merge4:%d\n",merge4);
	*/
    
    
    
    for(int i = 0; i < totalNumofIns; i++){
        decode(*(ins_memory + i), sign_extended);
        /* Debug for what each instruction array stores decimal 48 as char 1
         for(int j = 0; j < 32; j++){
         printf("%s\n", *(*(ins_memory+i)+j));
         }
         printf("next");
         */
        printf("\n");
    }
    
    return 0;
}
