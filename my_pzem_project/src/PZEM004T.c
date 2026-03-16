#include "PZEM004T.h"
#include <neorv32.h> // Thay vì gptmr.h, ta dùng thư viện core NEORV32

// Gói lệnh đọc 10 thanh ghi đầu tiên (Địa chỉ 0x01, Function 0x04)
static uint8_t PZEM_Request[8] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x70, 0x0D};
uint8_t PZEM_Response[25];

static float voltage = 0.0f;
static float current = 0.0f;
static float power   = 0.0f;
static float energy  = 0.0f;
static float freq    = 0.0f;
static float pf      = 0.0f;

void PZEM_Init(uint32_t baudrate) {
    // 1. Khởi tạo UART1
    neorv32_uart1_setup(baudrate, 0);
    // Không cần cấu hình Timer ở đây nữa
}

// CRC16 Modbus
static uint16_t Modbus_CRC16(uint8_t *buf, uint8_t len){
    uint16_t crc = 0xFFFF;
    for(uint8_t pos=0; pos<len; pos++){
        crc ^= (uint16_t)buf[pos];
        for(uint8_t i=0; i<8; i++){
            if(crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void PZEM_ReadAll(uint32_t timeout_ms) {
    // 1. Gửi lệnh qua UART1
    for (int i = 0; i < 8; i++) {
        neorv32_uart1_putc(PZEM_Request[i]);
    }

    // 2. Nhận phản hồi với Timeout sử dụng delay phần mềm
    uint8_t count = 0;
    uint32_t elapsed_ms = 0;

    while (count < 25 && elapsed_ms < timeout_ms) {
        if (neorv32_uart1_char_received()) {
            PZEM_Response[count++] = (uint8_t)neorv32_uart1_getc();
        } else {
            // Chờ 1ms nếu chưa nhận được byte
            neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 1);
            elapsed_ms++;
        }
    }
    
    // Nếu không nhận đủ dữ liệu
    if (count < 25) return;

    // 3. Kiểm tra CRC
    uint16_t crc_calc = Modbus_CRC16(PZEM_Response, 23);
    uint16_t crc_recv = (PZEM_Response[24] << 8) | PZEM_Response[23];
    if(crc_calc != crc_recv) return;

    // 4. Giải mã dữ liệu (giữ nguyên)
    voltage = ((uint16_t)PZEM_Response[3] << 8 | PZEM_Response[4]) / 10.0f;

    uint32_t i_current = ((uint32_t)PZEM_Response[7] << 24) | ((uint32_t)PZEM_Response[8] << 16) | 
                         ((uint32_t)PZEM_Response[5] << 8)  | PZEM_Response[6];
    current = i_current / 1000.0f;

    uint32_t i_power = ((uint32_t)PZEM_Response[11] << 24) | ((uint32_t)PZEM_Response[12] << 16) | 
                       ((uint32_t)PZEM_Response[9] << 8)  | PZEM_Response[10];
    power = i_power / 10.0f;

    uint32_t i_energy = ((uint32_t)PZEM_Response[15] << 24) | ((uint32_t)PZEM_Response[16] << 16) | 
                        ((uint32_t)PZEM_Response[13] << 8)  | PZEM_Response[14];
    energy = (float)i_energy;

    freq = ((uint16_t)PZEM_Response[17] << 8 | PZEM_Response[18]) / 10.0f;
    pf   = ((uint16_t)PZEM_Response[19] << 8 | PZEM_Response[20]) / 100.0f;
}

// ... các hàm Get còn lại giữ nguyên ...

float PZEM_GetVoltage(void)   { return voltage; }
float PZEM_GetCurrent(void)   { return current; }
float PZEM_GetPower(void)     { return power;   }
float PZEM_GetEnergy(void)    { return energy;  }
float PZEM_GetFrequency(void) { return freq;    }
float PZEM_GetPF(void)        { return pf;      }



