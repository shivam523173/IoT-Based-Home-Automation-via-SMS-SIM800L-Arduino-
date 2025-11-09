# IoT-Based Home Automation via SMS (SIM800L + Arduino)

Control four AC/DC loads over **SMS** using a **SIM800L GSM module** and **Arduino**. Send commands like `load1on` or `alloff` from authorized phone numbers. Relay states are saved in **EEPROM** so they persist after power loss.

---

## ✨ Features
- SMS control: on/off per load + `allon`/`alloff`
- `loadstatus` reply with current states
- Authorized numbers list (edit in code)
- EEPROM persistence for relay states
- Works with active-low relay boards

---

## 🧩 Hardware
| Part | Qty |
|---|---|
| Arduino (Uno/Nano/Mega) | 1 |
| SIM800L GSM module | 1 |
| 4-Channel Relay Module (active-low) | 1 |
| External 5V supply (stable) | 1 |
| Level shifting for SIM800 RX (recommended) | 1 |
| Wires, SIM with SMS plan | – |

> **Power tips:** SIM800L needs a solid 4V supply with enough current (2A peak). Use a buck converter and keep GSM GND common with Arduino GND.

---

## 🔌 Wiring (core)
- **SIM800 TX → Arduino D8**, **SIM800 RX ← Arduino D9** *(use resistor divider for 5V → 2.8–3.3V)*
- **Relays**: IN1→D2, IN2→D3, IN3→D4, IN4→D5 (active-low)
- Common GND between Arduino, SIM800, and relays

---

## 📩 SMS Commands (case-insensitive)
