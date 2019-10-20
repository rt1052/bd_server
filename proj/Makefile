BIN_DIR = ./bin
OBJ_DIR = ./obj
USER_DIR=../user
FUNC_DIR=../func

INC_DIR= -I${USER_DIR} \
         -I${FUNC_DIR}

SRC_USER = ${wildcard  ${USER_DIR}/*.c}
SRC_FUNC = ${wildcard  ${FUNC_DIR}/*.c}

OBJ_USER = ${patsubst %.c,${OBJ_DIR}/%.o,$(notdir ${SRC_USER})}
OBJ_FUNC = ${patsubst %.c,${OBJ_DIR}/%.o,$(notdir ${SRC_FUNC})}

TARGET=${BIN_DIR}/bd_server
# CC=gcc
 CC=arm-none-linux-gnueabi-gcc
CCFLAGS=-g -Wall ${INC_DIR} -std=gnu99

${TARGET}: ${OBJ_USER} ${OBJ_FUNC}
	${CC} ${OBJ_USER} ${OBJ_FUNC} -o $@ -lpthread
	@echo "Compile done."

$(OBJ_USER):${OBJ_DIR}/%.o:${USER_DIR}/%.c
	@echo "Compiling $< ==> $@"
	${CC} ${CCFLAGS} -c $< -o $@

$(OBJ_FUNC):${OBJ_DIR}/%.o:${FUNC_DIR}/%.c
	@echo "Compiling $< ==> $@"
	${CC} ${CCFLAGS} -c $< -o $@

clean:
	@rm -f ${OBJ_USER}
	@rm -f ${OBJ_FUNC}
	@echo "Clean object files done."

	@rm -f *~
	@echo "Clean tempreator files done."

	@rm -f ${TARGET}
	@echo "Clean target files done."

	@echo "Clean done."
