#include "BatteryMonitor.hpp"
#include <hal/rp2040_hal.h>
#include <Fw/Types/Assert.hpp>

namespace PipiCube {

BatteryMonitor::BatteryMonitor(const char *compName)
    : BatteryMonitorComponentBase(compName),
      m_lowThreshold(20U),
      m_lastChargeState(0xFFU),  // sentinel: no previous state
      m_lastFull(false),
      m_i2cErrors(0U),
      m_criticalEventSent(false)
{}

BatteryMonitor::~BatteryMonitor() {}

void BatteryMonitor::init(FwEnumStoreType instance) {
    BatteryMonitorComponentBase::init(instance);
}

// ── schedIn_handler ────────────────────────────────────────────────────────────
void BatteryMonitor::schedIn_handler(FwIndexType portNum,
                                      U32 context) {
    (void)portNum;
    (void)context;
    this->doPoll();
}

// ── doPoll ────────────────────────────────────────────────────────────────────
void BatteryMonitor::doPoll(void) {
    // Read registers 0x70 (READ0), 0x71 (READ1), 0x72 (READ2) in one burst.
    uint8_t buf[3] = {0U, 0U, 0U};
    int32_t rc = hal_i2c_read_reg(HAL_I2C1,
                                   IP5306_I2C_ADDR,
                                   IP5306_REG_READ0,
                                   buf,
                                   sizeof(buf),
                                   2000U);
    if (rc != HAL_OK) {
        m_i2cErrors++;
        this->log_WARNING_LO_I2cReadError(IP5306_REG_READ0,
                                           static_cast<I32>(rc));
        this->tlmWrite_I2cErrors(m_i2cErrors);
        return;
    }

    // ── Decode READ0 (0x70) ──────────────────────────────────────────────────
    bool is_full    = ((buf[0] & 0x08U) != 0U);  // bit 3: CHARGE_FULL

    // ── Decode READ1 (0x71): CHARGE_STATE bits [4:2] ─────────────────────────
    uint8_t charge_state = (buf[1] >> 2U) & 0x07U;

    // ── Decode READ2 (0x72): BATTERY_LEVEL bits [4:3] ────────────────────────
    uint8_t level_bits = (buf[2] >> 3U) & 0x03U;
    uint8_t percent    = is_full ? 100U : kLevelPercent[level_bits];

    // ── Read READ3 (0x77) separately for output-enable bit ───────────────────
    uint8_t reg3 = 0U;
    rc = hal_i2c_read_reg(HAL_I2C1, IP5306_I2C_ADDR, IP5306_REG_READ3,
                           &reg3, 1U, 2000U);
    bool output_en = (rc == HAL_OK) && ((reg3 & 0x08U) != 0U);

    // ── Publish telemetry ─────────────────────────────────────────────────────
    this->tlmWrite_BatteryPercent(percent);
    this->tlmWrite_ChargeState(charge_state);
    this->tlmWrite_OutputEnabled(output_en);

    // ── Threshold events ──────────────────────────────────────────────────────
    bool is_charging = (charge_state >= 1U && charge_state <= 3U);

    if (!is_charging) {
        if (percent <= BATTERY_CRITICAL_THRESHOLD && !m_criticalEventSent) {
            this->log_FATAL_BatteryCritical(percent);
            m_criticalEventSent = true;
        } else if (percent <= m_lowThreshold) {
            this->log_WARNING_HI_BatteryLow(percent);
        }
    } else {
        m_criticalEventSent = false;  // reset on charge
    }

    // ── Charge-state transition events ───────────────────────────────────────
    if (charge_state != m_lastChargeState) {
        this->log_ACTIVITY_LO_ChargeStateChanged(charge_state);
        m_lastChargeState = charge_state;
    }
    if (is_full && !m_lastFull) {
        this->log_ACTIVITY_HI_BatteryFull();
    }
    m_lastFull = is_full;
}

// ── Command handlers ──────────────────────────────────────────────────────────

void BatteryMonitor::BATTERY_POLL_cmdHandler(const FwOpcodeType opCode,
                                              const U32          cmdSeq) {
    this->doPoll();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void BatteryMonitor::BATTERY_SET_LOW_THRESHOLD_cmdHandler(
        const FwOpcodeType opCode,
        const U32          cmdSeq,
        const U8           threshold) {
    if (threshold < 1U || threshold > 99U) {
        this->cmdResponse_out(opCode, cmdSeq,
                              Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    m_lowThreshold = threshold;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

} // namespace PipiCube
