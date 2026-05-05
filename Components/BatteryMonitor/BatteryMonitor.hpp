#ifndef PIPICUBE_BATTERY_MONITOR_HPP
#define PIPICUBE_BATTERY_MONITOR_HPP

#include <Components/BatteryMonitor/BatteryMonitorComponentAc.hpp>
#include <config/FpConfig.h>

namespace PipiCube {

// ── IP5306 I2C constants ──────────────────────────────────────────────────────
static const uint8_t IP5306_I2C_ADDR   = 0x75U;
static const uint8_t IP5306_REG_READ0  = 0x70U; // CHARGE_FULL | CHARGE_ENABLE
static const uint8_t IP5306_REG_READ1  = 0x71U; // CHARGE_STATE [4:2]
static const uint8_t IP5306_REG_READ2  = 0x72U; // BATTERY_LEVEL [4:3]
static const uint8_t IP5306_REG_READ3  = 0x77U; // OUTPUT_EN bit 3
static const uint8_t IP5306_REG_SYS0   = 0x00U; // BOOST_EN | CHARGER_EN

// Battery level map: READ2 bits[4:3] → midpoint percent
static const uint8_t kLevelPercent[4] = {12U, 37U, 62U, 88U};

// Hard-coded critical threshold — never changes
static const uint8_t BATTERY_CRITICAL_THRESHOLD = 5U;

class BatteryMonitor final : public BatteryMonitorComponentBase {
  public:
    explicit BatteryMonitor(const char *compName);
    ~BatteryMonitor() override;

    void init(FwSizeType queueDepth, FwEnumStoreType instance = 0);

  private:
    // ── F' autocoded virtual methods ─────────────────────────────────────────

    void schedIn_handler(FwIndexType portNum,
                         U32 context) override;

    void BATTERY_POLL_cmdHandler(FwOpcodeType opCode,
                                  U32          cmdSeq) override;

    void BATTERY_SET_LOW_THRESHOLD_cmdHandler(FwOpcodeType opCode,
                                               U32          cmdSeq,
                                               U8           threshold) override;

    // ── Private helpers ──────────────────────────────────────────────────────
    void doPoll(void);

    // ── State ─────────────────────────────────────────────────────────────────
    U8   m_lowThreshold;    // configurable low-battery warning level
    U8   m_lastChargeState; // detect transitions
    bool m_lastFull;
    U32  m_i2cErrors;
    bool m_criticalEventSent; // avoid flooding fatal events
};

} // namespace PipiCube

#endif // PIPICUBE_BATTERY_MONITOR_HPP
