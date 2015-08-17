/*INSTRUCTION CODES
/* 0-NOP
/* 1-ADD
/* 2-ADDI
/* 3-LW
/* 4-SW
/* 5-MV
/* 6-SLT
/* 7-BEQZ
/* 8-BNEZ
/* 9-J
/* 10-PRINTPC
/* 11-PRINTREG
/* 12-PRINTEX
/* 13-EXIT
/*
/*TEN REGISTERS IN MEM
/* 0 - ZERO REGISTER
/* 1-5 - $T0-$T4
/* 6-9 - $S0-$S3
/**********************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "projclasses.h"

#define NUM_CL_ARGS 4
#define MAXSIZE 256

#define DEST 0
#define SRC 1
#define TARG 2
#define CONSRC 3
#define CONTRG 4
#define CONDES 5

# define NOP 0
# define ADD 1
# define ADDI 2
# define LW 3
# define SW 4
# define MV 5
# define SLT 6
# define BEQZ 7
# define BNEZ 8
# define J 9
# define PRINTPC 10
# define PRINTREG 11
# define PRINTEX 12
# define EXIT 13


using namespace std;


void tokenize_instructions(int &IF_count, char *token, char (&instruction_input)[MAXSIZE][MAXSIZE], instruction_struct &instruction_input_parameters);
void tokenize_memory(int &mem_count, char *token, char (&main_memory_input)[MAXSIZE][MAXSIZE], data_struct &data_memory_parameters);


int main(int argc, char *argv[])
{
	if((argc < NUM_CL_ARGS) || !strcmp(argv[1], "/0"))	/*Exit if no value was passed*/
	{		
		cout<<"ERROR: Proper command line arguments not passed to program. Program will now exit."<<endl;
		cout<<"Syntax: pipeline.exe -instruction_input_file -data_input_file -mode"<<endl;
		exit(-1);
	}	/*end if*/

	int i = 0, mode = 0, PC = 0, branch_PC = 0, no_of_instructions = 0, no_of_memory_blocks = 0;
	int stalled_instruction = 0, count_temp = 0;
	int IF_count = 0, instruction_code = 0, mem_count = 0, current_clock_cycle = 1;
	float Rd_of_previous_instr = -1, Rd_of_2nd_previous_instr = -2, Rd_of_3rd_previous_instr = -3;	/*Keeps track of previous target registers to determine dependencies. Initialized to negative numbers since there are no negative registers.*/
	float Rt_of_previous_instr = -1, Rt_of_2nd_previous_instr = -2, Rt_of_3rd_previous_instr = -3;	/*Keeps track of previous target registers to determine RAW dependencies. Initialized to negative numbers since there are no negative registers.*/
	float Rs_of_previous_instr = -1, Rs_of_2nd_previous_instr = -2, Rs_of_3rd_previous_instr = -3;	/*Keeps track of previous source registers to determine WAR anti-dependencies. Initialized to negative numbers since there are no negative registers.*/
	float source_reg = -1, target_reg = -1, destination_reg = -1, Rd_stall = 0, Rs_stall = 0, Rt_stall = 0;
	float branch_offset = 0, memory_offset = 0;
	float contents_of_src = 0, contents_of_targ = 0, contents_of_dest = 0, temp = 0, memory_temp = 0;
	char instruction_input[MAXSIZE][MAXSIZE], main_memory_input[MAXSIZE][MAXSIZE];
	char instr_filename[MAXSIZE], data_filename[MAXSIZE], *token, instr_initializer[MAXSIZE] = {"NOP"};
	bool branch_flag = false, pipeline_stall = false, pipeline_stalled_previously = false, mem_success = false, branched_previously = false;
	bool prev_stall = false, second_prev_stall = false, third_prev_stall = false, data_forwarded = false;
	bool dest_stall = false, trg_stall = false, src_stall = false, LW_pipeline_stall = false;
	bool dest_prev_stall = false, dest_sec_prev_stall = false, dest_third_prev_stall = false, trg_prev_stall = false, trg_sec_prev_stall = false;
	bool trg_third_prev_stall = false, src_prev_stall = false, src_sec_prev_stall = false, src_third_prev_stall = false;
	size_t length;

	instruction_struct instruction_input_parameters, ID_instruction_parameters;
	data_struct data_memory_parameters;
	memory_struct data_memory[MAXSIZE];
	register_struct register_file = {0};

	IF_instruction_fetch IF_stage;
	ID_instruction_decode ID_stage;
	EX_execution EX_stage;
	MEM_memory_access MEM_stage;
	WB_write_back WB_stage;


	string instruction_input_buffer, data_input_buffer;

	fstream instr_infile, data_infile;
	
	/**Instruction Input Array Initializer**/
	for(i = 0; i < MAXSIZE; i++)
	{
		strcpy(instruction_input[i], "NOP");
	}	/*end for*/
	
	/**Initializing Class Variables**/
	IF_stage.set_previous_instruction(0);
	ID_stage.set_previous_instruction(0);
	EX_stage.set_previous_instruction(0);
	MEM_stage.set_previous_instruction(0);
	WB_stage.set_previous_instruction(0);

	ID_stage.set_ic(0);
	EX_stage.set_ic(0);
	MEM_stage.set_ic(0);
	WB_stage.set_ic(0);

	token = instr_initializer;	/*Intializing "token" to avoid compiler warning*/

	/**Initializing ID_instruction_parameters*/
	strcpy(ID_instruction_parameters.instr_address, "");
	strcpy(ID_instruction_parameters.instr_arg1, "");
	strcpy(ID_instruction_parameters.instr_arg2, "");
	strcpy(ID_instruction_parameters.instr_arg3, "");
	strcpy(ID_instruction_parameters.instruction, "");

	/**Reading Command Line Arguments**/
	//strcpy(instr_filename, "instr_in_branch.txt");	/*Debugging purposes only*/
	//strcpy(data_filename, "data_in_test.txt");	/*Debugging purposes only*/
	//mode = 2;	/*Debugging purposes only*/
	strcpy(instr_filename, argv[1]);
	strcpy(data_filename, argv[2]);
	mode = atoi(argv[3]);


	/**Input File Opening**/
	instr_infile.open(instr_filename, fstream::in);
	data_infile.open(data_filename, fstream::in);
	if(instr_infile.is_open() == false)	/*Error checking instruction input file opening*/
	{
		printf("\nInput file for instructions not found.  Program will exit.");
		exit(-2);
	}	/*end if*/
	if(data_infile.is_open() == false)	/*Error checking data input file opening*/
	{
		printf("\nInput file for data not found.  Program will exit.");
		exit(-3);
	}	/*end if*/
	




	/*****Input File Read-in Block*****/
	/*Use getline() to read in each line from the instruction input file and each line from the data input file
	/*and place the code in the input arrays defined above.  The ID stage can separate each instruction into tokens.

	/*Instruction Input File Read*/
	for(i = 0; instr_infile != NULL; i++)	/*Reads each line of the input file until the end of the file is reached*/
	{
		if(i == MAXSIZE)	/*If the input file has more than 256 lines, ignore the rest of the lines*/
			break;
		
		if(!getline(instr_infile, instruction_input_buffer))	/*Required to prevent getline() from reading in the last line twice*/
			break;

		no_of_instructions++;
		length = instruction_input_buffer.copy(instruction_input[i],MAXSIZE,0);
		instruction_input[i][length] = '\0';	/*Setting the last element of the input string to be the null character; "/0" is ZERO, "\0" is NULL*/
	}	/*end for*/

	/*Data Memory File Read*/
	for(i = 0; data_infile != NULL; i++)	/*Reads each line of the input file until the end of the file is reached*/
	{
		if(i == MAXSIZE)	/*If the input file has more than 256 lines, ignore the rest of the lines*/
			break;
		
		if(!getline(data_infile, data_input_buffer))	/*Required to prevent getline() from reading in the last line twice*/
			break;

		no_of_memory_blocks++;
		length = data_input_buffer.copy(main_memory_input[i],MAXSIZE,0);
		main_memory_input[i][length] = '\0';	/*Setting the last element of the input string to be the null character; "/0" is ZERO, "\0" is NULL*/
	}	/*end for*/


	if(no_of_instructions > MAXSIZE)
	{
		cout<<"ERROR: Number of read-in instructions is greater than the maximum size of "<<MAXSIZE<<". Program will exit."<<endl;
		exit(-2);
	}	/*end if*/
	if(no_of_memory_blocks > MAXSIZE)
	{
		cout<<"ERROR: Number of read-in memory blocks is greater than the maximum size of "<<MAXSIZE<<". Program will exit."<<endl;
		exit(-3);
	}	/*end if*/
	/*****End Input File Read-in Block*****/





	/**Storing Read-in Data Memory Values**/
	for(mem_count = 0; mem_count != no_of_memory_blocks; mem_count++)
	{
		tokenize_memory(mem_count, token, main_memory_input, data_memory_parameters);
		
		data_memory[mem_count].data_address = atof(data_memory_parameters.data_address);	/*Convert string to float*/
		data_memory[mem_count].data = atof(data_memory_parameters.data);				
	}	/*end for*/





	switch(mode)
	{
		
		/**********************************************************************************************************/
		/*																										  */
		/*																										  */
		/**************************************************TASK 1**************************************************/
		/*																										  */
		/*																										  */
		/**********************************************************************************************************/



		case 1:	/*Without Forwarding*/

			
			while(IF_count <= no_of_instructions)	/*"Less than or equal to" condition allows for the EXIT instruction to run through the pipeline*/
			{
				/**************************************************IF Stage**************************************************/


				if(IF_count < no_of_instructions)	/*This condition is necessary so that the exit command can run through the pipeline*/
				{
					if(pipeline_stall == true)
						pipeline_stall = false;	/*This stage does nothing when the pipeline is stalled*/
				
					else
					{
						/*Tokenizing Instructions: The program will assume that memory addresses are tab- or space-delimited.  
						These instruction and data parameters must be separated using the defined structs.  
						Memory offsets for instructions that utilize memory will be placed in the second instruction argument.*/

						if(branch_flag == false)
							tokenize_instructions(IF_count, token, instruction_input, instruction_input_parameters);
						

						if(branch_flag == true)
						{
							count_temp = IF_count;
							IF_count = 0;
							for(i = 0; atoi(instruction_input_parameters.instr_address) != PC; i++)	/*Fetch the branched instruction*/
								tokenize_instructions(IF_count, token, instruction_input, instruction_input_parameters);

							PC = branch_PC + 4 + branch_offset*4;	/*The formula for the new PC is PC + 4 + offset*4.*/
							if(i == 0)	/*The branched instruction is already in ID*/
								IF_count = count_temp + 1;
						}	/*end if*/
						


						/*****Meta-Instruction Block*****/
						/*Because of the way the instruction inputs are read in, the names of the instruction input parameters that hold the 
						meta-instruction information do not accurately describe their contents*/

						if(!strcmp(instruction_input_parameters.instr_address, "PRINTPC"))	/*strcmp() returns 0 if strings are equal*/
						{
							cout<<endl;
							cout<<"PC = "<<PC<<endl;
							continue;
						}	/*end if*/

						else if(!strcmp(instruction_input_parameters.instr_address, "PRINTREG")) 
						{
							if(!strcmp(instruction_input_parameters.instruction, "$0"))
								temp = 0;
							else if(!strcmp(instruction_input_parameters.instruction, "$T0"))
								temp = register_file.reg_1;
							else if(!strcmp(instruction_input_parameters.instruction, "$T1"))
								temp = register_file.reg_2;
							else if(!strcmp(instruction_input_parameters.instruction, "$T2"))
								temp = register_file.reg_3;
							else if(!strcmp(instruction_input_parameters.instruction, "$T3"))
								temp = register_file.reg_4;
							else if(!strcmp(instruction_input_parameters.instruction, "$T4"))
								temp = register_file.reg_5;
							else if(!strcmp(instruction_input_parameters.instruction, "$S0"))
								temp = register_file.reg_6;
							else if(!strcmp(instruction_input_parameters.instruction, "$S1"))
								temp = register_file.reg_7;
							else if(!strcmp(instruction_input_parameters.instruction, "$S2"))
								temp = register_file.reg_8;
							else if(!strcmp(instruction_input_parameters.instruction, "$S3"))
								temp = register_file.reg_9;
							else
							{
								cout<<"ERROR: Meta-Instruction not properly formatted.  Program will exit."<<endl;
								exit(-6);
							}	/*end else*/

							cout<<endl;
							cout<<instruction_input_parameters.instruction<<" = "<<temp<<endl;
							continue;
						}	/*end if*/

						else if(!strcmp(instruction_input_parameters.instr_address, "PRINTEX"))
						{
							cout<<endl;
							cout<<"Clock cycle = "<<current_clock_cycle<<endl;
							cout<<"IF: "<<IF_stage.return_previous_instruction()<<endl;
							cout<<"ID: "<<ID_stage.return_previous_instruction()<<endl;
							cout<<"EX: "<<EX_stage.return_previous_instruction()<<endl;
							cout<<"MEM: "<<MEM_stage.return_previous_instruction()<<endl;
							cout<<"WB: "<<WB_stage.return_previous_instruction()<<endl;
							continue;
						}	/*end if*/
						/*****End Meta-Instruction Block*****/

						if(branch_flag == false)
							PC = PC + 4;

					}	/*end else*/
				}	/*end if*/


				


				/**************************************************ID Stage**************************************************/
				

				pipeline_stall = false;
				

				if(branch_flag == true)	/*Check for branch*/
					ID_stage.set_ic(NOP);


				/*****Check for Pipeline Stall Block*****/
				else 
				{
					if(source_reg > 0 && source_reg < 10)	/*Register must be between 1 and 9, the valid range for writable registers*/
					{
						if(source_reg == Rd_of_previous_instr || source_reg == Rd_of_2nd_previous_instr || source_reg == Rd_of_3rd_previous_instr)
							pipeline_stall = true;
					}	/*end if*/

					if(target_reg > 0 && target_reg < 10)
					{
						if(target_reg == Rd_of_previous_instr || target_reg == Rd_of_2nd_previous_instr || target_reg == Rd_of_3rd_previous_instr)
							pipeline_stall = true;
					}	/*end if*/

					if(destination_reg > 0 && destination_reg < 10)
					{
						if(destination_reg == Rd_of_previous_instr || destination_reg == Rd_of_2nd_previous_instr || destination_reg == Rd_of_3rd_previous_instr)
						{
							if(ID_instruction_parameters.instruction == "MV" || ID_instruction_parameters.instruction == "BNEZ" || ID_instruction_parameters.instruction == "BEQZ")
								pipeline_stall = true;
						}	/*end if*/
					}	/*end if*/

					if(EX_stage.return_register(SRC) > 0 && EX_stage.return_register(SRC) < 10)	/*Since source_reg and target_reg are not set to the stalled registers yet,*/
					{																			/*the program must check to see if the registers that caused the stall are still in use.*/
						if(EX_stage.return_register(SRC) == Rd_of_previous_instr || EX_stage.return_register(SRC) == Rd_of_2nd_previous_instr || EX_stage.return_register(SRC) == Rd_of_3rd_previous_instr)
							pipeline_stall = true;
					}	/*end if*/

					if(EX_stage.return_register(TARG) > 0 && EX_stage.return_register(TARG) < 10)
					{
						if(EX_stage.return_register(TARG) == Rd_of_previous_instr || EX_stage.return_register(TARG) == Rd_of_2nd_previous_instr || EX_stage.return_register(TARG) == Rd_of_3rd_previous_instr)
							pipeline_stall = true;
					}	/*end if*/

					if(EX_stage.return_register(DEST) > 0 && EX_stage.return_register(DEST) < 10)
					{
						if(EX_stage.return_register(DEST) == Rd_of_previous_instr || EX_stage.return_register(DEST) == Rd_of_2nd_previous_instr || EX_stage.return_register(DEST) == Rd_of_3rd_previous_instr)
						{
							if(EX_stage.return_ic() == MV || EX_stage.return_ic() == BNEZ || EX_stage.return_ic() == BEQZ)
								pipeline_stall = true;
						}	/*end if*/
					}	/*end if*/


				}	/*end else*/
				
				if(pipeline_stalled_previously == true)	/*Necessary to prevent ID_stage from resetting the instruction type*/
				{
					pipeline_stalled_previously = false;
					ID_stage.set_ic(stalled_instruction);
					if(branched_previously == true)
						ID_stage.set_ic(NOP);
					destination_reg = Rd_stall;
					source_reg = Rs_stall;
					target_reg = Rt_stall;
				}	/*end else if*/
				/*****End Check for Pipeline Stall block*****/





				else
				{
					/*****Set Instruction Code Block*****/
					if(!strcmp(ID_instruction_parameters.instruction, "ADD"))
					{
						ID_stage.set_ic(ADD);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "ADDI"))
					{
						ID_stage.set_ic(ADDI);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "LW"))
					{
						ID_stage.set_ic(LW);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "SW"))
					{
						ID_stage.set_ic(SW);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "MV"))
					{
						ID_stage.set_ic(MV);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "SLT"))
					{
						ID_stage.set_ic(SLT);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "BEQZ"))
					{
						ID_stage.set_ic(BEQZ);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "BNEZ"))
					{
						ID_stage.set_ic(BNEZ);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "J"))
					{
						ID_stage.set_ic(J);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instr_address, "EXIT"))
					{
						ID_stage.set_ic(EXIT);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else
					{
						ID_stage.set_ic(NOP);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end else*/


					
					/*****End Set Instruction Code Block*****/




					if(pipeline_stall == false)	/*Setting PC for stalls and exit conditions*/
					{
						if(pipeline_stalled_previously == false)	/*If the pipeline was not stalled previously, PC must be decremented on this cycle if NOP or EXIT is run*/
							if(ID_stage.return_ic() == NOP && ID_stage.return_ic() == EXIT)
								PC = PC - 4;
					}	/*end if*/
				}	/*end else*/

				if(pipeline_stall == true)
				{
					pipeline_stalled_previously = true;
					stalled_instruction = ID_stage.return_ic();	/*Holds the stalled instruction temporarily*/
					Rd_stall = destination_reg;
					Rs_stall = source_reg;
					Rt_stall = target_reg;

					Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
					Rt_of_2nd_previous_instr = Rt_of_previous_instr;
					Rt_of_previous_instr = 0;

					Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
					Rs_of_2nd_previous_instr = Rs_of_previous_instr;
					Rs_of_previous_instr = 0;

					Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
					Rd_of_2nd_previous_instr = Rd_of_previous_instr;
					Rd_of_previous_instr = 0;
				}	/*end if*/


			
				if(branch_flag == false)	/*These parameters must be set here instead of the post-pipeline operations because the branch_flag may be reset depending on the operation in EX*/
				{
					if(pipeline_stall == false)
						ID_instruction_parameters = instruction_input_parameters;
					else if(pipeline_stall == true && branch_PC != 0)	/*If the pipeline is stalled after a branch, the instruction in ID needs to be cleared*/
					{
						ID_stage.set_ic(NOP);
					}	/*end if*/
				}	/*end if*/
				else if(branch_offset > 1)	/*If branch_offset = 1, ID will have its correct instruction and should not be cleared.*/
				{
					if(branch_flag == true)	/*If this isn't here (branch_flag == true, the instr_in_branch.txt test file will load a previous instruction(ADD) into ID.*/
						ID_stage.set_ic(NOP);
				}	/*end else if*/
					
							



				/**************************************************EX Stage**************************************************/

			
				if(branch_flag == true)	/*Previous cycle had a branch so the instruction passed to EX from ID is invalid*/
				{
					EX_stage.set_ic(NOP);
					branch_flag = false;
				}	/*end if*/
				
				else if(pipeline_stall == false)
				{
					/*****Temporary Store Block*****/
					/*The contents of the source, target, and destination registers need to be temporarily stored by the EX stage
					for calculations*/
					if(EX_stage.return_register(SRC) == 0)
						EX_stage.set_register_contents(CONSRC, register_file.reg_0);
					else if(EX_stage.return_register(SRC) == 1)
						EX_stage.set_register_contents(CONSRC, register_file.reg_1);
					else if(EX_stage.return_register(SRC) == 2)
						EX_stage.set_register_contents(CONSRC, register_file.reg_2);
					else if(EX_stage.return_register(SRC) == 3)
						EX_stage.set_register_contents(CONSRC, register_file.reg_3);
					else if(EX_stage.return_register(SRC) == 4)
						EX_stage.set_register_contents(CONSRC, register_file.reg_4);
					else if(EX_stage.return_register(SRC) == 5)
						EX_stage.set_register_contents(CONSRC, register_file.reg_5);
					else if(EX_stage.return_register(SRC) == 6)
						EX_stage.set_register_contents(CONSRC, register_file.reg_6);
					else if(EX_stage.return_register(SRC) == 7)
						EX_stage.set_register_contents(CONSRC, register_file.reg_7);
					else if(EX_stage.return_register(SRC) == 8)
						EX_stage.set_register_contents(CONSRC, register_file.reg_8);
					else if(EX_stage.return_register(SRC) == 9)
						EX_stage.set_register_contents(CONSRC, register_file.reg_9);


					if(EX_stage.return_register(TARG) == 0)
						EX_stage.set_register_contents(CONTRG, register_file.reg_0);
					else if(EX_stage.return_register(TARG) == 1)
						EX_stage.set_register_contents(CONTRG, register_file.reg_1);
					else if(EX_stage.return_register(TARG) == 2)
						EX_stage.set_register_contents(CONTRG, register_file.reg_2);
					else if(EX_stage.return_register(TARG) == 3)
						EX_stage.set_register_contents(CONTRG, register_file.reg_3);
					else if(EX_stage.return_register(TARG) == 4)
						EX_stage.set_register_contents(CONTRG, register_file.reg_4);
					else if(EX_stage.return_register(TARG) == 5)
						EX_stage.set_register_contents(CONTRG, register_file.reg_5);
					else if(EX_stage.return_register(TARG) == 6)
						EX_stage.set_register_contents(CONTRG, register_file.reg_6);
					else if(EX_stage.return_register(TARG) == 7)
						EX_stage.set_register_contents(CONTRG, register_file.reg_7);
					else if(EX_stage.return_register(TARG) == 8)
						EX_stage.set_register_contents(CONTRG, register_file.reg_8);
					else if(EX_stage.return_register(TARG) == 9)
						EX_stage.set_register_contents(CONTRG, register_file.reg_9);


					if(EX_stage.return_register(DEST) == 0)
						EX_stage.set_register_contents(CONDES, register_file.reg_0);
					else if(EX_stage.return_register(DEST) == 1)
						EX_stage.set_register_contents(CONDES, register_file.reg_1);
					else if(EX_stage.return_register(DEST) == 2)
						EX_stage.set_register_contents(CONDES, register_file.reg_2);
					else if(EX_stage.return_register(DEST) == 3)
						EX_stage.set_register_contents(CONDES, register_file.reg_3);
					else if(EX_stage.return_register(DEST) == 4)
						EX_stage.set_register_contents(CONDES, register_file.reg_4);
					else if(EX_stage.return_register(DEST) == 5)
						EX_stage.set_register_contents(CONDES, register_file.reg_5);
					else if(EX_stage.return_register(DEST) == 6)
						EX_stage.set_register_contents(CONDES, register_file.reg_6);
					else if(EX_stage.return_register(DEST) == 7)
						EX_stage.set_register_contents(CONDES, register_file.reg_7);
					else if(EX_stage.return_register(DEST) == 8)
						EX_stage.set_register_contents(CONDES, register_file.reg_8);
					else if(EX_stage.return_register(DEST) == 9)
						EX_stage.set_register_contents(CONDES, register_file.reg_9);
					/*****End Temporary Store Block*****/





					/*****Instruction Calculation Block*****/
					if(EX_stage.return_ic() ==	1)	/*ADD*/
					{
						EX_stage.set_register_contents(CONDES, (EX_stage.return_register(CONSRC) + EX_stage.return_register(CONTRG)));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 2)	/*ADDI*/
					{
						EX_stage.set_register_contents(CONDES, (EX_stage.return_register(CONSRC) + EX_stage.return_register(TARG)));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 3)	/*LW*/
					{
						memory_offset = EX_stage.return_register(SRC) + EX_stage.return_register(CONTRG);
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 4)	/*SW*/
					{
						memory_offset = EX_stage.return_register(SRC) + EX_stage.return_register(CONTRG); 
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 5)	/*MV*/
					{
						EX_stage.set_register_contents(CONDES, EX_stage.return_register(SRC));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 6)	/*SLT*/
					{
						if(EX_stage.return_register(CONSRC) < EX_stage.return_register(CONTRG))
							EX_stage.set_register_contents(CONDES, 1);
						
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 7)	/*BEQZ*/
					{
						
						if(EX_stage.return_register(CONDES) == 0)
						{
							branch_offset = EX_stage.return_register(SRC);
							branch_flag = true;
							branched_previously = true;
							branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
						}	/*end if*/
						else
						{
							branch_offset = 0;
							branch_flag = false;
							branched_previously = false;
						}	/*end else*/
					}	/*end if*/

					else if(EX_stage.return_ic() == 8)	/*BNEZ*/
					{
						if(EX_stage.return_register(CONDES) != 0)
						{
							branch_offset = EX_stage.return_register(SRC);
							branch_flag = true;
							branched_previously = true;
							branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
						}	/*end if*/
						else
						{
							branch_offset = 0;
							branch_flag = false;
							branched_previously = false;
						}	/*end else*/
					}	/*end if*/

					else if(EX_stage.return_ic() == 9)	/*J*/
					{
						branch_offset = EX_stage.return_register(DEST);
						branch_flag = true;
						branched_previously = true;
						branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
					}	/*end if*/
				}	/*end else*/





				/*****Track Previous Register Block*****/
				/*If instruction is 3,4,5,7, or 8, set source_reg to 0. If 9, set source_reg and destination_reg to 0.*/
				if(pipeline_stall == false)	/*If the pipeline is stalled, the tracking of previous registers will occur in ID*/
				{
					if((EX_stage.return_ic() == 1) || (EX_stage.return_ic() == 2) || (EX_stage.return_ic() == 3) || (EX_stage.return_ic() == 5) || (EX_stage.return_ic() == 6) || (EX_stage.return_ic() == 13))
					{
						if((EX_stage.return_ic() == 3) || (EX_stage.return_ic() == 5))
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end if*/

						else 
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = EX_stage.return_register(SRC);

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end else*/
					}	/*end if*/

					else if((EX_stage.return_ic() == 4) || (EX_stage.return_ic() == 7) || (EX_stage.return_ic() == 8) || (EX_stage.return_ic() == 9))
					{
						if(EX_stage.return_ic() == 9)
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = 0;

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end if*/
						
						else
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end else*/
					}	/*end if*/

					else
					{
						Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
						Rt_of_2nd_previous_instr = Rt_of_previous_instr;
						Rt_of_previous_instr = 0;	/*If NOP, set previous registers to 0*/

						Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
						Rs_of_2nd_previous_instr = Rs_of_previous_instr;
						Rs_of_previous_instr = 0;

						Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
						Rd_of_2nd_previous_instr = Rd_of_previous_instr;
						Rd_of_previous_instr = EX_stage.return_register(DEST);
					}	/*end else*/
				}	/*end if*/
				/*****End Track Previous Register Block*****/
					





				/**************************************************MEM Stage**************************************************/

				mem_success = false;

				if((MEM_stage.return_ic() == 3) || (MEM_stage.return_ic() == 4))	/*LW or SW*/
				{

					if(MEM_stage.return_ic() == 3)	/*LW - Read from MEM*/
					{
						
						for(i = 0; i != no_of_memory_blocks; i++)
						{
							if(data_memory[i].data_address == MEM_stage.set_read_offset(0, memory_offset))	/*Address match*/
							{
								memory_temp = data_memory[i].data;
								mem_success = true;
								break;
							}	/*end if*/
						}	/*end for*/
					
					}	/*end if*/

					else if(MEM_stage.return_ic() == 4)	/*SW - Write to MEM*/
					{
						for(i = 0; i != no_of_memory_blocks; i++)
						{
							if(data_memory[i].data_address == MEM_stage.set_read_offset(0, memory_offset))	/*Address match*/
							{
								data_memory[i].data = MEM_stage.return_register(CONDES);
								mem_success = true;
								break;
							}	/*end if*/
						}	/*end for*/	
					}	/*end if*/

					if(mem_success == false)	/*Memory R/W failed*/
					{
						cout<<"Memory R/W failed. Program will now exit. memory_offset = "<<memory_offset<<endl;
						exit(-9);
					}
				}	/*end if*/

				

				/**************************************************WB Stage**************************************************/

				if((WB_stage.return_ic() == 1) || (WB_stage.return_ic() == 2) || (WB_stage.return_ic() == 3) || (WB_stage.return_ic() == 5) || (WB_stage.return_ic() == 6) || (WB_stage.return_ic() == 13))
				{
					if(WB_stage.return_ic() == 13)	/*EXIT-Print program values*/
					{
						cout<<endl;
						cout<<"Final Register Values:"<<endl;
						cout<<"$0  = "<<register_file.reg_0<<endl;
						cout<<"$T0 = "<<register_file.reg_1<<endl;
						cout<<"$T1 = "<<register_file.reg_2<<endl;
						cout<<"$T2 = "<<register_file.reg_3<<endl;
						cout<<"$T3 = "<<register_file.reg_4<<endl;
						cout<<"$T4 = "<<register_file.reg_5<<endl;
						cout<<"$S0 = "<<register_file.reg_6<<endl;
						cout<<"$S1 = "<<register_file.reg_7<<endl;
						cout<<"$S2 = "<<register_file.reg_8<<endl;
						cout<<"$S3 = "<<register_file.reg_9<<endl;
						cout<<endl;
						cout<<"Number of read-in instructions = "<<no_of_instructions<<endl;
						cout<<"Number of clock cycles taken = "<<current_clock_cycle<<endl;
						exit(0);
					}	/*end if*/

					//if(WB_stage.return_ic() == 3)
						//WB_stage.set_register_contents(CONDES, memory_temp);


					if(WB_stage.return_register(DEST) == 0)
						register_file.reg_0 = 0;

					else if(WB_stage.return_register(DEST) == 1)
						register_file.reg_1 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 2)
						register_file.reg_2 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 3)
						register_file.reg_3 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 4)
						register_file.reg_4 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 5)
						register_file.reg_5 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 6)
						register_file.reg_6 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 7)
						register_file.reg_7 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 8)
						register_file.reg_8 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 9)
						register_file.reg_9 = WB_stage.return_register(CONDES);
				}	/*end if*/
					




				/**************************************************Post-Pipeline Operations**************************************************/
				current_clock_cycle++;

				if(!strcmp(instruction_input_parameters.instruction, "ADD"))	/*This block is necessary for PRINTEX to work correctly*/
					IF_stage.set_previous_instruction(ADD);
				else if(!strcmp(instruction_input_parameters.instruction, "ADDI"))	
					IF_stage.set_previous_instruction(ADDI);
				else if(!strcmp(instruction_input_parameters.instruction, "LW"))	
					IF_stage.set_previous_instruction(LW);
				else if(!strcmp(instruction_input_parameters.instruction, "SW"))	
					IF_stage.set_previous_instruction(SW);
				else if(!strcmp(instruction_input_parameters.instruction, "MV"))	
					IF_stage.set_previous_instruction(MV);
				else if(!strcmp(instruction_input_parameters.instruction, "SLT"))	
					IF_stage.set_previous_instruction(SLT);
				else if(!strcmp(instruction_input_parameters.instruction, "BEQZ"))	
					IF_stage.set_previous_instruction(BEQZ);
				else if(!strcmp(instruction_input_parameters.instruction, "BNEZ"))	
					IF_stage.set_previous_instruction(BNEZ);
				else if(!strcmp(instruction_input_parameters.instruction, "J"))	
					IF_stage.set_previous_instruction(J);
				else	
					IF_stage.set_previous_instruction(NOP);

				ID_stage.set_previous_instruction(ID_stage.return_ic());	
				EX_stage.set_previous_instruction(EX_stage.return_ic());
				MEM_stage.set_previous_instruction(MEM_stage.return_ic());
				WB_stage.set_previous_instruction(WB_stage.return_ic());
				

				if(pipeline_stall == true)
				{
					ID_stage.set_ic(stalled_instruction);	/*Holds the stalled instruction temporarily*/
					destination_reg = Rd_stall;
					source_reg = Rs_stall;
					target_reg = Rt_stall;

					if(pipeline_stall == true && branch_PC != 0)	/*If the pipeline is stalled after a branch, the instruction in ID needs to be cleared*/
					{
						ID_instruction_parameters = instruction_input_parameters;
						ID_stage.set_ic(NOP);
						branch_PC = 0;
					}	/*end if*/

				}	/*end if*/

				else
					ID_instruction_parameters = instruction_input_parameters;


				if(pipeline_stall == false)	/*A stalled instruction will stay in EX for the cycle*/
				{
					WB_stage.set_ic(MEM_stage.return_ic());	/*Setting the stage instruction codes must occur from last stage to earliest stage (unless stalled)*/
					MEM_stage.set_ic(EX_stage.return_ic());
					EX_stage.set_ic(ID_stage.return_ic());
				}	/*end if*/

				else
				{
					WB_stage.set_ic(MEM_stage.return_ic());
					MEM_stage.set_ic(NOP);
				}	/*end if*/
				
	
				if(WB_stage.return_ic() == LW)
					WB_stage.set_registers(MEM_stage.return_register(DEST), MEM_stage.return_register(SRC), MEM_stage.return_register(TARG), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), memory_temp);
				else
					WB_stage.set_registers(MEM_stage.return_register(DEST), MEM_stage.return_register(SRC), MEM_stage.return_register(TARG), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), MEM_stage.return_register(CONDES));
				
				MEM_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
				MEM_stage.set_read_offset(1, memory_offset);


				if(pipeline_stall == false)
					EX_stage.set_registers(destination_reg, source_reg, target_reg, contents_of_src, contents_of_targ, contents_of_dest);
				
			}	/*end while*/















































		/**********************************************************************************************************/
		/*																										  */
		/*																										  */
		/**************************************************TASK 2**************************************************/
		/*																										  */
		/*																										  */
		/**********************************************************************************************************/















































		case 2:	/*With Forwarding*/

			while(IF_count <= no_of_instructions)	/*"Less than or equal to" condition allows for the EXIT instruction to run through the pipeline*/
			{
				/**************************************************IF Stage**************************************************/
				

				if(IF_count < no_of_instructions)	/*This condition is necessary so that the exit command can run through the pipeline*/
				{
					if(pipeline_stall == true)
						pipeline_stall = false;	/*This stage does nothing when the pipeline is stalled*/
				
					else
					{
						/*Tokenizing Instructions: The program will assume that memory addresses are tab- or space-delimited.  
						These instruction and data parameters must be separated using the defined structs.  
						Memory offsets for instructions that utilize memory will be placed in the second instruction argument.*/
					
						if(branch_flag == false)
							tokenize_instructions(IF_count, token, instruction_input, instruction_input_parameters);


						if(branch_flag == true)
						{
							count_temp = IF_count;
							IF_count = 0;
							for(i = 0; atoi(instruction_input_parameters.instr_address) != PC; i++)	/*Fetch the branched instruction*/
								tokenize_instructions(IF_count, token, instruction_input, instruction_input_parameters);

							PC = branch_PC + 4 + branch_offset*4;	/*The formula for the new PC is PC + 4 + offset*4.*/
							if(i == 0)	/*The branched instruction is already in ID*/
								IF_count = count_temp + 1;
						}	/*end if*/

						/*****Meta-Instruction Block*****/
						/*Because of the way the instruction inputs are read in, the names of the instruction input parameters that hold the 
						meta-instruction information do not accurately describe their contents*/

						if(!strcmp(instruction_input_parameters.instr_address, "PRINTPC"))	/*strcmp() returns 0 if strings are equal*/
						{
							cout<<endl;
							cout<<"PC = "<<PC<<endl;
							continue;
						}	/*end if*/

						else if(!strcmp(instruction_input_parameters.instr_address, "PRINTREG")) 
						{
							if(!strcmp(instruction_input_parameters.instruction, "$0"))
								temp = 0;
							else if(!strcmp(instruction_input_parameters.instruction, "$T0"))
								temp = register_file.reg_1;
							else if(!strcmp(instruction_input_parameters.instruction, "$T1"))
								temp = register_file.reg_2;
							else if(!strcmp(instruction_input_parameters.instruction, "$T2"))
								temp = register_file.reg_3;
							else if(!strcmp(instruction_input_parameters.instruction, "$T3"))
								temp = register_file.reg_4;
							else if(!strcmp(instruction_input_parameters.instruction, "$T4"))
								temp = register_file.reg_5;
							else if(!strcmp(instruction_input_parameters.instruction, "$S0"))
								temp = register_file.reg_6;
							else if(!strcmp(instruction_input_parameters.instruction, "$S1"))
								temp = register_file.reg_7;
							else if(!strcmp(instruction_input_parameters.instruction, "$S2"))
								temp = register_file.reg_8;
							else if(!strcmp(instruction_input_parameters.instruction, "$S3"))
								temp = register_file.reg_9;
							else
							{
								cout<<"ERROR: Meta-Instruction not properly formatted.  Program will exit."<<endl;
								exit(-6);
							}	/*end else*/

							cout<<endl;
							cout<<instruction_input_parameters.instruction<<" = "<<temp<<endl;
							continue;
						}	/*end if*/

						else if(!strcmp(instruction_input_parameters.instr_address, "PRINTEX"))
						{
							cout<<endl;
							cout<<"Clock cycle = "<<current_clock_cycle<<endl;
							cout<<"IF: "<<IF_stage.return_previous_instruction()<<endl;
							cout<<"ID: "<<ID_stage.return_previous_instruction()<<endl;
							cout<<"EX: "<<EX_stage.return_previous_instruction()<<endl;
							cout<<"MEM: "<<MEM_stage.return_previous_instruction()<<endl;
							cout<<"WB: "<<WB_stage.return_previous_instruction()<<endl;
							continue;
						}	/*end if*/
						/*****End Meta-Instruction Block*****/

						if(branch_flag == false)
							PC = PC + 4;

					}	/*end else*/
				}	/*end if*/


				


				/**************************************************ID Stage**************************************************/
				

				pipeline_stall = false;
				LW_pipeline_stall = false;
				prev_stall = false;
				second_prev_stall = false;
				third_prev_stall = false;
				data_forwarded = false;
				dest_prev_stall = false;
				dest_sec_prev_stall = false;
				dest_third_prev_stall = false;
				trg_prev_stall = false;
				trg_sec_prev_stall = false;
				trg_third_prev_stall = false;
				src_prev_stall = false;
				src_sec_prev_stall = false;
				src_third_prev_stall = false;
				

				if(branch_flag == true)	/*Check for branch*/
					ID_stage.set_ic(NOP);
					



				/*****Check for Pipeline Stall Block*****/
				else 
				{
					if(source_reg > 0 && source_reg < 10)	/*Register must be between 1 and 9, the valid range for writable registers*/
					{
						if(source_reg == Rd_of_previous_instr)
						{
							prev_stall = true;
							src_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(source_reg == Rd_of_2nd_previous_instr)
						{
							second_prev_stall = true;
							src_sec_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(source_reg == Rd_of_3rd_previous_instr)
						{
							third_prev_stall = true;
							src_third_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
					}	/*end if*/

					if(target_reg > 0 && target_reg < 10)
					{
						if(target_reg == Rd_of_previous_instr)
						{
							prev_stall = true;
							trg_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(target_reg == Rd_of_2nd_previous_instr)
						{
							second_prev_stall = true;
							trg_sec_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(target_reg == Rd_of_3rd_previous_instr)
						{
							third_prev_stall = true;
							trg_third_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
					}	/*end if*/

					if(destination_reg > 0 && destination_reg < 10)
					{
						if(destination_reg == Rd_of_previous_instr)
						{
							if(ID_instruction_parameters.instruction == "MV" || ID_instruction_parameters.instruction == "BNEZ" || ID_instruction_parameters.instruction == "BEQZ")
							{
								prev_stall = true;
								dest_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/
						if(destination_reg == Rd_of_2nd_previous_instr) 
						{
							if(ID_instruction_parameters.instruction == "MV" || ID_instruction_parameters.instruction == "BNEZ" || ID_instruction_parameters.instruction == "BEQZ")
							{
								second_prev_stall = true;
								dest_sec_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/
						if(destination_reg == Rd_of_3rd_previous_instr)
						{
							if(ID_instruction_parameters.instruction == "MV" || ID_instruction_parameters.instruction == "BNEZ" || ID_instruction_parameters.instruction == "BEQZ")
							{
								third_prev_stall = true;
								dest_third_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/								
					}	/*end if*/

					if(EX_stage.return_register(SRC) > 0 && EX_stage.return_register(SRC) < 10)	/*Since source_reg and target_reg are not set to the stalled registers yet,*/
					{																			/*the program must check to see if the registers that caused the stall are still in use by EX.*/
						if(EX_stage.return_register(SRC) == Rd_of_previous_instr)
						{
							prev_stall = true;
							src_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(EX_stage.return_register(SRC) == Rd_of_2nd_previous_instr)
						{
							second_prev_stall = true;
							src_sec_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(EX_stage.return_register(SRC) == Rd_of_3rd_previous_instr)
						{
							third_prev_stall = true;
							src_third_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/					
					}	/*end if*/

					if(EX_stage.return_register(TARG) > 0 && EX_stage.return_register(TARG) < 10)
					{
						if(EX_stage.return_register(TARG) == Rd_of_previous_instr)
						{
							prev_stall = true;
							trg_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(EX_stage.return_register(TARG) == Rd_of_2nd_previous_instr)
						{
							second_prev_stall = true;
							trg_sec_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/
						if(EX_stage.return_register(TARG) == Rd_of_3rd_previous_instr)
						{
							third_prev_stall = true;
							trg_third_prev_stall = true;
							pipeline_stall = true;
						}	/*end if*/		
					}	/*end if*/

					if(EX_stage.return_register(DEST) > 0 && EX_stage.return_register(DEST) < 10)
					{
						if(EX_stage.return_register(DEST) == Rd_of_previous_instr)
						{
							if(EX_stage.return_ic() == MV || EX_stage.return_ic() == BNEZ || EX_stage.return_ic() == BEQZ)
							{
								prev_stall = true;
								dest_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/
						if(EX_stage.return_register(DEST) == Rd_of_2nd_previous_instr) 
						{
							if(EX_stage.return_ic() == MV || EX_stage.return_ic() == BNEZ || EX_stage.return_ic() == BEQZ)
							{
								second_prev_stall = true;
								dest_sec_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/
						if(EX_stage.return_register(DEST) == Rd_of_3rd_previous_instr)
						{
							if(EX_stage.return_ic() == MV || EX_stage.return_ic() == BNEZ || EX_stage.return_ic() == BEQZ)
							{
								third_prev_stall = true;
								dest_third_prev_stall = true;
								pipeline_stall = true;
							}	/*end if*/
						}	/*end if*/								
					}	/*end if*/

				}	/*end else*/
				
				if(pipeline_stalled_previously == true)	/*Necessary to prevent ID_stage from resetting the instruction type*/
				{
					pipeline_stalled_previously = false;
					ID_stage.set_ic(stalled_instruction);
					if(branched_previously == true)
						ID_stage.set_ic(NOP);
					destination_reg = Rd_stall;
					source_reg = Rs_stall;
					target_reg = Rt_stall;
				}	/*end else if*/
				/*****End Check for Pipeline Stall block*****/





				else
				{
					/*****Set Instruction Code Block*****/
					if(!strcmp(ID_instruction_parameters.instruction, "ADD"))
					{
						ID_stage.set_ic(ADD);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "ADDI"))
					{
						ID_stage.set_ic(ADDI);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "LW"))
					{
						ID_stage.set_ic(LW);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "SW"))
					{
						ID_stage.set_ic(SW);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "MV"))
					{
						ID_stage.set_ic(MV);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "SLT"))
					{
						ID_stage.set_ic(SLT);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "BEQZ"))
					{
						ID_stage.set_ic(BEQZ);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "BNEZ"))
					{
						ID_stage.set_ic(BNEZ);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instruction, "J"))
					{
						ID_stage.set_ic(J);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
						branch_PC = PC;
					}	/*end if*/
					else if(!strcmp(ID_instruction_parameters.instr_address, "EXIT"))
					{
						ID_stage.set_ic(EXIT);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end if*/

					else
					{
						ID_stage.set_ic(NOP);
						ID_stage.set_registers(destination_reg, source_reg, target_reg, ID_instruction_parameters, register_file);
					}	/*end else*/
					/*****End Set Instruction Code Block*****/




					if(pipeline_stall == false)	/*Setting PC for stalls and exit conditions*/
					{
						if(pipeline_stalled_previously == false)	/*If the pipeline was not stalled previously, PC must be decremented on this cycle if NOP or EXIT is run*/
							if(ID_stage.return_ic() == NOP && ID_stage.return_ic() == EXIT)
								PC = PC - 4;
					}	/*end if*/
				}	/*end else*/

				if(pipeline_stall == true)
				{
					//pipeline_stalled_previously = true;

					/*If LW is not causing the register conflict AND is not the previous instruction, no need to stall.  Forwarding will suffice.*/
					/*Determine what register (dest, src, or tgt) is causing the stall. The proper stage must forward the contents of the specified register to EX*/
		
					if(prev_stall == true)	/*Register conflict with the previous instruction*/
					{
						if(MEM_stage.return_ic() != LW)	/*The stall is not identified until the previous instruction has left the EX stage*/
						{
							LW_pipeline_stall = false;
							pipeline_stall = false;
							pipeline_stalled_previously = false;
							data_forwarded = true;
							//PC = PC + 4;
						}	/*end if*/
						
						else
							LW_pipeline_stall = true;
					}	/*end if*/

					else if(second_prev_stall == true && LW_pipeline_stall == false)	/*Register conflict with the second previous instruction*/
					{
							pipeline_stall = false;
							pipeline_stalled_previously = false;
							data_forwarded = true;
							//PC = PC + 4;
					}	/*end else if*/

					else if(third_prev_stall == true && LW_pipeline_stall == false)	/*Register conflict with the third previous instruction*/
					{
						pipeline_stall = false;
						pipeline_stalled_previously = false;
						data_forwarded = true;
						//PC = PC + 4;
					}	/*end else*/


					if(pipeline_stall == true)	/*Data can't be forwarded and a stall must occur-only for the LW case*/
					{
						data_forwarded = false;
						stalled_instruction = ID_stage.return_ic();	/*Holds the stalled instruction temporarily*/
						Rd_stall = destination_reg;
						Rs_stall = source_reg;
						Rt_stall = target_reg;

						Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
						Rt_of_2nd_previous_instr = Rt_of_previous_instr;
						Rt_of_previous_instr = 0;

						Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
						Rs_of_2nd_previous_instr = Rs_of_previous_instr;
						Rs_of_previous_instr = 0;

						Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
						Rd_of_2nd_previous_instr = Rd_of_previous_instr;
						Rd_of_previous_instr = 0;
					}	/*end if*/
				}	/*end if*/

				
				if(pipeline_stall == false)
				{
					if(data_forwarded == true)	/*Data needs to be forwarded to EX*/
					{
						if(prev_stall == true)
						{
							if(src_prev_stall == true && trg_prev_stall == false && dest_prev_stall == false)	
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(SRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
								else
									EX_stage.set_registers(EX_stage.return_register(DEST), MEM_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));	/*Need output of EX stage for previous instruction stall*/
							if(trg_prev_stall == true && src_prev_stall == false && dest_prev_stall == false)
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), MEM_stage.return_register(SRC), EX_stage.return_register(CONDES));
								else
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), MEM_stage.return_register(TARG), EX_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
							if(dest_prev_stall == true && src_prev_stall == false && dest_prev_stall == false)	/*This is set for MV, BEQZ, and BNEZ stalls*/
								EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));

							


							if(src_prev_stall == true && trg_prev_stall == true && dest_prev_stall == false)	/*Same register for source and target*/
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), EX_stage.return_register(CONDES));
								else
									EX_stage.set_registers(EX_stage.return_register(DEST), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONSRC), EX_stage.return_register(CONDES));	/*Need output of EX stage for previous instruction stall*/
							
							if(src_prev_stall == true && trg_prev_stall == false && dest_prev_stall == true)	/*Same register for source and destination*/
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(SRC), EX_stage.return_register(CONTRG), MEM_stage.return_register(SRC));
								else
									EX_stage.set_registers(MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));	/*Need output of EX stage for previous instruction stall*/

							if(src_prev_stall == false && trg_prev_stall == true && dest_prev_stall == true)	/*Same register for target and destination*/
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC));
								else
									EX_stage.set_registers(MEM_stage.return_register(TARG), EX_stage.return_register(SRC), MEM_stage.return_register(TARG), EX_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), MEM_stage.return_register(CONTRG));
						
							if(src_prev_stall == false && trg_prev_stall == true && dest_prev_stall == true)	/*Same registers for all three*/
								if(MEM_stage.return_ic() == MV)	/*Special case required for instruction MV, since the forwarded register contents are set in the MEM_stage.source_reg argument*/
									EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC));
								else
									EX_stage.set_registers(MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), MEM_stage.return_register(SRC), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONSRC));	/*Need output of EX stage for previous instruction stall*/

						}	/*end if*/


						if(second_prev_stall == true)
						{

							if(prev_stall == true || third_prev_stall)	/*Data can't be forwarded and a stall must occur*/
							{
								pipeline_stall = true;
								data_forwarded = false;
								stalled_instruction = ID_stage.return_ic();	/*Holds the stalled instruction temporarily*/
								Rd_stall = destination_reg;
								Rs_stall = source_reg;
								Rt_stall = target_reg;

								Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
								Rt_of_2nd_previous_instr = Rt_of_previous_instr;
								Rt_of_previous_instr = 0;

								Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
								Rs_of_2nd_previous_instr = Rs_of_previous_instr;
								Rs_of_previous_instr = 0;

								Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
								Rd_of_2nd_previous_instr = Rd_of_previous_instr;
								Rd_of_previous_instr = 0;
							}	/*end if*/

							else
							{
								if(src_sec_prev_stall == true)
								{
									if(WB_stage.return_ic() == LW)	/*Special case required for instruction LW, since the contents of the memory are not passed immediately to WB when the stage begins its operation (held in memory_temp)*/
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), memory_temp, EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
									else
										EX_stage.set_registers(EX_stage.return_register(DEST), WB_stage.return_register(SRC), EX_stage.return_register(TARG), WB_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));	/*Need output of MEM stage for previous instruction stall*/
								}	/*end if*/
								if(trg_sec_prev_stall == true)
								{
									if(WB_stage.return_ic() == LW)		/*Special case required for instruction LW, since the contents of the memory are not passed immediately to WB when the stage begins its operation (held in memory_temp)*/
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), memory_temp, EX_stage.return_register(CONDES));
									else if(WB_stage.return_ic() == MV)	/*Special case required for instruction MV, since the contents needed are equal to the value of the source register*/
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), WB_stage.return_register(SRC), EX_stage.return_register(CONDES));
									else
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), WB_stage.return_register(TARG), EX_stage.return_register(CONSRC), WB_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
								}	/*end if*/
								if(dest_sec_prev_stall == true)	/*dest_stall is only set for MV, BEQZ, and BNEZ stalls*/
								{
									if(WB_stage.return_ic() == LW)		/*Special case required for instruction LW, since the contents of the memory are not passed immediately to WB when the stage begins its operation (held in memory_temp)*/
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), memory_temp);
									else
										EX_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), WB_stage.return_register(TARG), EX_stage.return_register(CONSRC), WB_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
								}	/*end if*/
							}	/*end else*/
						}	/*end if*/

						if(third_prev_stall == true)	/*With third previous instruction stall, the register contents in EX need to be updated again*/
						{
							/*EX register contents need to be set again because WB has written a new value to the registers*/
							if(second_prev_stall == true || prev_stall == true)	/*Data can't be forwarded and a stall must occur*/
							{
								pipeline_stall = true;
								data_forwarded = false;
								stalled_instruction = ID_stage.return_ic();	/*Holds the stalled instruction temporarily*/
								Rd_stall = destination_reg;
								Rs_stall = source_reg;
								Rt_stall = target_reg;

								Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
								Rt_of_2nd_previous_instr = Rt_of_previous_instr;
								Rt_of_previous_instr = 0;

								Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
								Rs_of_2nd_previous_instr = Rs_of_previous_instr;
								Rs_of_previous_instr = 0;

								Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
								Rd_of_2nd_previous_instr = Rd_of_previous_instr;
								Rd_of_previous_instr = 0;
							}	/*end if*/


							else
							{
								if(src_third_prev_stall == true)
								{
									if(EX_stage.return_register(SRC) == 0)
										EX_stage.set_register_contents(CONSRC, register_file.reg_0);
									else if(EX_stage.return_register(SRC) == 1)
										EX_stage.set_register_contents(CONSRC, register_file.reg_1);
									else if(EX_stage.return_register(SRC) == 2)
										EX_stage.set_register_contents(CONSRC, register_file.reg_2);
									else if(EX_stage.return_register(SRC) == 3)
										EX_stage.set_register_contents(CONSRC, register_file.reg_3);
									else if(EX_stage.return_register(SRC) == 4)
										EX_stage.set_register_contents(CONSRC, register_file.reg_4);
									else if(EX_stage.return_register(SRC) == 5)
										EX_stage.set_register_contents(CONSRC, register_file.reg_5);
									else if(EX_stage.return_register(SRC) == 6)
										EX_stage.set_register_contents(CONSRC, register_file.reg_6);
									else if(EX_stage.return_register(SRC) == 7)
										EX_stage.set_register_contents(CONSRC, register_file.reg_7);
									else if(EX_stage.return_register(SRC) == 8)
										EX_stage.set_register_contents(CONSRC, register_file.reg_8);
									else if(EX_stage.return_register(SRC) == 9)
										EX_stage.set_register_contents(CONSRC, register_file.reg_9);
								}	/*end if*/

									
								if(trg_third_prev_stall == true)
								{
									if(EX_stage.return_register(TARG) == 0)
										EX_stage.set_register_contents(CONTRG, register_file.reg_0);
									else if(EX_stage.return_register(TARG) == 1)
										EX_stage.set_register_contents(CONTRG, register_file.reg_1);
									else if(EX_stage.return_register(TARG) == 2)
										EX_stage.set_register_contents(CONTRG, register_file.reg_2);
									else if(EX_stage.return_register(TARG) == 3)
										EX_stage.set_register_contents(CONTRG, register_file.reg_3);
									else if(EX_stage.return_register(TARG) == 4)
										EX_stage.set_register_contents(CONTRG, register_file.reg_4);
									else if(EX_stage.return_register(TARG) == 5)
										EX_stage.set_register_contents(CONTRG, register_file.reg_5);
									else if(EX_stage.return_register(TARG) == 6)
										EX_stage.set_register_contents(CONTRG, register_file.reg_6);
									else if(EX_stage.return_register(TARG) == 7)
										EX_stage.set_register_contents(CONTRG, register_file.reg_7);
									else if(EX_stage.return_register(TARG) == 8)
										EX_stage.set_register_contents(CONTRG, register_file.reg_8);
									else if(EX_stage.return_register(TARG) == 9)
										EX_stage.set_register_contents(CONTRG, register_file.reg_9);
								}	/*end else if*/

								if(dest_third_prev_stall == true)
								{
									if(EX_stage.return_register(DEST) == 0)
										EX_stage.set_register_contents(CONDES, register_file.reg_0);
									else if(EX_stage.return_register(DEST) == 1)
										EX_stage.set_register_contents(CONDES, register_file.reg_1);
									else if(EX_stage.return_register(DEST) == 2)
										EX_stage.set_register_contents(CONDES, register_file.reg_2);
									else if(EX_stage.return_register(DEST) == 3)
										EX_stage.set_register_contents(CONDES, register_file.reg_3);
									else if(EX_stage.return_register(DEST) == 4)
										EX_stage.set_register_contents(CONDES, register_file.reg_4);
									else if(EX_stage.return_register(DEST) == 5)
										EX_stage.set_register_contents(CONDES, register_file.reg_5);
									else if(EX_stage.return_register(DEST) == 6)
										EX_stage.set_register_contents(CONDES, register_file.reg_6);
									else if(EX_stage.return_register(DEST) == 7)
										EX_stage.set_register_contents(CONDES, register_file.reg_7);
									else if(EX_stage.return_register(DEST) == 8)
										EX_stage.set_register_contents(CONDES, register_file.reg_8);
									else if(EX_stage.return_register(DEST) == 9)
										EX_stage.set_register_contents(CONDES, register_file.reg_9);
								}	/*end else if*/
							}	/*end if*/
						}	/*end else if*/				
					}	/*end if*/

				}	/*end if*/



				if(branch_flag == false)	/*These parameters must be set here instead of the post-pipeline operations because the branch_flag may be reset depending on the operation in EX*/
				{
					if(pipeline_stall == false)
					{
						ID_instruction_parameters = instruction_input_parameters;

						if(data_forwarded == true && branch_PC != 0)		/*If the pipeline is stalled after a branch and data forwarding is required, the instruction in ID needs to be cleared*/
							ID_stage.set_ic(NOP);
					}	/*end if*/

						
					else if(pipeline_stall == true && branch_PC != 0)	/*If the pipeline is stalled after a branch (or data forwarding is required), the instruction in ID needs to be cleared*/
					{
						ID_stage.set_ic(NOP);
					}	/*end if*/
				}	/*end if*/
				else if(branch_offset > 1)	/*If branch_offset = 1, ID will have its correct instruction and should not be cleared.*/
				{
					if(branch_flag == true)	/*If this isn't here (branch_flag == true, the instr_in_branch.txt test file will load a previous instruction(ADD) into ID.*/
						ID_stage.set_ic(NOP);
				}	/*end else if*/

				



				/**************************************************EX Stage**************************************************/
				
			
				if(branch_flag == true)	/*Previous cycle had a branch so the instruction passed to EX from ID is invalid*/
				{
					EX_stage.set_ic(NOP);
					branch_flag = false;
				}	/*end if*/
				
				else if(pipeline_stall == false)
				{
					if(data_forwarded == false)
					{
						/*****Temporary Store Block*****/
						/*The contents of the source, target, and destination registers need to be temporarily stored by the EX stage
						for calculations*/
						if(EX_stage.return_register(SRC) == 0)
							EX_stage.set_register_contents(CONSRC, register_file.reg_0);
						else if(EX_stage.return_register(SRC) == 1)
							EX_stage.set_register_contents(CONSRC, register_file.reg_1);
						else if(EX_stage.return_register(SRC) == 2)
							EX_stage.set_register_contents(CONSRC, register_file.reg_2);
						else if(EX_stage.return_register(SRC) == 3)
							EX_stage.set_register_contents(CONSRC, register_file.reg_3);
						else if(EX_stage.return_register(SRC) == 4)
							EX_stage.set_register_contents(CONSRC, register_file.reg_4);
						else if(EX_stage.return_register(SRC) == 5)
							EX_stage.set_register_contents(CONSRC, register_file.reg_5);
						else if(EX_stage.return_register(SRC) == 6)
							EX_stage.set_register_contents(CONSRC, register_file.reg_6);
						else if(EX_stage.return_register(SRC) == 7)
							EX_stage.set_register_contents(CONSRC, register_file.reg_7);
						else if(EX_stage.return_register(SRC) == 8)
							EX_stage.set_register_contents(CONSRC, register_file.reg_8);
						else if(EX_stage.return_register(SRC) == 9)
							EX_stage.set_register_contents(CONSRC, register_file.reg_9);


						if(EX_stage.return_register(TARG) == 0)
							EX_stage.set_register_contents(CONTRG, register_file.reg_0);
						else if(EX_stage.return_register(TARG) == 1)
							EX_stage.set_register_contents(CONTRG, register_file.reg_1);
						else if(EX_stage.return_register(TARG) == 2)
							EX_stage.set_register_contents(CONTRG, register_file.reg_2);
						else if(EX_stage.return_register(TARG) == 3)
							EX_stage.set_register_contents(CONTRG, register_file.reg_3);
						else if(EX_stage.return_register(TARG) == 4)
							EX_stage.set_register_contents(CONTRG, register_file.reg_4);
						else if(EX_stage.return_register(TARG) == 5)
							EX_stage.set_register_contents(CONTRG, register_file.reg_5);
						else if(EX_stage.return_register(TARG) == 6)
							EX_stage.set_register_contents(CONTRG, register_file.reg_6);
						else if(EX_stage.return_register(TARG) == 7)
							EX_stage.set_register_contents(CONTRG, register_file.reg_7);
						else if(EX_stage.return_register(TARG) == 8)
							EX_stage.set_register_contents(CONTRG, register_file.reg_8);
						else if(EX_stage.return_register(TARG) == 9)
							EX_stage.set_register_contents(CONTRG, register_file.reg_9);


						if(EX_stage.return_register(DEST) == 0)
							EX_stage.set_register_contents(CONDES, register_file.reg_0);
						else if(EX_stage.return_register(DEST) == 1)
							EX_stage.set_register_contents(CONDES, register_file.reg_1);
						else if(EX_stage.return_register(DEST) == 2)
							EX_stage.set_register_contents(CONDES, register_file.reg_2);
						else if(EX_stage.return_register(DEST) == 3)
							EX_stage.set_register_contents(CONDES, register_file.reg_3);
						else if(EX_stage.return_register(DEST) == 4)
							EX_stage.set_register_contents(CONDES, register_file.reg_4);
						else if(EX_stage.return_register(DEST) == 5)
							EX_stage.set_register_contents(CONDES, register_file.reg_5);
						else if(EX_stage.return_register(DEST) == 6)
							EX_stage.set_register_contents(CONDES, register_file.reg_6);
						else if(EX_stage.return_register(DEST) == 7)
							EX_stage.set_register_contents(CONDES, register_file.reg_7);
						else if(EX_stage.return_register(DEST) == 8)
							EX_stage.set_register_contents(CONDES, register_file.reg_8);
						else if(EX_stage.return_register(DEST) == 9)
							EX_stage.set_register_contents(CONDES, register_file.reg_9);
						/*****End Temporary Store Block*****/
					}	/*end if*/





					/*****Instruction Calculation Block*****/
					if(EX_stage.return_ic() ==	1)	/*ADD*/
					{
						EX_stage.set_register_contents(CONDES, (EX_stage.return_register(CONSRC) + EX_stage.return_register(CONTRG)));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 2)	/*ADDI*/
					{
						EX_stage.set_register_contents(CONDES, (EX_stage.return_register(CONSRC) + EX_stage.return_register(TARG)));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 3)	/*LW*/
					{
						memory_offset = EX_stage.return_register(SRC) + EX_stage.return_register(CONTRG);
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 4)	/*SW*/
					{
						memory_offset = EX_stage.return_register(SRC) + EX_stage.return_register(CONTRG); 
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 5)	/*MV*/
					{
						EX_stage.set_register_contents(CONDES, EX_stage.return_register(SRC));
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 6)	/*SLT*/
					{
						if(EX_stage.return_register(CONSRC) < EX_stage.return_register(CONTRG))
							EX_stage.set_register_contents(CONDES, 1);
						
						branch_flag = false;
						branched_previously = false;
					}	/*end if*/

					else if(EX_stage.return_ic() == 7)	/*BEQZ*/
					{
						
						if(EX_stage.return_register(CONDES) == 0)
						{
							branch_offset = EX_stage.return_register(SRC);
							branch_flag = true;
							branched_previously = true;
							branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
						}	/*end if*/
						else
						{
							branch_offset = 0;
							branch_flag = false;
							branched_previously = false;
						}	/*end else*/
					}	/*end if*/

					else if(EX_stage.return_ic() == 8)	/*BNEZ*/
					{
						if(EX_stage.return_register(CONDES) != 0)
						{
							branch_offset = EX_stage.return_register(SRC);
							branch_flag = true;
							branched_previously = true;
							branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
						}	/*end if*/
						else
						{
							branch_offset = 0;
							branch_flag = false;
							branched_previously = false;
						}	/*end else*/
					}	/*end if*/

					else if(EX_stage.return_ic() == 9)	/*J*/
					{
						branch_offset = EX_stage.return_register(DEST);
						branch_flag = true;
						branched_previously = true;
						branch_PC = PC - 12;	/*PC - 12 is the location of the branch/jump instruction*/
					}	/*end if*/
				}	/*end else*/





				/*****Track Previous Register Block*****/
				/*If instruction is 3,4,5,7, or 8, set source_reg to 0. If 9, set source_reg and destination_reg to 0.*/
				if(pipeline_stall == false)	/*If the pipeline is stalled, the tracking of previous registers will occur in ID*/
				{
					if((EX_stage.return_ic() == 1) || (EX_stage.return_ic() == 2) || (EX_stage.return_ic() == 3) || (EX_stage.return_ic() == 5) || (EX_stage.return_ic() == 6) || (EX_stage.return_ic() == 13))
					{
						if((EX_stage.return_ic() == 3) || (EX_stage.return_ic() == 5))
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end if*/

						else 
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = EX_stage.return_register(SRC);

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end else*/
					}	/*end if*/

					else if((EX_stage.return_ic() == 4) || (EX_stage.return_ic() == 7) || (EX_stage.return_ic() == 8) || (EX_stage.return_ic() == 9))
					{
						if(EX_stage.return_ic() == 9)
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = 0;

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end if*/
						
						else
						{
							Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
							Rt_of_2nd_previous_instr = Rt_of_previous_instr;
							Rt_of_previous_instr = EX_stage.return_register(TARG);

							Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
							Rs_of_2nd_previous_instr = Rs_of_previous_instr;
							Rs_of_previous_instr = 0;

							Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
							Rd_of_2nd_previous_instr = Rd_of_previous_instr;
							Rd_of_previous_instr = EX_stage.return_register(DEST);
						}	/*end else*/
					}	/*end if*/

					else
					{
						Rt_of_3rd_previous_instr = Rt_of_2nd_previous_instr;
						Rt_of_2nd_previous_instr = Rt_of_previous_instr;
						Rt_of_previous_instr = 0;	/*If NOP, set previous registers to 0*/

						Rs_of_3rd_previous_instr = Rs_of_2nd_previous_instr;
						Rs_of_2nd_previous_instr = Rs_of_previous_instr;
						Rs_of_previous_instr = 0;

						Rd_of_3rd_previous_instr = Rd_of_2nd_previous_instr;
						Rd_of_2nd_previous_instr = Rd_of_previous_instr;
						Rd_of_previous_instr = EX_stage.return_register(DEST);
					}	/*end else*/
				}	/*end if*/
				/*****End Track Previous Register Block*****/
					





				/**************************************************MEM Stage**************************************************/
		

				mem_success = false;

				if((MEM_stage.return_ic() == 3) || (MEM_stage.return_ic() == 4))	/*LW or SW*/
				{

					if(MEM_stage.return_ic() == 3)	/*LW - Read from MEM*/
					{
						
						for(i = 0; i != no_of_memory_blocks; i++)
						{
							if(data_memory[i].data_address == MEM_stage.set_read_offset(0, memory_offset))	/*Address match*/
							{
								memory_temp = data_memory[i].data;
								mem_success = true;
								break;
							}	/*end if*/
						}	/*end for*/
					
					}	/*end if*/

					else if(MEM_stage.return_ic() == 4)	/*SW - Write to MEM*/
					{
						for(i = 0; i != no_of_memory_blocks; i++)
						{
							if(data_memory[i].data_address == MEM_stage.set_read_offset(0, memory_offset))	/*Address match*/
							{
								data_memory[i].data = MEM_stage.return_register(CONDES);
								mem_success = true;
								break;
							}	/*end if*/
						}	/*end for*/	
					}	/*end if*/

					if(mem_success == false)	/*Memory R/W failed*/
					{
						cout<<"Memory R/W failed. Program will now exit. memory_offset = "<<memory_offset<<endl;
						exit(-9);
					}
				}	/*end if*/

				

				/**************************************************WB Stage**************************************************/
				

				if((WB_stage.return_ic() == 1) || (WB_stage.return_ic() == 2) || (WB_stage.return_ic() == 3) || (WB_stage.return_ic() == 5) || (WB_stage.return_ic() == 6) || (WB_stage.return_ic() == 13))
				{
					if(WB_stage.return_ic() == 13)	/*EXIT-Print program values*/
					{
						cout<<endl;
						cout<<"Final Register Values:"<<endl;
						cout<<"$0  = "<<register_file.reg_0<<endl;
						cout<<"$T0 = "<<register_file.reg_1<<endl;
						cout<<"$T1 = "<<register_file.reg_2<<endl;
						cout<<"$T2 = "<<register_file.reg_3<<endl;
						cout<<"$T3 = "<<register_file.reg_4<<endl;
						cout<<"$T4 = "<<register_file.reg_5<<endl;
						cout<<"$S0 = "<<register_file.reg_6<<endl;
						cout<<"$S1 = "<<register_file.reg_7<<endl;
						cout<<"$S2 = "<<register_file.reg_8<<endl;
						cout<<"$S3 = "<<register_file.reg_9<<endl;
						cout<<endl;
						cout<<"Number of read-in instructions = "<<no_of_instructions<<endl;
						cout<<"Number of clock cycles taken = "<<current_clock_cycle<<endl;
						exit(0);
					}	/*end if*/

				//	if(WB_stage.return_ic() == 3)
				//		WB_stage.set_register_contents(CONDES, memory_temp);


					if(WB_stage.return_register(DEST) == 0)
						register_file.reg_0 = 0;

					else if(WB_stage.return_register(DEST) == 1)
						register_file.reg_1 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 2)
						register_file.reg_2 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 3)
						register_file.reg_3 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 4)
						register_file.reg_4 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 5)
						register_file.reg_5 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 6)
						register_file.reg_6 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 7)
						register_file.reg_7 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 8)
						register_file.reg_8 = WB_stage.return_register(CONDES);

					else if(WB_stage.return_register(DEST) == 9)
						register_file.reg_9 = WB_stage.return_register(CONDES);
				}	/*end if*/
					




				/**************************************************Post-Pipeline Operations**************************************************/
				current_clock_cycle++;


				if(!strcmp(instruction_input_parameters.instruction, "ADD"))	/*This block is necessary for PRINTEX to work correctly*/
					IF_stage.set_previous_instruction(ADD);
				else if(!strcmp(instruction_input_parameters.instruction, "ADDI"))	
					IF_stage.set_previous_instruction(ADDI);
				else if(!strcmp(instruction_input_parameters.instruction, "LW"))	
					IF_stage.set_previous_instruction(LW);
				else if(!strcmp(instruction_input_parameters.instruction, "SW"))	
					IF_stage.set_previous_instruction(SW);
				else if(!strcmp(instruction_input_parameters.instruction, "MV"))	
					IF_stage.set_previous_instruction(MV);
				else if(!strcmp(instruction_input_parameters.instruction, "SLT"))	
					IF_stage.set_previous_instruction(SLT);
				else if(!strcmp(instruction_input_parameters.instruction, "BEQZ"))	
					IF_stage.set_previous_instruction(BEQZ);
				else if(!strcmp(instruction_input_parameters.instruction, "BNEZ"))	
					IF_stage.set_previous_instruction(BNEZ);
				else if(!strcmp(instruction_input_parameters.instruction, "J"))	
					IF_stage.set_previous_instruction(J);
				else	
					IF_stage.set_previous_instruction(NOP);

				ID_stage.set_previous_instruction(ID_stage.return_ic());	
				EX_stage.set_previous_instruction(EX_stage.return_ic());
				MEM_stage.set_previous_instruction(MEM_stage.return_ic());
				WB_stage.set_previous_instruction(WB_stage.return_ic());
				


				if(pipeline_stall == true)
				{
					pipeline_stalled_previously = true;
					ID_stage.set_ic(stalled_instruction);	/*Holds the stalled instruction temporarily*/
					destination_reg = Rd_stall;
					source_reg = Rs_stall;
					target_reg = Rt_stall;

					if(pipeline_stall == true && branch_PC != 0)	/*If the pipeline is stalled after a branch, the instruction in ID needs to be cleared*/
					{
						ID_instruction_parameters = instruction_input_parameters;
						ID_stage.set_ic(NOP);
						branch_PC = 0;
					}	/*end if*/

				}	/*end if*/

				else
				{
					pipeline_stalled_previously = false;
					ID_instruction_parameters = instruction_input_parameters;
				}	/*end else*/

				if(pipeline_stall == false)	/*A stalled instruction will stay in EX for the cycle*/
				{
					WB_stage.set_ic(MEM_stage.return_ic());	/*Setting the stage instruction codes must occur from last stage to earliest stage (unless stalled)*/
					MEM_stage.set_ic(EX_stage.return_ic());
					EX_stage.set_ic(ID_stage.return_ic());
				}	/*end if*/

				else
				{
					WB_stage.set_ic(MEM_stage.return_ic());
					MEM_stage.set_ic(NOP);
				}	/*end if*/
				
				if(WB_stage.return_ic() == LW)
					WB_stage.set_registers(MEM_stage.return_register(DEST), MEM_stage.return_register(SRC), MEM_stage.return_register(TARG), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), memory_temp);
				else
					WB_stage.set_registers(MEM_stage.return_register(DEST), MEM_stage.return_register(SRC), MEM_stage.return_register(TARG), MEM_stage.return_register(CONSRC), MEM_stage.return_register(CONTRG), MEM_stage.return_register(CONDES));
				
				MEM_stage.set_registers(EX_stage.return_register(DEST), EX_stage.return_register(SRC), EX_stage.return_register(TARG), EX_stage.return_register(CONSRC), EX_stage.return_register(CONTRG), EX_stage.return_register(CONDES));
				MEM_stage.set_read_offset(1, memory_offset);

				if(pipeline_stall == false)							
					EX_stage.set_registers(destination_reg, source_reg, target_reg, contents_of_src, contents_of_targ, contents_of_dest);		
						
			}	/*end while*/


		default:
			cout<<"ERROR: Improper value for argument 'mode'.  Mode must be equal to 1 or 2.  Program will exit."<<endl;
			exit(0);

	}	/*end switch*/
}	/* end main*/