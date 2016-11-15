
RM := rm -f
RMDIR := rmdir

OUTDIR ?= ./out
CODE_DIR := ../../../code
CONFIG_DIR := ../../config

# Project source files must go here
C_SRCS := ./src/ICC_Test_Types.c 

OBJS := $(OUTDIR)/ICC_Test_Types.o

DEFS := $(OUTDIR)/ICC_Test_Types.d

INC_DIRS := -I"$(CODE_DIR)/linux/inc" \
			-I"$(CODE_DIR)/linux/os" \
			-I"$(CONFIG_DIR)/include" \
			-I"./src"

EXECUTABLE := $(OUTDIR)/ICC_Relocate_Test

# All Target
all: $(EXECUTABLE)

$(OUTDIR):
	@echo 'Making directory: $@'
	mkdir -p $(OUTDIR)

# Each subdirectory must supply rules for building sources it contributes
$(OUTDIR)/%.o: $(C_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc $(INC_DIRS) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" 
	@echo 'Finished building: $<'
	@echo ' '

# Tool invocations
$(EXECUTABLE): $(OUTDIR) $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "$(EXECUTABLE)" $(OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS) $(DEFS) $(EXECUTABLE)
	-$(RMDIR) $(OUTDIR)
	-@echo ' '

.PHONY: all clean
