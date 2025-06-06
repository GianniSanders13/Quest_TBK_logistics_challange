# Leaphy Logistics Challenge – Spelcode

Welkom! In deze map bevindt zich alle benodigde code voor de **Leaphy Logistics Challenge**. Voor deze challenge is een spel ontwikkeld waarin voertuigen via ESP-NOW communiceren. Deze handleiding legt uit welke onderdelen nodig zijn om het spel succesvol te laten draaien.

---

## 📦 Benodigde Code

### 🚦 Communicatiemodules

Er zijn **twee Arduino Nano ESP32**-modules nodig voor de communicatie. De code voor deze modules is te vinden in de map `control panel code`:

- `IndustriegebiedCommunicatieModule`
- `PizzatownCommunicatieModule`

> ⚠️ **Belangrijk:**  
> Als een voertuig niet reageert of niet rijdt, controleer dan het **MAC-adres** in de communicatiemodule. Dit moet overeenkomen met het MAC-adres van het voertuig. Pas dit indien nodig aan in de code!

---

### 🚚 Voertuigen

In de map `Transportation vehicle code` vind je de benodigde code voor de twee voertuigen:

- **Bezorgvoertuig (Pizzatown):**  
  Gebruik `TransportationVehicleCode`

- **Vrachtvoertuig (Industriegebied):**  
  Gebruik `FreightTransportVehicle`

---

## 🛠 Tools

De map `tools` bevat diverse hulpprogramma’s die handig zijn tijdens het testen en instellen:

- **RFID Reader & Writer**  
  Leest de UID van RFID-tags en biedt de mogelijkheid om deze te wijzigen.

- **ESPNow Receiver Test**  
  Gebruik deze testcode om te controleren of ESP-NOW-berichten correct worden ontvangen.

- **MAC-adres Reader**  
  Leest het MAC-adres van een ESP32. Dit is nodig voor het correct instellen van communicatie tussen modules.

---

## ❓ Problemen?

- Controleer of het juiste MAC-adres is ingesteld in de communicatiecode.
- Zorg dat alle modules correct zijn geüpload en verbonden met voeding.
- Test communicatie met de ESPNow Receiver Tool indien nodig.

Veel succes met de challenge! 🚀

---

