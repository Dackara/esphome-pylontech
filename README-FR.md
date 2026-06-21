# Composant ESPHome Pylontech

[🇬🇧 English Documentation](README.md)

---

🎥 **YouTube** https://youtube.com/@DackaraDemo

💬 **Discord** https://discord.gg/DAeEtv4e7k

☕ **Buy Me a Coffee** https://github.com/sponsors/Dackara

---

# État du projet

**Version actuelle :** v1.0.0

État actuel des validations :

| Matériel                   | État         |
| -------------------------- | ------------ |
| Pylontech US2000C (Master) | ✅ Validé    |
| Pylontech US2000C (Slave)  | ⚠ En cours   |
| Autres modèles             | ⚠ En cours   |

---

# Présentation

ESPHome Pylontech Component est un composant externe ESPHome permettant de communiquer nativement avec les batteries Pylontech via UART.

Contrairement aux implémentations historiques limitées à une partie du protocole, ce projet vise à exploiter un maximum d'informations utiles tout en restant stable, performant et adapté aux grandes installations Home Assistant.

Fonctionnalités principales :

* Mode Master et Slave
* Surveillance PWRSYS
* Surveillance PWR
* Surveillance GETPWR
* Lecture INFO
* Lecture STAT
* Lecture BAT
* Lecture SOH
* Tensions cellules
* Températures cellules
* États des cellules
* Intégration Home Assistant
* Optimisation mémoire PSRAM optionnelle
* Outils avancés de diagnostic

---

# Fonctionnement

## Boucle rapide

Commandes rapides :

* PWRSYS
* PWR
* GETPWR

Ces commandes fournissent les informations temps réel.

## Boucle lente

Commandes lentes :

* INFO
* STAT
* BAT
* SOH

Ces commandes fournissent les informations détaillées et historiques.

## Optimisation mémoire

La version 1.0.0 introduit trois modes mémoire :

```yaml
memory_mode: internal
memory_mode: auto
memory_mode: psram
```

### internal

Comportement standard ESPHome.

### auto

Utilisation automatique de la PSRAM lorsqu'elle est disponible.

### psram

Déplacement forcé des gros buffers gérés par le composant vers la PSRAM lorsqu'elle est disponible.

Ce mode est principalement destiné aux projets ESP32-S3 fortement chargés comme PVBrain.

Le buffer UART interne d'ESPHome n'est pas concerné par ce réglage.

---

# Installation

Ajoutez le composant externe :

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/Dackara/esphome-pylontech
```

Consultez le dossier documentation pour les exemples complets.

---

# Diagnostics

Les entités de diagnostic sont principalement destinées :

* au développement
* à la validation du protocole
* au dépannage
* au debug avancé

La majorité des utilisateurs n'en auront pas besoin.

---

# Remerciements

Un immense merci à [KJM](https://github.com/kjm5759) dont l'ancienne implémentation "Custom" ESPHome et son récent composant, a servi de base de travail et de référence principale pour ce projet.

Merci également à [Functionpointer](https://github.com/functionpointer) pour ses travaux autour du protocole Pylontech et de sa gestion UART.

---

# Notes de développement

Ce projet est développé par Dackara.

Certaines phases d'analyse, de revue de code et d'optimisation ont été réalisées avec l'aide d'outils d'assistance basés sur l'intelligence artificielle tels que ChatGPT, Codex et Claude.

L'ensemble des choix techniques, des validations et des décisions de développement restent sous la responsabilité du mainteneur du projet.

---

# Avertissement

Ce projet n'est pas affilié à Pylontech.

Utilisation à vos risques et périls.
