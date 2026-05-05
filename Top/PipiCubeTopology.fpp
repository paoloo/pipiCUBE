module PipiCube {

  # ════════════════════════════════════════════════════════════════════════════
  # Component instances
  # ════════════════════════════════════════════════════════════════════════════

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x0010

  instance rateGroup1Hz: Svc.ActiveRateGroup base id 0x0100 \
    queue size 10 stack size 4096 priority 90

  instance rateGroup4Hz: Svc.ActiveRateGroup base id 0x0200 \
    queue size 10 stack size 4096 priority 85

  instance cmdDisp: Svc.CommandDispatcher base id 0x0300 \
    queue size 20 stack size 4096 priority 70

  instance tlmChan: Svc.TlmChan base id 0x0400 \
    queue size 10 stack size 4096 priority 60

  instance eventManager: Svc.EventManager base id 0x0500 \
    queue size 10 stack size 4096 priority 65

  instance systemTime: Svc.OsTime base id 0x0600

  instance batteryMonitor: PipiCube.BatteryMonitor base id 0x1000 \
    queue size 10 stack size 2048 priority 80

  instance gpsReceiver: PipiCube.GpsReceiver base id 0x2000 \
    queue size 10 stack size 2048 priority 75

  instance loraDriver: PipiCube.LoRaDriver base id 0x3000 \
    queue size 20 stack size 4096 priority 70

  # ════════════════════════════════════════════════════════════════════════════
  # Topology
  # ════════════════════════════════════════════════════════════════════════════

  topology PipiCube {

    # ── Topology members ──────────────────────────────────────────────────────
    instance rateGroupDriver
    instance rateGroup1Hz
    instance rateGroup4Hz
    instance cmdDisp
    instance tlmChan
    instance eventManager
    instance systemTime
    instance batteryMonitor
    instance gpsReceiver
    instance loraDriver

    # ── Pattern specifiers (auto-wire standard F' services) ───────────────────
    command connections instance cmdDisp
    event connections instance eventManager
    telemetry connections instance tlmChan
    time connections instance systemTime

    # ── Rate group driver → rate groups ────────────────────────────────────────
    connections RateGroups {
      rateGroupDriver.CycleOut[0] -> rateGroup4Hz.CycleIn
      rateGroupDriver.CycleOut[1] -> rateGroup1Hz.CycleIn
    }

    # ── 1 Hz schedule (battery + GPS + tlm + cmd) ─────────────────────────────
    connections Sched1Hz {
      rateGroup1Hz.RateGroupMemberOut[0] -> batteryMonitor.schedIn
      rateGroup1Hz.RateGroupMemberOut[1] -> gpsReceiver.schedIn
      rateGroup1Hz.RateGroupMemberOut[2] -> tlmChan.Run
      rateGroup1Hz.RateGroupMemberOut[3] -> cmdDisp.run
    }

    # ── 4 Hz schedule (LoRa AT+RCV polling) ───────────────────────────────────
    connections Sched4Hz {
      rateGroup4Hz.RateGroupMemberOut[0] -> loraDriver.schedIn
    }

  }

}
