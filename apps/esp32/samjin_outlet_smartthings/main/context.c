#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "context.h"

#define context_set(p, v) \
    do                    \
    {                     \
        if ((p) != (v))   \
        {                 \
            (p) = (v);    \
        }                 \
    } while (0)

context_t *context_create(void)
{
    context_t *context = calloc(1, sizeof(context_t));

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    context->spinlock = spinlock;
    context->event_group = xEventGroupCreate();

    context->meter.power = CONTEXT_UNKNOWN_VALUE;
    context->meter.current = CONTEXT_UNKNOWN_VALUE;
    context->meter.voltage = CONTEXT_UNKNOWN_VALUE;

    return context;
}

void context_lock(context_t *context)
{
    portENTER_CRITICAL(&context->spinlock);
}

void context_unlock(context_t *context)
{
    portEXIT_CRITICAL(&context->spinlock);
}

esp_err_t context_set_power_current_voltage(context_t *context, float power, float current, float voltage, double energy, uint64_t powaccu)
{
    context_lock(context);
    context_set(context->meter.power, power);
    context_set(context->meter.current, current);
    context_set(context->meter.voltage, voltage);
    context_set(context->meter.energy, energy);
    context_set(context->meter.powaccu, powaccu);
    context_unlock(context);
    return ESP_OK;
}
esp_err_t context_set_energy(context_t *context, double energy, uint64_t powaccu)
{
  context_lock(context);
  context_set(context->meter.energy, energy);
  context_set(context->meter.powaccu, powaccu);
  context_unlock(context);
  return ESP_OK;
}

esp_err_t context_set_lastenergy(context_t *context, uint64_t lastenergy)
{
  context_lock(context);
  context_set(context->meter.lastenergy, lastenergy);
  context_unlock(context);
  return ESP_OK;
}