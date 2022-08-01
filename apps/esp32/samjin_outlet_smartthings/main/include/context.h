
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "esp_bit_defs.h"

#define CONTEXT_UNKNOWN_VALUE INT16_MIN

typedef struct
{
    portMUX_TYPE spinlock;
    EventGroupHandle_t event_group;

    struct
    {
        volatile float power;
        volatile float current;
        volatile float voltage;
        volatile double energy;
        volatile uint64_t lastenergy;
        volatile uint64_t powaccu;
    } meter;

} context_t;

context_t *context_create(void);

void context_lock(context_t *context);

void context_unlock(context_t *context);

esp_err_t context_set_power_current_voltage(context_t *context, float power, float current, float voltage, double energy, uint64_t powaccu);

esp_err_t context_set_energy(context_t *context, double energy, uint64_t powaccu);

esp_err_t context_set_lastenergy(context_t *context, uint64_t lastenergy);