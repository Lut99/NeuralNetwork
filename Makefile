# MAKEFILE for the NEURAL NETWORK
#   by Lut99

##### CONTANTS #####

# Compilers & their arguments
GCC=gcc
GCC_ARGS=-std=c11 -O2 -Wall -Wextra
GCC_LINK_ARGS=
NVCC=nvcc
NVCC_ARGS=-O2 --gpu-architecture=compute_75 --gpu-code=sm_75

# Extra libraries that need to be linked against
EXT_LIBS=-lm

# Directories
SRC=src
LIB=$(SRC)/lib
BIN=bin
OBJ=$(BIN)/obj
TST=tests
TST_BIN=$(BIN)/tests

INCLUDES=-I$(LIB)/include



##### MAKE FLAGS #####

# If run with "DEBUG=1", adds debug symbols to the compiler arguments
ifdef DEBUG
GCC_ARGS+=-g
NVCC_ARGS+=-g
endif

# A bit more powerful form of DEBUG, usable with profile tools
ifdef PROFILE
GCC_ARGS+=-pg
endif

# If given, sets the PLOT flag, which lets the framework output a list of costs per iterations
ifdef PLOT
GCC_ARGS+=-DPLOT
NVCC_ARGS+=-DPLOT
endif

# If given, sets the BENCHMARK symbol while compiling, letting the network only print timing information
ifdef BENCHMARK
GCC_ARGS+=-DBENCHMARK
NVCC_ARGS+=-DBENCHMARK
endif

# Used to add special compiler flags depending on which NeuralNetwork version is run.
# The version itself is selected by giving the appropriate suffix for one of the files in lib/NeuralNetwork
ifdef BACKEND
ifneq ($(BACKEND), sequential)
ifneq (,$(findstring cuda,$(shell echo $(BACKEND) | tr A-Z a-z)))
GCC_LINK_ARGS+= -L/opt/cuda/lib64
EXT_LIBS += -lcuda -lcudart
else
GCC_ARGS+=-fopenmp
endif
endif
else
BACKEND=sequential
endif

# Prepend the normal GCC args to the link args
GCC_LINK_ARGS:=$(GCC_ARGS) $(GCC_LINK_ARGS)



##### ORGANISATIONAL RULES #####

# Define the phonies and which is our default
.PHONY: default dirs digits testdata debug_cuda plot all
default: digits

# Rule to compile everything
all: digits testdata debug_cuda plot

# Cleans it all up, including any result files.
clean:
	rm -f $(BIN)/*.out
	rm -f $(TST_BIN)/*.out
	rm -f $(OBJ)/*.o
	rm -f ./nn_costs.dat
	rm -f ./nn_costs.png

# Rules to create directories
$(BIN):
	mkdir -p $@
$(OBJ):
	mkdir -p $@
$(TST_BIN):
	mkdir -p $@
dirs: $(BIN) $(OBJ) $(TST_BIN)



##### GENERAL COMPILATION RULES #####

# Compiles everything directly under lib/
$(OBJ)/%.o: $(LIB)/%.c | dirs
	$(GCC) $(GCC_ARGS) $(INCLUDES) -o $@ -c $<

# Compiles any non-CUDA backend
$(OBJ)/NeuralNetwork_%.o: $(LIB)/NeuralNetwork/NeuralNetwork_%.c | dirs
	$(GCC) $(GCC_ARGS) $(INCLUDES) -o $@ -c $<
# Compiles CUDA backends
$(OBJ)/NeuralNetwork_%.o: $(LIB)/NeuralNetwork/NeuralNetwork_%.cu | dirs
	$(NVCC) $(NVCC_ARGS) $(INCLUDES) -o $@ -c $<



##### MAIN RULES #####

# Compile the digits fmain ile
$(OBJ)/Digits.o: $(SRC)/Digits.c | dirs
	$(GCC) $(GCC_ARGS) $(INCLUDES) -o $@ -c $<
# Compile the testdata main file
$(OBJ)/TestData.o: $(SRC)/TestData.c | dirs
	$(GCC) $(GCC_ARGS) $(INCLUDES) -o $@ -c $<

# Links the digits executable
$(BIN)/digits.out: $(OBJ)/Digits.o $(OBJ)/CSV.o $(OBJ)/NeuralNetwork.o $(OBJ)/NeuralNetwork_${BACKEND}.o | dirs
	$(GCC) $(GCC_LINK_ARGS) $(INCLUDES) -o $@ $^ $(EXT_LIBS)
digits: $(BIN)/digits.out

# Links the testdata executable
$(BIN)/testdata.out: $(OBJ)/TestData.o $(OBJ)/CSV.o $(OBJ)/NeuralNetwork.o $(OBJ)/NeuralNetwork_${BACKEND}.o | dirs
	$(GCC) $(GCC_LINK_ARGS) $(INCLUDES) -o $@ $^ $(EXT_LIBS)
testdata: $(BIN)/testdata.out



##### TEST RULES #####

# Compiles the debug_cuda file
$(OBJ)/debug_cuda.o: $(TST)/debug_cuda.c | dirs
	$(GCC) $(GCC_ARGS) $(INCLUDES) -o $@ -c $^ $(EXT_LIBS)

# Links the debug_cuda executable
$(TST_BIN)/debug_cuda.out: $(OBJ)/debug_cuda.o $(OBJ)/NeuralNetwork.o $(OBJ)/NeuralNetwork_CUDA_GPU1.o | dirs
	$(NVCC) $(NVCC_ARGS) $(INCLUDES) -o $@ $^ $(EXT_LIBS)
debug_cuda: $(TST_BIN)/debug_cuda.out



##### MISCELLANEOUS #####

# Can be used after a run with PLOT=1 is done to show a plot with the learning rate of the neural network over the iterations.
plot: nn_costs.dat
	gnuplot -e "set terminal png size 600,400; set output 'nn_costs.png'; set yrange[0:]; plot \"nn_costs.dat\""
