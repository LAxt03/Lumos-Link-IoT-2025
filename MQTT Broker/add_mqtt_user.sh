#!/bin/bash

# === Konfiguration ===
CONFIG_DIR="./config"                      # Dein lokaler Mosquitto config Ordner
PWFILE="${CONFIG_DIR}/pwfile"             # Pfad zur pwfile
IMAGE="eclipse-mosquitto"                 # Mosquitto Docker Image

# === Nutzerabfrage ===
read -p "MQTT Benutzername: " USERNAME

# Passwort verdeckt eingeben
read -s -p "Passwort: " PASSWORD
echo
read -s -p "Passwort (wiederholen): " PASSWORD_REPEAT
echo

# === Validierung ===
if [ "$PASSWORD" != "$PASSWORD_REPEAT" ]; then
  echo "Passwörter stimmen nicht überein. Abbruch."
  exit 1
fi

# === Passwort setzen/ändern ===
echo "🔐 Benutzer \"$USERNAME\" wird hinzugefügt/geändert …"

docker run --rm -i \
  -v "$(pwd)/config:/mosquitto/config" \
  $IMAGE \
  sh -c "echo '$PASSWORD' | mosquitto_passwd -b /mosquitto/config/pwfile '$USERNAME'"

if [ $? -eq 0 ]; then
  echo "Benutzer erfolgreich hinzugefügt/geändert."
else
  echo "Fehler beim Hinzufügen/Ändern des Benutzers."
fi
