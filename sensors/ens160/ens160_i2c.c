#include "ens160_i2c.h"

uint8_t write_reg_to_ens160(uint8_t addr, uint8_t reg) {
    uint8_t tx_buf[1] = {reg}; // Write the register
    bool write_ok =
        furi_hal_i2c_tx(&furi_hal_i2c_handle_external, addr << 1, tx_buf, sizeof(tx_buf), 50);
    if(write_ok) {
        return 0;
    }
    return -1;
}

uint8_t write_cmd_to_ens160(uint8_t addr, uint8_t reg, uint8_t cmd) {
    uint8_t tx_buf[2] = {reg, cmd}; // Write the register, and command
    bool write_ok =
        furi_hal_i2c_tx(&furi_hal_i2c_handle_external, addr << 1, tx_buf, sizeof(tx_buf), 50);
    if(write_ok) {
        return 0;
    }
    return -1;
}

uint8_t read_from_ens160_reg(uint8_t addr, uint8_t reg, uint8_t* buf) {
    uint8_t result = write_reg_to_ens160(addr, reg);
    if(result != 0) {
        return result;
    }
    bool read_ok = furi_hal_i2c_rx(&furi_hal_i2c_handle_external, addr << 1, buf, sizeof(buf), 50);
    if(read_ok) {
        return 0;
    }
    return -1;
}

uint8_t ens160_read_measurement(uint8_t* aiq_index, uint16_t* eco2, uint16_t* tvoc) {
    uint8_t aiq_index_buf[1];
    uint8_t voc_buf[2];
    uint8_t eco2_buf[2];
    uint8_t status_buf[1];
    uint8_t status = 0;
    uint8_t result = 0;

    do {
        furi_delay_ms(1);
        result += read_from_ens160_reg(ENS160_I2CADDR_1, ENS160_REG_DATA_STATUS, status_buf);
        status = status_buf[0];

    } while(!IS_NEWDAT(status));

    // Read predictions
    if(IS_NEWDAT(status)) {
        result += read_from_ens160_reg(ENS160_I2CADDR_1, ENS160_REG_DATA_AQI, aiq_index_buf);
        *aiq_index = aiq_index_buf[0];
        result += read_from_ens160_reg(ENS160_I2CADDR_1, ENS160_REG_DATA_TVOC, voc_buf);
        *tvoc = voc_buf[0] | ((uint16_t)voc_buf[1] << 8);
        result += read_from_ens160_reg(ENS160_I2CADDR_1, ENS160_REG_DATA_ECO2, eco2_buf);
        *eco2 = eco2_buf[0] | ((uint16_t)eco2_buf[1] << 8);
    }
    return result;
}

uint8_t ens160_init() {
    furi_delay_ms(ENS160_BOOTING);
    int result = write_cmd_to_ens160(ENS160_I2CADDR_1, ENS160_REG_OPMODE, ENS160_OPMODE_RESET);
    furi_delay_ms(ENS160_BOOTING);
    result += write_cmd_to_ens160(ENS160_I2CADDR_1, ENS160_REG_OPMODE, ENS160_OPMODE_STD);
    furi_delay_ms(ENS160_BOOTING);

    return result;
}

bool ens160_i2c_hal_init(void) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    return furi_hal_i2c_is_device_ready(
        &furi_hal_i2c_handle_external, (ENS160_I2CADDR_1 << 1), 50);
}

void ens160_i2c_hal_free(void) {
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}
