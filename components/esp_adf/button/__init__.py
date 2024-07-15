import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_SENSOR, ICON_POWER, UNIT_EMPTY

from .. import (
    CONF_ESP_ADF_ID,
    ESPADF,
    esp_adf_ns,
)

DEPENDENCIES = ["esp_adf"]
AUTO_LOAD = ["sensor"]

CONF_BUTTON1_SENSOR = "button1_sensor"
CONF_BUTTON2_SENSOR = "button2_sensor"
CONF_BUTTON3_SENSOR = "button3_sensor"
CONF_BUTTON4_SENSOR = "button4_sensor"
CONF_BUTTON5_SENSOR = "button5_sensor"
CONF_BUTTON6_SENSOR = "button6_sensor"

ESPADFButtonHandler = esp_adf_ns.class_("ButtonHandler", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESPADFButtonHandler),
            cv.GenerateID(CONF_ESP_ADF_ID): cv.use_id(ESPADF),
            cv.Optional(CONF_BUTTON1_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
            cv.Optional(CONF_BUTTON2_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
            cv.Optional(CONF_BUTTON3_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
            cv.Optional(CONF_BUTTON4_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
            cv.Optional(CONF_BUTTON5_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
            cv.Optional(CONF_BUTTON6_SENSOR): sensor.sensor_schema(
                UNIT_EMPTY, ICON_POWER, 0
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.only_with_esp_idf,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_ESP_ADF_ID])

    if CONF_BUTTON1_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON1_SENSOR])
        cg.add(var.button1_sensor, sens)

    if CONF_BUTTON2_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON2_SENSOR])
        cg.add(var.button2_sensor, sens)

    if CONF_BUTTON3_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON3_SENSOR])
        cg.add(var.button3_sensor, sens)

    if CONF_BUTTON4_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON4_SENSOR])
        cg.add(var.button4_sensor, sens)

    if CONF_BUTTON5_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON5_SENSOR])
        cg.add(var.button5_sensor, sens)

    if CONF_BUTTON6_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_BUTTON6_SENSOR])
        cg.add(var.button6_sensor, sens)
