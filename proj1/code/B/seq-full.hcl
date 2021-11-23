#/* $begin seq-all-hcl */
####################################################################
#  HCL Description of Control for Single Cycle Y86 Processor SEQ   #
#  Copyright (C) Randal E. Bryant, David R. O'Hallaron, 2010       #
####################################################################

## Your task is to implement the iaddl instruction
## The file contains a declaration of the icodes
## for iaddl (IIADDL) .
## Your job is to add the rest of the logic to make it work

# 519021910095 QianyunGuo
# 519021910575 MuchenPan
#  Firstly, define ILEAVE as symbolic representation of leave instruction codes.
#  Add IIADDL and ILEAVE in the choice region instr\_valid to make them valid. 
#  Add IIADDL in the choice region of need\_regids since iaddl operation involves one register. The leave instruction doesn’t need it.
#  Add IIADDL in the choice region of need\_valC since iaddl operation need constant parameter. The leave instruction doesn’t need it.
#  When icode is ILEAVE, set the srcA as REBP. For iaddl, there is no need to set because the first operand of iaddl is not a register.
#  When icode is IIADDL, set the srcB as rB. When icode is ILEAVE, set the srcB as REBP.
#  When icode is IIADDL, set the dstE as rB. When icode is ILEAVE set the dstE as RESP. Because iaddl adds the immediate number into the destination register and leave writes value into register \%esp.
#  When icode is ILEAVE, set the dstM as REBP because we also need to update the value in register \%ebp. For IIADDL, there is no need to set.
#  When icode is IIADDL, set aluA as valC. When icode is ILEAVE, set aluA as 4. It’s the first operand of ALU.
#  When icode is IIADDL, set aluB as valB. When icode is ILEAVE, set aluB as valA. It’s the second operand of ALU.
#  When icode is IIADDL, alufun will be ALUADD since the operation is “adding”.
#  Add IIADDL in the choice region of set\_cc since iaddl operation involves ALU operation which will set flags. For leave, we don’t need it.
#  When icode is ILEAVE, set the control signal mem\_read. Instruction iaddl doesn’t need access to memory.
#  When icode is ILEAVE, set mem\_addr as valA. For iaddl, we don’t need it.

####################################################################
#    C Include's.  Don't alter these                               #
####################################################################

quote '#include <stdio.h>'
quote '#include "isa.h"'
quote '#include "sim.h"'
quote 'int sim_main(int argc, char *argv[]);'
quote 'int gen_pc(){return 0;}'
quote 'int main(int argc, char *argv[])'
quote '  {plusmode=0;return sim_main(argc,argv);}'

####################################################################
#    Declarations.  Do not change/remove/delete any of these       #
####################################################################

##### Symbolic representation of Y86 Instruction Codes #############
intsig INOP 	'I_NOP'
intsig IHALT	'I_HALT'
intsig IRRMOVL	'I_RRMOVL'
intsig IIRMOVL	'I_IRMOVL'
intsig IRMMOVL	'I_RMMOVL'
intsig IMRMOVL	'I_MRMOVL'
intsig IOPL	'I_ALU'
intsig IJXX	'I_JMP'
intsig ICALL	'I_CALL'
intsig IRET	'I_RET'
intsig IPUSHL	'I_PUSHL'
intsig IPOPL	'I_POPL'
# Instruction code for iaddl instruction
intsig IIADDL	'I_IADDL'

# Instruction code for leave instruction
intsig ILEAVE   'I_LEAVE'

##### Symbolic represenations of Y86 function codes                  #####
intsig FNONE    'F_NONE'        # Default function code

##### Symbolic representation of Y86 Registers referenced explicitly #####
intsig RESP     'REG_ESP'    	# Stack Pointer
intsig REBP     'REG_EBP'    	# Frame Pointer
intsig RNONE    'REG_NONE'   	# Special value indicating "no register"

##### ALU Functions referenced explicitly                            #####
intsig ALUADD	'A_ADD'		# ALU should add its arguments

##### Possible instruction status values                             #####
intsig SAOK	'STAT_AOK'		# Normal execution
intsig SADR	'STAT_ADR'	# Invalid memory address
intsig SINS	'STAT_INS'	# Invalid instruction
intsig SHLT	'STAT_HLT'	# Halt instruction encountered

##### Signals that can be referenced by control logic ####################

##### Fetch stage inputs		#####
intsig pc 'pc'				# Program counter
##### Fetch stage computations		#####
intsig imem_icode 'imem_icode'		# icode field from instruction memory
intsig imem_ifun  'imem_ifun' 		# ifun field from instruction memory
intsig icode	  'icode'		# Instruction control code
intsig ifun	  'ifun'		# Instruction function
intsig rA	  'ra'			# rA field from instruction
intsig rB	  'rb'			# rB field from instruction
intsig valC	  'valc'		# Constant from instruction
intsig valP	  'valp'		# Address of following instruction
boolsig imem_error 'imem_error'		# Error signal from instruction memory
boolsig instr_valid 'instr_valid'	# Is fetched instruction valid?

##### Decode stage computations		#####
intsig valA	'vala'			# Value from register A port
intsig valB	'valb'			# Value from register B port

##### Execute stage computations	#####
intsig valE	'vale'			# Value computed by ALU
boolsig Cnd	'cond'			# Branch test

##### Memory stage computations		#####
intsig valM	'valm'			# Value read from memory
boolsig dmem_error 'dmem_error'		# Error signal from data memory


####################################################################
#    Control Signal Definitions.                                   #
####################################################################

################ Fetch Stage     ###################################

# Determine instruction code
int icode = [
	imem_error: INOP;
	1: imem_icode;		# Default: get from instruction memory
];

# Determine instruction function
int ifun = [
	imem_error: FNONE;
	1: imem_ifun;		# Default: get from instruction memory
];

bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVL, IIRMOVL, IRMMOVL, IMRMOVL,
	       IOPL, IIADDL,IJXX, ICALL, IRET, IPUSHL, IPOPL, ILEAVE };

# Does fetched instruction require a regid byte?
bool need_regids =
	icode in { IRRMOVL, IOPL, IIADDL, IPUSHL, IPOPL, 
		     IIRMOVL, IRMMOVL, IMRMOVL };

# Does fetched instruction require a constant word?
bool need_valC =
	icode in { IIRMOVL, IIADDL, IRMMOVL, IMRMOVL, IJXX, ICALL };


################ Decode Stage    ###################################

## What register should be used as the A source?
int srcA = [
	icode in { IRRMOVL, IRMMOVL, IOPL, IPUSHL  } : rA;
	icode in { IPOPL, IRET } : RESP;
        icode in { ILEAVE } : REBP;
	1 : RNONE; # Don't need register
];

## What register should be used as the B source?
int srcB = [
	icode in { IOPL, IIADDL, IRMMOVL, IMRMOVL  } : rB;
	icode in { IPUSHL, IPOPL, ICALL, IRET } : RESP;
        icode in { ILEAVE } : REBP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
int dstE = [
	icode in { IRRMOVL } && Cnd : rB;
	icode in { IIRMOVL, IOPL,IIADDL} : rB;
	icode in { IPUSHL, IPOPL, ICALL, IRET, ILEAVE } : RESP;
	1 : RNONE;  # Don't write any register
];

## What register should be used as the M destination?
int dstM = [
	icode in { IMRMOVL, IPOPL } : rA;
        icode in { ILEAVE } : REBP;
	1 : RNONE;  # Don't write any register
];

################ Execute Stage   ###################################

## Select input A to ALU
int aluA = [
	icode in { IRRMOVL, IOPL } : valA;
	icode in { IIRMOVL, IRMMOVL, IMRMOVL, IIADDL } : valC;
	icode in { ICALL, IPUSHL } : -4;
	icode in { IRET, IPOPL, ILEAVE } : 4;
	# Other instructions don't need ALU
];

## Select input B to ALU
int aluB = [
	icode in { IRMMOVL, IMRMOVL, IOPL, IIADDL, ICALL, 
		      IPUSHL, IRET, IPOPL } : valB;
	icode in { IRRMOVL, IIRMOVL } : 0;
        icode in { ILEAVE } : valA;
	# Other instructions don't need ALU
];

## Set the ALU function
int alufun = [
	icode == IOPL : ifun;
        icode == IIADDL : ALUADD;
	1 : ALUADD;
];

## Should the condition codes be updated?
bool set_cc = icode in { IOPL,IIADDL };

################ Memory Stage    ###################################

## Set read control signal
bool mem_read = icode in { IMRMOVL, IPOPL, IRET, ILEAVE };

## Set write control signal
bool mem_write = icode in { IRMMOVL, IPUSHL, ICALL };

## Select memory address
int mem_addr = [
	icode in { IRMMOVL, IPUSHL, ICALL, IMRMOVL } : valE;
	icode in { IPOPL, IRET, ILEAVE } : valA;
	# Other instructions don't need address
];

## Select memory input data
int mem_data = [
	# Value from register
	icode in { IRMMOVL, IPUSHL } : valA;
	# Return PC
	icode == ICALL : valP;
	# Default: Don't write anything
];

## Determine instruction status
int Stat = [
	imem_error || dmem_error : SADR;
	!instr_valid: SINS;
	icode == IHALT : SHLT;
	1 : SAOK;
];

################ Program Counter Update ############################

## What address should instruction be fetched at

int new_pc = [
	# Call.  Use instruction constant
	icode == ICALL : valC;
	# Taken branch.  Use instruction constant
	icode == IJXX && Cnd : valC;
	# Completion of RET instruction.  Use value from stack
	icode == IRET : valM;
	# Default: Use incremented PC
	1 : valP;
];
#/* $end seq-all-hcl */
