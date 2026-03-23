#include <neorv32.h>
#include <neorv32_twi.h>
#include <neorv32_uart.h>
#include "PZEM004T.h"
#include "OLED_LCD_SSD1306.h"
#include "fonts.h"

#define OUT_PIN_BUZZER 0
#define IN_PIN_BUTTON  0

// --- KHAI BÁO PROTOTYPE ---
void uart0_puts(const char *s);
void uart0_putu(uint32_t n);
void neorv32_uart_print_float(float value, int precision);
void float_to_buf_fixed(char *buf, float val, int precision);
static uint8_t last_button_state = 1;
// --- BIẾN TOÀN CỤC ---
volatile uint8_t button_event = 0; 
float thresholds[] = {5.0f, 10.0f, 15.0f, 20.0f}; // Các mức ngưỡng xoay vòng
volatile uint8_t threshold_index = 0;             // Chỉ số ngưỡng hiện tại
uint8_t toggle_state = 1;                         // Trạng thái nhấp nháy (1 là tắt)

// --- HÀM NGẮT (ISR) ---
// void my_gpio_irq_handler(void) {
//     if (neorv32_gpio_irq_get() & (1 << IN_PIN_BUTTON)) {
//         button_event = 1; // Chỉ ghi nhận sự kiện nhấn nút
//         NEORV32_GPIO->IRQ_PENDING = (1 << IN_PIN_BUTTON);
//     }
// }

// --- HÀM HỖ TRỢ UART ---
void uart0_puts(const char *s) {
    while (*s) neorv32_uart0_putc(*s++);
}

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

void neorv32_uart_print_float(float value, int precision) {
    if (value < 0) { neorv32_uart0_putc('-'); value = -value; }
    uint32_t int_part = (uint32_t)value;
    uart0_putu(int_part);
    neorv32_uart0_putc('.');
    float remainder = value - (float)int_part;
    for (int i = 0; i < precision; i++) {
        remainder *= 10.0f;
        uint32_t d = (uint32_t)(remainder + 0.5f);
        neorv32_uart0_putc((d % 10) + '0');
    }
}

void float_to_buf_fixed(char *buf, float val, int precision) {
    uint32_t i = (uint32_t)val;
    uint32_t f;
    if (precision == 2) f = (uint32_t)((val - (float)i) * 100.0f + 0.05f);
    else f = (uint32_t)((val - (float)i) * 10.0f + 0.05f);

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
    // neorv32_rte_setup(); 
    // neorv32_rte_handler_install(GPIO_TRAP_CODE, my_gpio_irq_handler);
    // neorv32_gpio_irq_setup(IN_PIN_BUTTON, GPIO_TRIG_EDGE_FALLING);
    // neorv32_gpio_irq_enable(1 << IN_PIN_BUTTON);
    // neorv32_cpu_csr_set(CSR_MIE, 1U << GPIO_FIRQ_ENABLE);
    // neorv32_cpu_csr_set(CSR_MSTATUS, 1U << CSR_MSTATUS_MIE);

    neorv32_twi_setup(7, 0, 0); 
    neorv32_twi_enable();

    SSD1306_Name myOLED;
    SSD1306_Init(&myOLED);
    
    SSD1306_Clear(&myOLED);
    
    // Dùng chuỗi ngắn để tiết kiệm từng byte .rodata
    SSD1306_GotoXY(&myOLED, 0, 0);  SSD1306_Puts(&myOLED, "U:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 13); SSD1306_Puts(&myOLED, "I:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 26); SSD1306_Puts(&myOLED, "P:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 39); SSD1306_Puts(&myOLED, "F:", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 0, 52); SSD1306_Puts(&myOLED, "SET:", &Font_7x10, SSD1306_COLOR_WHITE);

    SSD1306_GotoXY(&myOLED, 75, 0);  SSD1306_Puts(&myOLED, "V", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 75, 13); SSD1306_Puts(&myOLED, "A", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 75, 26); SSD1306_Puts(&myOLED, "W", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 75, 39); SSD1306_Puts(&myOLED, "Hz", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(&myOLED, 75, 52); SSD1306_Puts(&myOLED, "W", &Font_7x10, SSD1306_COLOR_WHITE);
    
    SSD1306_UpdateScreen(&myOLED);


    char val_buf[12];

    while (1) {
        // 2. Xử lý sự kiện nút nhấn (Thay đổi ngưỡng)
        // if (button_event) {
        //     neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 50); // Debounce
        //     if ((NEORV32_GPIO->PORT_IN & (1 << IN_PIN_BUTTON)) == 0) {
        //         threshold_index = (threshold_index + 1) % 4; // Xoay vòng 0->1->2->3->0
        //         uart0_puts("Mức ngưỡng mới: ");
        //         neorv32_uart_print_float(thresholds[threshold_index], 1);
        //         uart0_puts("W\r\n");
        //     }
        //     button_event = 0;
        //     // Chờ nhả nút để không bị nhảy mức liên tục
        //     while ((NEORV32_GPIO->PORT_IN & (1 << IN_PIN_BUTTON)) == 0);
        // }

        // 2. Xử lý nút nhấn KHÔNG GÂY KẸT (Non-blocking)
        
        uint8_t current_button_state = (NEORV32_GPIO->PORT_IN >> IN_PIN_BUTTON) & 1;

        if (last_button_state == 1 && current_button_state == 0) { // Cạnh xuống
            neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 20); // Debounce ngắn
            threshold_index = (threshold_index + 1) % 4;
        
        // Chỉ in khi thay đổi để tránh tràn log
            uart0_puts("\r\n--- New Threshold: ");
            neorv32_uart_print_float(thresholds[threshold_index], 1);
            uart0_puts("W ---\r\n");
        }
        last_button_state = current_button_state;

        // 1. Đọc dữ liệu từ PZEM
        PZEM_ReadAll(1000);
        float power = PZEM_GetPower();
        float current_threshold = thresholds[threshold_index];


        // 4. In log UART tóm tắt (Gộp vào 1 dòng để tránh quá tải UART)
        uart0_puts("U:"); uart0_putu((uint32_t)PZEM_GetVoltage());
        uart0_puts("V P:"); neorv32_uart_print_float(power, 1);
        uart0_puts("W\r\n");

        // --- Cập nhật OLED ---
        float_to_buf_fixed(val_buf, PZEM_GetVoltage(), 1);
        SSD1306_GotoXY(&myOLED, 20, 0);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetCurrent(), 2);
        SSD1306_GotoXY(&myOLED, 20, 13);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetPower(), 1);
        SSD1306_GotoXY(&myOLED, 20, 26);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        float_to_buf_fixed(val_buf, PZEM_GetFrequency(), 1);
        SSD1306_GotoXY(&myOLED, 20, 39);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        // Hiển thị ngưỡng đang cài đặt (SET)
        float_to_buf_fixed(val_buf, current_threshold, 1);
        SSD1306_GotoXY(&myOLED, 20, 52);
        SSD1306_Puts(&myOLED, val_buf, &Font_7x10, SSD1306_COLOR_WHITE);

        SSD1306_UpdateScreen(&myOLED);
        
        // 3. Logic Cảnh báo nhấp nháy
        if (power > current_threshold) {
            toggle_state = !toggle_state; // Đảo trạng thái để nhấp nháy
            neorv32_gpio_pin_set(OUT_PIN_BUZZER, toggle_state);
        } else {
            toggle_state = 1; // Đảm bảo tắt khi không quá tải
            neorv32_gpio_pin_set(OUT_PIN_BUZZER, 1);
        }

        neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 200);
    }
}