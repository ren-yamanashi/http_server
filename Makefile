.PHONY: runServer libServer

OUT_DIR = target
SRC_DIR = lib
OBJS = $(addprefix $(OUT_DIR)/lib/, helper.o io.o request.o response.o server.o lib.o)
LIB = $(OUT_DIR)/lib/libServer.dylib
NODE_FFI = node_ffi_sample
default:
	make libServer

# runServer
runServer: $(OUT_DIR)/main/a.out
	export LD_LIBRARY_PATH=${OUT_DIR}/lib:$$LD_LIBRARY_PATH; \
	./$(OUT_DIR)/main/a.out
	
$(OUT_DIR)/main/a.out: $(SRC_DIR)/main.c $(LIB)
	mkdir -p $(OUT_DIR)/main
	gcc -L$(OUT_DIR)/lib -lServer -o $@ $(SRC_DIR)/main.c

# libServer
libServer: $(LIB)

$(LIB): $(OBJS)
	gcc -shared -o $@ $^

$(OUT_DIR)/lib/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OUT_DIR)/lib
	gcc -c $< -o $@

# runNodeFFI
runNodeFFI:
	cd node_ffi && npm run dev
