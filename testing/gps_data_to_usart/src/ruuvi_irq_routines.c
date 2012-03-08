// Collection of the interrupt routines



CH_IRQ_HANDLER(myIRQ) {
  CH_IRQ_PROLOGUE();
 
  /* IRQ handling code, preemptable if the architecture supports it.*/
 
  chSysLockFromIsr();
  /* Invocation of some I-Class system APIs, never preemptable.*/
  chSysUnlockFromIsr();
 
  /* More IRQ handling code, again preemptable.*/
 
  CH_IRQ_EPILOGUE();
}
