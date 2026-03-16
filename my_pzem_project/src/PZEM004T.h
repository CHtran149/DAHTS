#ifndef __PZEM004T_H__
#define __PZEM004T_H__

#include <neorv32.h>
#include <stdint.h>
#include <stdbool.h>

extern uint8_t PZEM_Response[25];
// Hàm khởi tạo UART1 (Baudrate mặc định của PZEM là 9600)
void PZEM_Init(uint32_t baudrate);

// Đọc tất cả dữ liệu (timeout tính bằng mili giây)
void PZEM_ReadAll(uint32_t timeout_ms);

// Getter (lấy dữ liệu đã đọc)
float PZEM_GetVoltage(void);
float PZEM_GetCurrent(void);
float PZEM_GetPower(void);
float PZEM_GetEnergy(void);
float PZEM_GetFrequency(void);
float PZEM_GetPF(void);

#endif
