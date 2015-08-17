#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <sstream>

#define INSTRSIZE 10	/*Instruction arguments should not require more than 10 characters to describe*/
#define MAXSIZE 256


struct instruction_struct
{
	char instr_address[INSTRSIZE];
	char instruction[INSTRSIZE];
	char instr_arg1[INSTRSIZE];
	char instr_arg2[INSTRSIZE];
	char instr_arg3[INSTRSIZE];
};	/*end struct*/

struct data_struct
{
	char data_address[INSTRSIZE];
	char data[MAXSIZE];
};	/*end struct*/

struct memory_struct
{
	float data_address;
	float data;
};	/*end struct*/

struct register_struct
{
	float reg_0;	/*$0*/
	float reg_1;	/*$1*/
	float reg_2;	/*$2*/
	float reg_3;	/*$3*/
	float reg_4;	/*$4*/
	float reg_5;	/*$5*/
	float reg_6;	/*$6*/
	float reg_7;	/*$7*/
	float reg_8;	/*$8*/
	float reg_9;	/*$9*/
};	/*end struct*/


class IF_instruction_fetch
{
	public:
		void set_previous_instruction(int code);
		char* return_previous_instruction(void);
	private:
		char previous_instruction[MAXSIZE];
};

class ID_instruction_decode
{
	public:
		int set_ic(int);	/*Can also be used by other stages to set this stage's instruction code*/
		int return_ic(void);
		void set_previous_instruction(int code);
		char* return_previous_instruction(void);
		void set_registers(float &destination_reg, float &source_reg, float &target_reg, instruction_struct instruction_input_parameters, register_struct &reg_file);
	private:
		int instruction_code;
		char previous_instruction[MAXSIZE];
};

class EX_execution
{
	public:
		int set_ic(int);	/*Can also be used by other stages to set this stage's instruction code*/
		int return_ic(void);
		void set_previous_instruction(int code);
		char* return_previous_instruction(void);
		void set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest);
		void set_register_contents(int code, float temp);
		float return_register(int code);
	private:
		int instruction_code;
		char previous_instruction[MAXSIZE];
		float EX_source_reg, EX_target_reg, EX_destination_reg;
		float EX_contents_of_src, EX_contents_of_targ, EX_contents_of_dest;
};

class MEM_memory_access
{
	public:
		int set_ic(int);	/*Can also be used by other stages to set this stage's instruction code*/
		int return_ic(void);
		void set_previous_instruction(int code);
		char* return_previous_instruction(void);
		float set_read_offset(int code, float mem_offset_main);
		void set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest);
		void set_register_contents(int code, float temp);
		float return_register_contents(int code);
		float return_register(int code);
	private:
		int instruction_code;
		char previous_instruction[MAXSIZE];
		float MEM_source_reg, MEM_target_reg, MEM_destination_reg, MEM_offset;
		float MEM_contents_of_src, MEM_contents_of_targ, MEM_contents_of_dest;
};

class WB_write_back
{
	public:
		int set_ic(int);	/*Can also be used by other stages to set this stage's instruction code*/
		int return_ic(void);
		void set_previous_instruction(int code);
		char* return_previous_instruction(void);
		void set_registers(float destination_reg, float source_reg, float target_reg, float contents_of_src, float contents_of_targ, float contents_of_dest);
		void set_register_contents(int code, float temp);
		float return_register_contents(int code);
		float return_register(int code);
	private:
		int instruction_code;
		char previous_instruction[MAXSIZE];
		float WB_source_reg, WB_target_reg, WB_destination_reg;
		float WB_contents_of_src, WB_contents_of_targ, WB_contents_of_dest;
};

