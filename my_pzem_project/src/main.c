#include <neorv32.h>
#include <neorv32_twi.h>
#include <neorv32_uart.h>
#include "PZEM004T.h"
#include "OLED_LCD_SSD1306.h"
#include "fonts.h"

#define OUT_PIN_BUZZER 0
#define IN_PIN_BUTTON 0

// --- KHAI BÁO PROTOTYPE (Để trình biên dịch biết trước) ---
void uart0_puts(const char *s);
void uart0_putu(uint32_t n);

// Biến cờ cảnh báo (dùng volatile vì được thay đổi trong hàm ngắt)
volatile uint8_t is_muted = 0;

// --- Hàm xử lý ngắt (ISR) ---
void my_gpio_irq_handler(void) {
    // 1. Kiểm tra xem có phải ngắt từ chân nút nhấn không
    if (neorv32_gpio_irq_get() & (1 << IN_PIN_BUTTON)) {
        uart0_puts("Da nhan nut");
        is_muted = !is_muted; // Nếu đang 0 thành 1, đang 1 thành 0
        if (is_muted) {
            neorv32_gpio_pin_set(OUT_PIN_BUZZER, 1); 
        }
        
        // 2. Xoá cờ ngắt
        neorv32_gpio_irq_clr(1 << IN_PIN_BUTTON);
    }
}

// 1. Hàm in chuỗi thô (Thay thế printf cho chuỗi)
void uart0_puts(const char *s) {
    while (*s) neorv32_uart0_putc(*s++);
}

// 2. Hàm in số nguyên thô (Thay thế printf cho %d, %u)
void uart0_putu(uint32_t n) {
    char buf[10];
    int i = 0;
    if (n == 0) { neorv32_uart0_putc('0'); return; }
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (--i >= 0) neorv32_uart0_putc(buf[i]);
}

// 3. Hàm in float "siêu nhẹ" (Không dùng printf)
void neorv32_uart_print_float(float value, int precision) {
    if (value < 0) {
        neorv32_uart0_putc('-');
        value = -value;
    }
    uint32_t int_part = (uint32_t)value;
    uart0_putu(int_part);
    neorv32_uart0_putc('.');
    
    float remainder = value - (float)int_part;
    for (int i = 0; i < precision; i++) {
        remainder *= 10.0f;
        uint32_t d = (uint32_t)(remainder + 0.5f); // Làm tròn
        neorv32_uart0_putc((d % 10) + '0');
    }
}

// 4. Hàm chuyển float thành chuỗi cho OLED (Giữ nguyên thuật toán của bạn nhưng bỏ float thừa)
void float_to_buf_fixed(char *buf, float val, int precision) {
    uint32_t i = (uint32_t)val;
    uint32_t f;
    
    // Sử dụng làm tròn chuẩn hơn
    if (precision == 2) f = (uint32_t)((val - (float)i) * 100.0f + 0.05f);
    else f = (uint32_t)((val - (float)i) * 10.0f + 0.05f);

    // Xử lý hàng đơn vị (cho phép hiển thị 0.xx)
    buf[0] = (i >= 100) ? (i / 100) + '0' : ' ';
    buf[1] = (i >= 10) ? ((i / 10) % 10) + '0' : ' ';
    buf[2] = (i % 10) + '0';
    buf[3] = '.';
    
    if (precision == 2) {
        buf[4] = (f / 10) + '0';
        buf[5] = (f % 10) + '0';
        buf[6] = '\0';
    } else {
        buf[4] = f + '0';
        buf[5] = ' '; 
        buf[6] = '\0';
    }
}

int main() {
    neorv32_uart0_setup(19200, 0);
    PZEM_Init(9600);
    neorv32_gpio_port_set(0x00000000);
    // --- CẤU HÌNH NGẮT (RTE) ---
    neorv32_rte_setup(); 
    neorv32_rte_handler_install(GPIO_TRAP_CODE, my_gpio_irq_handler);
    neorv32_gpio_irq_setup(IN_PIN_BUTTON, GPIO_TRIG_EDGE_FALLING);
    neorv32_gpio_irq_enable(1 << IN_PIN_BUTTON);
    neorv32_cpu_csr_set(CSR_MIE, 1U << GPIO_FIRQ_ENABLE);
    neorv32_cpu_csr_set(CSR_MSTATUS, 1U << CSR_MSTATUS_MIE);

    neorv32_twi_setup(7, 0, 0); 
    neorv32_twi_enable();

    SSD1306_Name myOLED;
    SSD1306_Init(&myOLED);
    
    SSD1306_Clear(&myOLED);
    
    // Dùng chuỗi ngắn để tiết kiệm từng byte .rodata
    SSD1306_GotoXY(&myOLED, 0, 0);  SSD1306_Puts(&myOLED, "U:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 14); SSD1306_Puts(&myOLED, "I:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 28); SSD1306_Puts(&myOLED, "P:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 42); SSD1306_Puts(&myOLED, "F:", &Font_7x10, SSD1306_COLOR_WHITE);

    SSD1306_GotoXY(&myOLED, 85, 0);  SSD1306_Puts(&myOLED, "V", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 85, 14); SSD1306_Puts(&myOLED, "A", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 85, 28); SSD1306_Puts(&myOLED, "W", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 85, 42); SSD1306_Puts(&myOLED, "Hz", &Font_7x10, SSD1306_COLOR_WHITE);
    
    SSD1306_UpdateScreen(&myOLED);


    char val_buf[8];

    while (1) {
        PZEM_ReadAll(1000);
        float power = PZEM_GetPower();
        // --- Logic Cảnh báo ---
        if (power > 5.0) {
            // KHÔNG ép còi về 1 ở đây nếu đã mute, 
            // mà chỉ bật còi nếu CHƯA mute
            if (!is_muted) {
                neorv32_gpio_pin_set(OUT_PIN_BUZZER, 0); // Bật còi
            } else {
                neorv32_gpio_pin_set(OUT_PIN_BUZZER, 1); // Tắt còi (do đang mute)
            }
        } else {
            // Khi công suất thấp, reset lại trạng thái cho lần cảnh báo sau
            is_muted = 0; 
            neorv32_gpio_pin_set(OUT_PIN_BUZZER, 1); // Tắt còi
        }
        // --- In Log UART (Dùng hàm mới, tuyệt đối không dùng printf) ---
        // uart0_puts("D:");
        // neorv32_uart_print_float(PZEM_GetVoltage(), 1);
        // uart0_puts("V ");
        // neorv32_uart_print_float(PZEM_GetCurrent(), 2);
        // uart0_puts("A\r\n");
        // neorv32_uart_print_float(PZEM_GetPower(), 3);
        // uart0_puts("W\r\n");
        // neorv32_uart_print_float(PZEM_GetFrequency(), 4);
        // uart0_puts("Hz\r\n");

        // --- Cập nhật OLED ---
        float_to_buf_fixed(val_buf, PZEM_GetVoltage(), 1);
        SSD1306_GotoXY(&myOLED, 25, 0);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetCurrent(), 2);
        SSD1306_GotoXY(&myOLED, 25, 14);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetPower(), 1);
        SSD1306_GotoXY(&myOLED, 25, 28);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetFrequency(), 1);
        SSD1306_GotoXY(&myOLED, 25, 42);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        SSD1306_UpdateScreen(&myOLED);
        
        neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 1000);
    }
}