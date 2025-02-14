PROJECT_NAME = FetchISO

JAVAC = javac
JAVA = java
SRC_DIR = main/java
OUT_DIR = build
PACKAGE = com/github/mrr736/fetchiso
LIB_DIR = lib

ifeq ($(OS),Windows_NT)
    PATH_SEPARATOR = ;
else
    PATH_SEPARATOR = :
endif

CLASSPATH = $(LIB_DIR)/json.jar$(PATH_SEPARATOR)$(OUT_DIR)

SRCS = $(shell find $(SRC_DIR)/$(PACKAGE) -name "*.java")\

all: JAR

JAR: compile
	jar cfm $(PROJECT_NAME).jar MANIFEST.MF -C $(OUT_DIR) .
	rm -rf $(OUT_DIR)

compile:
	@mkdir -p $(OUT_DIR)
	$(JAVAC) -d $(OUT_DIR) -cp $(CLASSPATH) $(SRCS)
	unzip -q -o $(LIB_DIR)/json.jar -x "META-INF/*" -d $(OUT_DIR)

run: compile
	$(JAVA) -cp $(CLASSPATH) com.github.mrr736.fetchiso.FetchISOApp

clean:
	rm -rf $(PROJECT_NAME).jar $(OUT_DIR)

# Phony Targets
.PHONY: all compile run clean
