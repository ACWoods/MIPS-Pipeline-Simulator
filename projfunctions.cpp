#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "projclasses.h"

#define MAXSIZE 256

using namespace std;

void tokenize_instructions(int &IF_count, char *token, char (&instruction_input)[MAXSIZE][MAXSIZE], instruction_struct &instruction_input_parameters)
{
	int i;

	token = strtok(instruction_input[IF_count]," \t,()");
	if(token == NULL)
	{
		printf("ERROR: Tokenizing of input instructions failed.  Program will exit.");
		exit(-4);
	}	/*end if*/

	for(i = 0; token != NULL; i++)
	{
		switch(i)
		{
			case 0:	/*Copies instruction address to struct member instr_address*/
				strcpy(instruction_input_parameters.instr_address, token);

				break;

			case 1:	/*Copies instruction to struct member instruction*/
				token = strtok(NULL, " \t,()");	/*NULL allows strtok to continue scanning from where it previously left off*/
				if(token != NULL)
				{
					strcpy(instruction_input_parameters.instruction, token);
					break;	/*From switch*/
				}	/*end if*/
				else
				{
					strcpy(instruction_input_parameters.instruction, "0");	/*Set the parameter to 0 if it is not used by the instruction*/
					break;	/*From switch*/
					break;	/*From for loop*/
				}	/*end else*/
			
			case 2:	/*Copies instruction argument 1 to struct member instr_arg1*/
				token = strtok(NULL, " \t,()");	/*NULL allows strtok to continue scanning from where it previously left off*/
				if(token != NULL)
				{
					strcpy(instruction_input_parameters.instr_arg1, token);
					break;	/*From switch*/
				}	/*end if*/
				else
				{
					strcpy(instruction_input_parameters.instr_arg1, "0");	/*Set the parameter to 0 if it is not used by the instruction*/
					break;	/*From switch*/
					break;	/*From for loop*/
				}	/*end else*/

			case 3:	/*Copies instruction argument 2 to struct member instr_arg2*/
				token = strtok(NULL, " \t,()");
				if(token != NULL)
				{
					strcpy(instruction_input_parameters.instr_arg2, token);
					break;
				}	/*end if*/
				else
				{
					strcpy(instruction_input_parameters.instr_arg2, "0");	/*Set the parameter to 0 if it is not used by the instruction*/
					break;
					break;
				}	/*end else*/
			
			case 4:	/*Copies instruction argument 3 to struct member instr_arg3*/
				token = strtok(NULL, " \t,()");
				if(token != NULL)
				{
					strcpy(instruction_input_parameters.instr_arg3, token);
					break;
				}	/*end if*/
				else
				{
					strcpy(instruction_input_parameters.instr_arg3, "0");	/*Set the parameter to 0 if it is not used by the instruction*/
					break;
					break;
				}	/*end else*/

			default:	/*Reports an error or breaks out of the loop when all parameters have been copied*/
				token = strtok(NULL, " \t,()");
				if(token != NULL)
				{
					printf("ERROR: An instruction from the input file was not read in properly or is not of the proper format.  Program will exit.");
					exit(-5);
				}	/*end if*/
				else
				{
					break;
					break;
				}	/*end else*/
		}	/*end switch*/
	}	/*end for*/

	IF_count++;	/*Counts how many instructions have been tokenized for transfer through the pipeline.  
					Used in conjunction with "no_of_instructions" variable to prevent IF stage from fetching nonexistent instructions.
					Also, if this block is placed in a loop, it will allow the block to sequentially tokenize each instruction
					in the input file*/
}	/*end tokenize_instructions()*/

void tokenize_memory(int &mem_count, char *token, char (&main_memory_input)[MAXSIZE][MAXSIZE], data_struct &data_memory_parameters)
{
	int i;

	token = strtok(main_memory_input[mem_count]," \t");
	if(token == NULL)
	{
		printf("ERROR: Tokenizing of memory input failed.  Program will exit.");
		exit(-7);
	}	/*end if*/

	for(i = 0; token != NULL; i++)
	{
		switch(i)
		{
			case 0:	/*Copies instruction address to struct member data_address*/
				strcpy(data_memory_parameters.data_address, token);

				break;

			case 1:	/*Copies instruction to struct member data*/
				token = strtok(NULL, " \t");	/*NULL allows strtok to continue scanning from where it previously left off*/
				if(token != NULL)
				{
					strcpy(data_memory_parameters.data, token);
					break;	/*From switch*/
				}	/*end if*/
				else
				{
					strcpy(data_memory_parameters.data, "0");	/*Set the parameter to 0 if data is not read*/
					break;	/*From switch*/
					break;	/*From for loop*/
				}	/*end else*/

			default:	/*Reports an error or breaks out of the loop when all parameters have been copied*/
				token = strtok(NULL, " \t");
				if(token != NULL)
				{
					printf("ERROR: A memory block from the input file was not read in properly or is not of the proper format.  Program will exit.");
					exit(-8);
				}	/*end if*/
				else
				{
					break;
					break;
				}	/*end else*/
		}	/*end switch*/
	}	/*end for*/
}	/*end tokenize_memory()*/