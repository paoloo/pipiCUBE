module PipiCube {

  @ Passive component that bridges the hardware timer ISR to the F' rate group.
  @ Call tick() from the main loop; it invokes CycleOut which is wired to
  @ rateGroupDriver.CycleIn in the topology.
  passive component BaremetalTimer {
    output port CycleOut: Svc.Cycle
  }

}
