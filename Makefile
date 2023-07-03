.PHONY: server runServer

OUT_DIR = target

runServer: $(OUT_DIR)/server/a.out
	./$(OUT_DIR)/server/a.out

	
$(OUT_DIR)/server/a.out: src/server.c
	mkdir -p $(OUT_DIR)/server
	gcc src/server.c -o $@