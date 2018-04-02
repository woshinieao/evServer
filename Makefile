
TOP_DIR =.
BIN_DIR =./bin
OBJ_OUT_DIR=$(TOP_DIR)
PROG = libevServer.a

 
CC=g++
AR=ar
LD=g++


CC_OPTS=-Wall -fPIC

CONFIG=Debug
ifeq ($(CONFIG), Debug)
CC_OPTS+=-g
DEFINE=-DDEBUG
OBJ_DIR =$(OBJ_OUT_DIR)/Debug
else
CC_OPTS+=-O3 -fno-strict-aliasing
DEFINE=
OBJ_DIR =$(OBJ_OUT_DIR)/release
endif




#预定义
DEFINE +=  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 


AR_OPTS = -rc
LD_OPTS = -lpthread 

#各部分的源文件路径
MAIN_SOURCE_PATH       :=$(TOP_DIR)/src


#各个源文件列表
MAIN_SRCS        :=$(wildcard $(MAIN_SOURCE_PATH)/*.cpp)


#生成的OBJ列表
MAIN_OBJS        :=$(patsubst $(MAIN_SOURCE_PATH)/%.cpp,$(OBJ_DIR)/%.o,$(MAIN_SRCS))


#所有的目标对象
ALL_OBJS = $(MAIN_OBJS)

SYSTEM_SERVER_INC= -I$(TOP_DIR)/inc 

INC_DIR =  $(SYSTEM_SERVER_INC)

LIB_DIR =-Wl,-rpath, -L./lib/linux -L/usr/lib/x86_64-linux-gnu/ -L./lib 
 
LIBS +=  -lpthread   -levent $(LIB_DIR)  
#-lptz

target:$(PROG) 
	@echo "+----------------------------------------------------+"
	@echo "+       building server program                      +"
	@echo "+----------------------------------------------------+"
	@date
	@echo ""
$(PROG):$(ALL_OBJS) 
#	$(CC) -o $@ $(ALL_OBJS) $(LIBS)  

	$(AR) -r $(PROG) $(ALL_OBJS)
#	mv $(PROG) $(BIN_DIR) -f



#主程序目标文件
$(OBJ_DIR)/%.o:$(MAIN_SOURCE_PATH)/%.cpp
	$(CC) $(DEFINE) $(INC_DIR) -c $(CC_OPTS) $< -o $@


.PHONY:commit
commit:
	git add .
	git commit -a
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(PROG) $(PROG)
	
