
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




#Ԥ����
DEFINE +=  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 


AR_OPTS = -rc
LD_OPTS = -lpthread 

#�����ֵ�Դ�ļ�·��
MAIN_SOURCE_PATH       :=$(TOP_DIR)/src


#����Դ�ļ��б�
MAIN_SRCS        :=$(wildcard $(MAIN_SOURCE_PATH)/*.cpp)


#���ɵ�OBJ�б�
MAIN_OBJS        :=$(patsubst $(MAIN_SOURCE_PATH)/%.cpp,$(OBJ_DIR)/%.o,$(MAIN_SRCS))


#���е�Ŀ�����
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



#������Ŀ���ļ�
$(OBJ_DIR)/%.o:$(MAIN_SOURCE_PATH)/%.cpp
	$(CC) $(DEFINE) $(INC_DIR) -c $(CC_OPTS) $< -o $@


.PHONY:commit
commit:
	git add .
	git commit -a
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(PROG) $(PROG)
	
