# Automated Closed Loop System for Direct Air CO₂ Capture and Mineralization

A compact experimental system designed to capture atmospheric CO₂ and convert it into stable compounds using a controlled chemical process. The setup integrates embedded control, fluid handling, and basic data monitoring to study CO₂ reduction cycles.

---

Overview

This project demonstrates a working prototype for CO₂ capture using a chemical absorption-regeneration loop. The system is automated using a microcontroller and relay-controlled components, enabling timed operation of pumps, valves, and air flow.

The focus is on:

- Controlled CO₂ absorption
- Regrowth of absorbent solution
- Conversion to stable carbonate compounds
- Simple monitoring and optimization

---

System Components

- Microcontroller-based control (ESP32)
- Relay-controlled pumps and solenoid valve
- Air flow system for CO₂ intake
- Liquid chambers for chemical processing
- CO₂, temperature, and humidity sensing
- LCD-based local display
- Optional web dashboard for monitoring

---

Working Principle

1. Air is introduced into the system.
2. CO₂ is absorbed into an alkaline solution.
3. The solution undergoes a conversion step forming carbonate compounds.
4. The absorbent is regenerated for reuse.
5. The process runs in repeated cycles with controlled timing.

---

Experimental Validation

Carbonate formation was verified through a standard acid reaction test.
Effervescence observed during testing indicates the presence of carbonate, confirming successful CO₂ capture and conversion.

---

Files Included

- "dashboard.html" — Web-based monitoring interface
- "experimental-setup.jpeg" — Physical system setup
- "reaction-confirmation.jpeg" — Validation of carbonate formation
- Firmware files — Embedded control logic

---

Notes

This repository contains a simplified representation of the system for demonstration purposes.
Detailed methodologies, optimizations, and analysis are part of ongoing research work.

---

Status

Active development and testing.
Additional modules and improvements will be integrated in future iterations.

---

Disclaimer

This project is intended for educational and experimental use.
Some aspects have been intentionally simplified or abstracted.
