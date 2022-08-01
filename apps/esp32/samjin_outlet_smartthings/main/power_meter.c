
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include "power_meter.h"
#include "device_control.h"

#include "global.h"
#include "esp_timer.h"

#include "esp_system.h"

#define TXD_PIN (GPIO_NUM_23)
#define RXD_PIN (GPIO_NUM_18)

#define BUF_SIZE 128

#define RELAY_MAX_CURRENT 16 // ampere

static const int uart_num = UART_NUM_1;
uint64_t power_accu = 0;

static void initialize_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_ODD,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
}

static void OVER_CURRENT_CHECK(float current)
{
    float val = current / 1000;
    if (val > RELAY_MAX_CURRENT)
    {
        change_switch_state(SWITCH_OFF);
        cap_switch_data->set_switch_value(cap_switch_data, caps_helper_switch.attr_switch.value_off);
        cap_switch_data->attr_switch_send(cap_switch_data);
    }
}

static int32_t register_read(uint16_t addr)
{
    int i, len = 0;
    uint8_t rx_buf[8];
    uint8_t tx_buf[8] = {0};
    tx_buf[0] = HEAD_BYTE;
    tx_buf[1] = (uint8_t)((addr & 0x0f00) >> 4) + READ_BIT;
    tx_buf[2] = (uint8_t)(addr & 0x00ff);
    for (i = 0; i < 7; i++)
    {
        tx_buf[7] += tx_buf[i];
    }
    tx_buf[7] = (~tx_buf[7]) + 0x33;
    uart_write_bytes(uart_num, (const char *)tx_buf, sizeof(tx_buf));
    len = uart_read_bytes(uart_num, rx_buf, 8, 100);
    if (len > 0)
    {
        if ((rx_buf[0] == HEAD_BYTE) && (rx_buf[1] == READ_BIT))
        {
            uint8_t cs = 0;
            int32_t reg_data = 0;
            for (i = 0; i < len - 1; i++)
            {
                cs += rx_buf[i];
            }
            cs = (~cs) + 0x33;
            if (cs == rx_buf[len - 1])
            {
                for (i = 0; i < 4; i++)
                {
                    reg_data |= (rx_buf[i + 3] << (i * 8));
                }
                return abs(reg_data);
            }
        }
    }
    return 0;
}

static void register_write(uint16_t addr, uint32_t data)
{
    uint8_t tx_buf[8] = {0};
    tx_buf[0] = HEAD_BYTE;
    tx_buf[1] = (uint8_t)((addr & 0x0f00) >> 4) + WRITE_BIT;
    tx_buf[2] = (uint8_t)(addr & 0x00ff);
    tx_buf[3] = (uint8_t)(data & 0x000000ff);
    tx_buf[4] = (uint8_t)((data & 0x0000ff00) >> 8);
    tx_buf[5] = (uint8_t)((data & 0x00ff0000) >> 16);
    tx_buf[6] = (uint8_t)((data & 0xff000000) >> 24);
    tx_buf[7] = 0;
    for (int i = 0; i < 7; i++)
    {
        tx_buf[7] += tx_buf[i];
    }
    tx_buf[7] = (~tx_buf[7]) + 0x33;
    uart_write_bytes(uart_num, (const char *)tx_buf, sizeof(tx_buf));
}
// static void energy_value(TimerHandle_t xTimer)
// {
//     if (iot_ctx->meter.power > 0)
//     {
//         ctx->meter.powaccu;
//     }
//     xTimerStart(xTimer, 0);
// }

static void power_meter_task(void *arg)
{
    int64_t prev = esp_timer_get_time()/16800;
    context_t *context = (context_t *)arg;
    register_write(SFTRST, 0x4572BEAF);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    context_set_energy(context, 0, power_accu);
    // register_write(SysCtrl, 0x28034008); // First Commit
    register_write(SysCtrl, 0x28414008);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    // register_write(IAC, 0xFB68A048);
    register_write(IAC, 0xF1380F87);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    register_write(UC, 0xFDEF7BDF);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    register_write(PAC, 0x0359CD12);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    uart_flush(uart_num);
    TickType_t last_wake_time = xTaskGetTickCount();
    while (true)
    {
        float power = register_read(PAAVG) / 50000.f;
        float current = register_read(IAAVG) / 50000.f;
        float voltage = register_read(UAVG) / 1000000.f;
        int64_t stamp = esp_timer_get_time()/16800;
        int64_t dur = stamp - prev;
        power = (power > 0.01) ? power : 0;
        power_accu = context->meter.powaccu + (power * dur);
        // double energy = (((double)power_accu/1.040) / 360000000)/1000;  
        // double energy = ((double)power_accu/1.040);
        // double energy = ((double)power_accu/1.040)/36000000000;
        // double energy = ((double)power_accu)/50576725193; // 5 menit awal berhasil ke 1 jam jauh
        double energy = ((double)power_accu)/3600000000;
        // printf("\n\nDuration: %lld\n", stamp-prev);
        // printf("\nPower: %f", power);
        // printf("\nPower_Accu:%lld", power_accu);
        // printf("\nNilai energy: %f" , energy);
        // printf("\n------------------\n");

        context_set_power_current_voltage(context, power, current, voltage, energy, power_accu);
        // context_set_energy(context, energy, power_accu); // ngaruh ke nilai - pada apps?
        OVER_CURRENT_CHECK(current);
        vTaskDelayUntil(&last_wake_time, 1000 / portTICK_PERIOD_MS);
    }
}

void power_meter_init(context_t *context)
{
    initialize_uart();
    xTaskCreate(power_meter_task, "power_meter", 1024 * 4, context, 5, NULL);
}