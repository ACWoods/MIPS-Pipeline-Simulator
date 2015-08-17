/*Implementation of functions related to classes*/
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include "projclasses.h"

#define	MAXCHAR 256

#define DEST 0
#define SRC 1
#define TARG 2
#define CONSRC 3
#define CONTRG 4
#define CONDES 5

using namespace std;

/*Each stage will keep track of its current instruction's registers and contents, but only one register file is created in the main program.*/

/*****IF Methods*****/
void IF_instruction_fetch::set_previous_instruction(int code)
{
	if(code == 0)
		strcpy(previous_instruction, "NOP");
	else if(code == 1)
		strcpy(previous_instruction, "ADD");
	else if(code == 2)
		strcpy(previous_instruction, "ADDI");
	else if(code == 3)
		strcpy(previous_instruction, "LW");
	else if(code == 4)
		strcpy(previous_instruction, "SW");
	else if(code == 5)
		strcpy(previous_instruction, "MV");
	else if(code == 6)
		strcpy(previous_instruction, "SLT");
	else if(code == 7)
		strcpy(previous_instruction, "BEQZ");
	else if(code == 8)
		strcpy(previous_instruction, "BNEZ");
	else if(code == 9)
		strcpy(previous_instruction, "J");
	else
		strcpy(previous_instruction, "NOP");
	
	return;
}	/*end set_previous_instruction()*/

char* IF_instruction_fetch::return_previous_instruction(void)
{
	return(previous_instruction);
}	/*end return_previous_instruction()*/
/*****End IF Methods*****/





/*****ID Methods*****/
int ID_instruction_decode::set_ic(int temp)
{
	instruction_code = temp;
	return(0);
}	/*end set_ic()*/


int ID_instruction_decode::return_ic(void)
{
	if((instruction_code < 1) || (instruction_code > 13))
		return(0);
	else
		return(instruction_code);
}	/*end return_ic()*/


void ID_instruction_decode::set_previous_instruction(int code)
{
	if(code == 0)
		strcpy(previous_instruction, "NOP");
	else if(code == 1)
		strcpy(previous_instruction, "ADD");
	else if(code == 2)
		strcpy(previous_instruction, "ADDI");
	else if(code == 3)
		strcpy(previous_instruction, "LW");
	else if(code == 4)
		strcpy(previous_instruction, "SW");
	else if(code == 5)
		strcpy(previous_instruction, "MV");
	else if(code == 6)
		strcpy(previous_instruction, "SLT");
	else if(code == 7)
		strcpy(previous_instruction, "BEQZ");
	else if(code == 8)
		strcpy(previous_instruction, "BNEZ");
	else if(code == 9)
		strcpy(previous_instruction, "J");
	else
		strcpy(previous_instruction, "NOP");
	
	return;
}	/*end set_previous_instruction()*/


char* ID_instruction_decode::return_previous_instruction(void)
{
	return(previous_instruction);
}	/*end return_previous_instruction()*/


void ID_instruction_decode::set_registers(float &destination_reg, float &source_reg, float &target_reg, instruction_struct instruction_input_parameters, register_struct &reg_file)
{
	/*This function sets the source, destination, and target registers based off the input string.  The EX stage in main() 
	will set offsets if any of these registers are not used. 

	Example: SW $S1, 100($S2) - FORMAT: (d,s,t) - 7 (for register $S1) will be set as the destination register (although this would be 
	considered the source register for a real pipeline), 100 will be set as the source register, and 8 (for register $S2)
	will be set as the target register.*/

	float temp1 = 0, temp2 = 0, temp3 = 0;

	if(!strcmp(instruction_input_parameters.instr_arg1, "$0"))
		temp1 = 0;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$T0"))
		temp1 = 1;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$T1"))
		temp1 = 2;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$T2"))
		temp1 = 3;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$T3"))
		temp1 = 4;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$T4"))
		temp1 = 5;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$S0"))
		temp1 = 6;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$S1"))
		temp1 = 7;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$S2"))
		temp1 = 8;
	else if(!strcmp(instruction_input_parameters.instr_arg1, "$S3"))
		temp1 = 9;
	else
		temp1 = atoi(instruction_input_parameters.instr_arg1);	/*Argument is an offset integer or immediate/literal*/ 


	if(!strcmp(instruction_input_parameters.instr_arg2, "$0"))
		temp2 = 0;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$T0"))
		temp2 = 1;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$T1"))
		temp2 = 2;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$T2"))
		temp2 = 3;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$T3"))
		temp2 = 4;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$T4"))
		temp2 = 5;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$S0"))
		temp2 = 6;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$S1"))
		temp2 = 7;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$S2"))
		temp2 = 8;
	else if(!strcmp(instruction_input_parameters.instr_arg2, "$S3"))
		temp2 = 9;
	else
		temp2 = atoi(instruction_input_parameters.instr_arg2);	/*Argument is an offset integer or immediate/literal*/ 

	
	if(!strcmp(instruction_input_parameters.instr_arg3, "$0"))
		temp3 = 0;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$T0"))
		temp3 = 1;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$T1"))
		temp3 = 2;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$T2"))
		temp3 = 3;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$T3"))
		temp3 = 4;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$T4"))
		temp3 = 5;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$S0"))
		temp3 = 6;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$S1"))
		temp3 = 7;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$S2"))
		temp3 = 8;
	else if(!strcmp(instruction_input_parameters.instr_arg3, "$S3"))
		temp3 = 9;
	else
		temp3 = atoi(instruction_input_parameters.instr_arg3);	/*Argument is an offset integer or immediate/literal*/ 


	destination_reg = temp1;
	source_reg = temp2;
	target_reg = temp3;
	
}	/*end set_registers()*/

/*****End ID Methods*****/





/*****EX Methods*****/
int EX_execution::set_ic(int temp)
{
	instruction_code = temp;
	return(0);
}	/*end set_ic()*/


int EX_execution::return_ic(void)
{
	if((instruction_code < 1) || (instruction_code > 13))
		return(0);
	else
		return(instruction_code);
}	/*end return_ic()*/


void EX_execution::set_previous_instruction(int code)
{
	if(code == 0)
		strcpy(previous_instruction, "NOP");
	else if(code == 1)
		strcpy(previous_instruction, "ADD");
	else if(code == 2)
		strcpy(previous_instruction, "ADDI");
	else if(code == 3)
		strcpy(previous_instruction, "LW");
	else if(code == 4)
		strcpy(previous_instruction, "SW");
	else if(code == 5)
		strcpy(previous_instruction, "MV");
	else if(code == 6)
		strcpy(previous_instruction, "SLT");
	else if(code == 7)
		strcpy(previous_instruction, "BEQZ");
	else if(code == 8)
		strcpy(previous_instruction, "BNEZ");
	else if(code == 9)
		strcpy(previous_instruction, "J");
	else
		strcpy(previous_instruction, "NOP");
	
	return;
}	/*end set_previous_instruction()*/


char* EX_execution::return_previous_instruction(void)
{
	return(previous_instruction);
}	/*end return_previous_instruction()*/

void EX_execution::set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest)
{
	EX_destination_reg = destination_reg;
	EX_source_reg = source_reg;
	EX_target_reg = target_reg;
	EX_contents_of_src = contents_of_src;
	EX_contents_of_targ = contents_of_targ;
	EX_contents_of_dest = contents_of_dest;
}	/*end set_registers()*/

void EX_execution::set_register_contents(int code, float temp)
{
	if(code == CONSRC)
		EX_contents_of_src = temp;
	else if(code == CONTRG)
		EX_contents_of_targ = temp;
	else if(code == CONDES)
		EX_contents_of_dest = temp;
}	/*end set_register_contents()*/


float EX_execution::return_register(int code)
{
	if(code == DEST)
		return(EX_destination_reg);
	else if(code == SRC)
		return(EX_source_reg);
	else if(code == TARG)
		return(EX_target_reg);
	else if(code == CONSRC)
		return(EX_contents_of_src);
	else if(code == CONTRG)
		return(EX_contents_of_targ);
	else if(code == CONDES)
		return(EX_contents_of_dest);
	else
		return(0);
}	/*end return_register()*/

/*****End EX Methods*****/





/*****MEM Methods*****/
int MEM_memory_access::set_ic(int temp)
{
	instruction_code = temp;
	return(0);
}	/*end set_ic()*/


int MEM_memory_access::return_ic(void)
{
	if((instruction_code < 1) || (instruction_code > 13))
		return(0);
	else
		return(instruction_code);
}	/*end return_ic()*/


void MEM_memory_access::set_previous_instruction(int code)
{
	if(code == 0)
		strcpy(previous_instruction, "NOP");
	else if(code == 1)
		strcpy(previous_instruction, "ADD");
	else if(code == 2)
		strcpy(previous_instruction, "ADDI");
	else if(code == 3)
		strcpy(previous_instruction, "LW");
	else if(code == 4)
		strcpy(previous_instruction, "SW");
	else if(code == 5)
		strcpy(previous_instruction, "MV");
	else if(code == 6)
		strcpy(previous_instruction, "SLT");
	else if(code == 7)
		strcpy(previous_instruction, "BEQZ");
	else if(code == 8)
		strcpy(previous_instruction, "BNEZ");
	else if(code == 9)
		strcpy(previous_instruction, "J");
	else
		strcpy(previous_instruction, "NOP");
	
	return;
}	/*end set_previous_instruction()*/


char* MEM_memory_access::return_previous_instruction(void)
{
	return(previous_instruction);
}	/*end return_previous_instruction()*/


float MEM_memory_access::set_read_offset(int code, float mem_offset_main)
{
	if(code == 0)	/*Read memory offset*/
		return(MEM_offset);
	else	/*Set memory offset*/
	{
		MEM_offset = mem_offset_main;
		return(0);
	}	/*end else*/
}	/*end set_read_offset*/


void MEM_memory_access::set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest)
{
	MEM_destination_reg = destination_reg;
	MEM_source_reg = source_reg;
	MEM_target_reg = target_reg;
	MEM_contents_of_src = contents_of_src;
	MEM_contents_of_targ = contents_of_targ;
	MEM_contents_of_dest = contents_of_dest;
}	/*end set_registers()*/


void MEM_memory_access::set_register_contents(int code, float temp)
{
	if(code == CONSRC)
		MEM_contents_of_src = temp;
	else if(code == CONTRG)
		MEM_contents_of_targ = temp;
	else if(code == CONDES)
		MEM_contents_of_dest = temp;
}	/*end set_register_contents()*/


float MEM_memory_access::return_register(int code)
{
	if(code == DEST)
		return(MEM_destination_reg);
	else if(code == SRC)
		return(MEM_source_reg);
	else if(code == TARG)
		return(MEM_target_reg);
	else if(code == CONSRC)
		return(MEM_contents_of_src);
	else if(code == CONTRG)
		return(MEM_contents_of_targ);
	else if(code == CONDES)
		return(MEM_contents_of_dest);
	else
		return(0);
}	/*end return_register()*/

/*****End MEM Methods*****/





/*****WB Methods*****/
int WB_write_back::set_ic(int temp)
{
	instruction_code = temp;
	return(0);
}	/*end set_ic()*/


int WB_write_back::return_ic(void)
{
	if((instruction_code < 1) || (instruction_code > 13))
		return(0);
	else
		return(instruction_code);
}	/*end return_ic()*/


void WB_write_back::set_previous_instruction(int code)
{
	if(code == 0)
		strcpy(previous_instruction, "NOP");
	else if(code == 1)
		strcpy(previous_instruction, "ADD");
	else if(code == 2)
		strcpy(previous_instruction, "ADDI");
	else if(code == 3)
		strcpy(previous_instruction, "LW");
	else if(code == 4)
		strcpy(previous_instruction, "SW");
	else if(code == 5)
		strcpy(previous_instruction, "MV");
	else if(code == 6)
		strcpy(previous_instruction, "SLT");
	else if(code == 7)
		strcpy(previous_instruction, "BEQZ");
	else if(code == 8)
		strcpy(previous_instruction, "BNEZ");
	else if(code == 9)
		strcpy(previous_instruction, "J");
	else
		strcpy(previous_instruction, "NOP");
	
	return;
}	/*end set_previous_instruction()*/


char* WB_write_back::return_previous_instruction(void)
{
	return(previous_instruction);
}	/*end return_previous_instruction()*/


void WB_write_back::set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest)
{
	WB_destination_reg = destination_reg;
	WB_source_reg = source_reg;
	WB_target_reg = target_reg;
	WB_contents_of_src = contents_of_src;
	WB_contents_of_targ = contents_of_targ;
	WB_contents_of_dest = contents_of_dest;
}	/*end set_registers()*/


void WB_write_back::set_register_contents(int code, float temp)
{
	if(code == CONSRC)
		WB_contents_of_src = temp;
	else if(code == CONTRG)
		WB_contents_of_targ = temp;
	else if(code == CONDES)
		WB_contents_of_dest = temp;
}	/*end set_register_contents()*/


float WB_write_back::return_register(int code)
{
	if(code == DEST)
		return(WB_destination_reg);
	else if(code == SRC)
		return(WB_source_reg);
	else if(code == TARG)
		return(WB_target_reg);
	else if(code == CONSRC)
		return(WB_contents_of_src);
	else if(code == CONTRG)
		return(WB_contents_of_targ);
	else if(code == CONDES)
		return(WB_contents_of_dest);
	else
		return(0);
}	/*end return_register()*/

/*****End WB Methods*****/
