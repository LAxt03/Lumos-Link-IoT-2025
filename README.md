# Lumos-Link-IoT-2025
# DIY-Anleitung: Lumos-Lampe

Diese Anleitung beschreibt Schritt für Schritt, wie du deine Lumos-Lampe mit ESP32 und Sensoren aufbaust.

---

## 🔧 Benötigte Teile

- **1x ESP32**
- **1x LED-Ring**
- **1x Lichtsensor**
- **1x Sound-Sensor**
- **2x Taster** (jeweils mit 2 Kabeln angeschlossen)
- **1x 3-poliges Kabel**
- **3x Jumperkabel**
- **1x Plastikbase** (schwarz oder weiß)
- **1x transparente Plastikplatte**
- **1x LED-Ring-Kappe**
- **1x Lumos-Geist**
- **1x Stabilitätsring**

---

## 🛠️ Aufbauanleitung

### Schritt 1 – Vorbereitung
1. Den **ESP32** in die transparente Plastikplatte einklinken.  
   → Achte darauf, dass der Stromanschluss zur Kante der Platte zeigt.  
2. Den **Sound-Sensor** direkt neben dem ESP32 einklinken.  

---

### Schritt 2 – LED-Ring einsetzen
3. Den **LED-Ring** von oben mittig auf die Plastikbase legen, Pins nach unten.  
4. Die **LED-Ring-Kappe** darüber platzieren und mit der Base einrasten.  

---

### Schritt 3 – Taster montieren
5. Die beiden **Taster** von oben in die Plastikbase einsetzen.  
   → Kabel von oben durchstecken und die Taster einrasten lassen.  

---

### Schritt 4 – Sensoren einsetzen
6. Den **Lichtsensor** mit dem Anschluss nach unten in die Base, direkt neben dem LED-Ring stecken.  
7. **Jumperkabel** am Lichtsensor anschließen.  
8. **Jumperkabel** am Sound-Sensor anschließen.  

---

### Schritt 5 – Elektrische Verkabelung
9. Das **3-Pol-Kabel** am LED-Ring anschließen:  
   - GND → GND  
   - Power → 5V  
   - Data Input → Pin 12  

10. Anschlüsse LED-Ring zum ESP32:  
    - GND → GND  
    - Power → 5V  
    - Data → GPIO 12  

11. Äußerer Taster (Rand der Base):  
    - ein Kabel → GND  
    - ein Kabel → GPIO 14  

12. Innerer Taster (neben Lichtsensor):  
    - ein Kabel → GND  
    - ein Kabel → GPIO 4  

13. Sound-Sensor:  
    - Jumperkabel → GPIO 34  

14. Lichtsensor:  
    - Jumperkabel → GPIO 33  

---

### Schritt 6 – Zusammenbau
15. Die **Plastikbase** auf die transparente Plastikplatte stecken.  
    → Kabel ordentlich verstauen, damit kein Anschluss blockiert wird.  
16. Den **Stabilitätsring** auf die Plastikbase setzen.  
17. Den **Lumos-Geist** über den Stabilitätsring platzieren.  

---

✅ Fertig! Deine Lumos-Lampe ist nun aufgebaut und bereit zur Inbetriebnahme.  


