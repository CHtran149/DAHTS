# Application name
APP_NAME = pzem_monitor

# Override the default CPU ISA
MARCH = rv32i_zicsr_zifencei

# Đường dẫn include: Tìm .h trong src và neorv32_lib/include
USER_FLAGS += -I src -I neorv32_lib/include

# --------------------------------------------------------------------------
# CHỈ ĐỊNH CÁC FILE NGUỒN (APP_SRC)
# Không dùng wildcard để tránh kéo theo các thư viện thừa gây tràn ROM
# --------------------------------------------------------------------------
# 1. File mã nguồn chính của bạn trong thư mục src/
APP_SRC = src/main.c src/PZEM004T.c src/OLED_LCD_SSD1306.c src/fonts.c

# 2. Các thư viện NEORV32 tối thiểu bắt buộc phải có
APP_SRC += neorv32_lib/source/neorv32_uart.c
APP_SRC += neorv32_lib/source/neorv32_twi.c
APP_SRC += neorv32_lib/source/neorv32_aux.c
APP_SRC += neorv32_lib/source/neorv32_rte.c
APP_SRC += neorv32_lib/source/neorv32_cpu.c
APP_SRC += neorv32_lib/source/neorv32_gpio.c
# --------------------------------------------------------------------------

# Tối ưu hóa dung lượng tối đa cho Tang Nano 9K
EFFORT = -Os

# Thêm các cờ tối ưu hóa Linker
USER_FLAGS += -ffunction-sections -fdata-sections
USER_FLAGS += -Wl,--gc-sections
USER_FLAGS += -mrelax

# Đặt lại dung lượng chuẩn để kiểm soát lỗi khi Make
USER_FLAGS += -Wl,--defsym,__neorv32_ram_size=16k
USER_FLAGS += -Wl,--defsym,__neorv32_rom_size=32k

# Đường dẫn NEORV32 root
NEORV32_HOME ?= ../..

# Include the main NEORV32 makefile
include $(NEORV32_HOME)/sw/common/common.mk