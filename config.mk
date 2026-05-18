CC ?= gcc
CFLAGS ?= -MMD -MP -I.. -I.
CFLAGS += -g
#CFLAGS += -Werror 
CFLAGS += -Wall 
CFLAGS += -Wextra 
CFLAGS += -Wno-unused-variable 
CFLAGS += -Wno-unused-value 
CFLAGS += -Wno-unused-but-set-variable

ifneq ($(V), 1)
Q = @
endif