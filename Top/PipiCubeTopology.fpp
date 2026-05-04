module PipiCube {

  # ════════════════════════════════════════════════════════════════════════════
  # Component instances
  # ════════════════════════════════════════════════════════════════════════════

  # ── F' standard infrastructure ──────────────────────────────────────────────

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x0010 \
    {
      phase Fpp.ToCpp.Phases.configComponents """
        rateGroupDriver.configure(
            PipiCube::rateDivs,
            FW_NUM_ARRAY_ELEMENTS(PipiCube::rateDivs)
        );
      """
    }

  instance rateGroup1Hz: Svc.ActiveRateGroup base id 0x0100 \
    queue size 10 stack size 4096 priority 90 \
    {
      phase Fpp.ToCpp.Phases.configComponents """
        rateGroup1Hz.configure(
            PipiCube::contexts1Hz,
            FW_NUM_ARRAY_ELEMENTS(PipiCube::contexts1Hz)
        );
      """
    }

  instance rateGroup4Hz: Svc.ActiveRateGroup base id 0x0200 \
    queue size 10 stack size 4096 priority 85 \
    {
      phase Fpp.ToCpp.Phases.configComponents """
        rateGroup4Hz.configure(
            PipiCube::contexts4Hz,
            FW_NUM_ARRAY_ELEMENTS(PipiCube::contexts4Hz)
        );
      """
    }

  instance cmdDisp: Svc.CmdDispatcher base id 0x0300 \
    queue size 20 stack size 4096 priority 70

  instance tlmChan: Svc.TlmChan base id 0x0400 \
    queue size 10 stack size 4096 priority 60

  instance eventLogger: Svc.ActiveLogger base id 0x0500 \
    queue size 10 stack size 4096 priority 65

  instance systemTime: Svc.Time base id 0x0600

  # ── Application components ───────────────────────────────────────────────────

  instance batteryMonitor: PipiCube.BatteryMonitor base id 0x1000 \
    queue size 10 stack size 2048 priority 80

  instance gpsReceiver: PipiCube.GpsReceiver base id 0x2000 \
    queue size 10 stack size 2048 priority 75

  instance loraDriver: PipiCube.LoRaDriver base id 0x3000 \
    queue size 20 stack size 4096 priority 70

  # ════════════════════════════════════════════════════════════════════════════
  # Topology connections
  # ════════════════════════════════════════════════════════════════════════════

  topology PipiCube {

    # ── Rate group driver → rate groups ────────────────────────────────────────
    connections RateGroups {
      rateGroupDriver.CycleOut[0] -> rateGroup4Hz.CycleIn
      rateGroupDriver.CycleOut[1] -> rateGroup1Hz.CycleIn
    }

    # ── 1 Hz schedule → battery monitor + GPS receiver ─────────────────────────
    connections Sched1Hz {
      rateGroup1Hz.RateGroupMemberOut[0] -> batteryMonitor.schedIn
      rateGroup1Hz.RateGroupMemberOut[1] -> gpsReceiver.schedIn
    }

    # ── 4 Hz schedule → LoRa driver (polls for incoming AT+RCV) ───────────────
    connections Sched4Hz {
      rateGroup4Hz.RateGroupMemberOut[0] -> loraDriver.schedIn
    }

    # ── Command dispatch ───────────────────────────────────────────────────────
    connections Commands {
      cmdDisp.compCmdSend[0] -> batteryMonitor.cmdIn
      cmdDisp.compCmdSend[1] -> gpsReceiver.cmdIn
      cmdDisp.compCmdSend[2] -> loraDriver.cmdIn

      batteryMonitor.cmdRegOut    -> cmdDisp.compCmdReg[0]
      gpsReceiver.cmdRegOut       -> cmdDisp.compCmdReg[1]
      loraDriver.cmdRegOut        -> cmdDisp.compCmdReg[2]

      batteryMonitor.cmdResponseOut -> cmdDisp.compCmdStatus[0]
      gpsReceiver.cmdResponseOut    -> cmdDisp.compCmdStatus[1]
      loraDriver.cmdResponseOut     -> cmdDisp.compCmdStatus[2]
    }

    # ── Telemetry ──────────────────────────────────────────────────────────────
    connections Telemetry {
      batteryMonitor.tlmOut -> tlmChan.TlmRecv
      gpsReceiver.tlmOut    -> tlmChan.TlmRecv
      loraDriver.tlmOut     -> tlmChan.TlmRecv
    }

    # ── Events ────────────────────────────────────────────────────────────────
    connections Events {
      batteryMonitor.logOut     -> eventLogger.LogRecv
      gpsReceiver.logOut        -> eventLogger.LogRecv
      loraDriver.logOut         -> eventLogger.LogRecv

      batteryMonitor.logTextOut -> eventLogger.LogTextRecv
      gpsReceiver.logTextOut    -> eventLogger.LogTextRecv
      loraDriver.logTextOut     -> eventLogger.LogTextRecv
    }

    # ── Time ──────────────────────────────────────────────────────────────────
    connections Time {
      batteryMonitor.timeCaller -> systemTime.timeGetPort
      gpsReceiver.timeCaller    -> systemTime.timeGetPort
      loraDriver.timeCaller     -> systemTime.timeGetPort
    }

  }

}
