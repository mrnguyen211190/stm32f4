
CFLAGS += -DHAL_ETH_MODULE_ENABLED
C_SRCS += $(ROOT_DIR)/eth/eth.c
C_SRCS += $(ROOT_DIR)/eth/eth_it.c
C_SRCS += $(ROOT_DIR)/eth/eth_msp.c
C_SRCS += $(ROOT_DIR)/eth/mdio.c
C_SRCS += $(HAL_DIR)/Src/stm32f4xx_hal_eth.c

IP_DIR = $(ROOT_DIR)/eth/myip
include $(IP_DIR)/myip.mk
