#  Display MQTT ovládaný gesty
Cílem projektu bylo vytvoření programu pro ESP32. Projekt se provádí na ESP32 desce s použitím periferií, a to konkrétně OLED displeje SPI[[1]](#1) a senzoru snímání gest I2C[[2]](#2). Se zařízením se interaguje pomocí gest prováděnými nad senzorem.

## Zapojení
Bylo využito referenčního zapojení dle dodaného zadání[[2]](#3). ESP32 se dá zapojit pomocí micro USB do zdroje nebo počítače. Všechny periferie měly přivedeny proud z 3,3V výstupu desky.

### SPI displej
Displej komunikuje s deskou pomocí více portů. Zapojení portů:

| ESP32 | SPI Displej |
| --- | --- |
| 3,3V | VCC |
| GND | GND |
| IO48 | D0 (SCLK) |
| IO23 | D1 (MOSI) |
| IO5 | CS |
| IO27 | DC (DATA/control) |
| IO17 | RESET |

### I2C Senzor
Senzor je schopen snímat jak gesta, tak i barvy. V projektu je použito pouze dat ze snímání gest. Zapojení portů:

| ESP32 | I2C senzor |
| --- | --- |
| 3,3V | VCC |
| GND | GND |
| SCL | SCL |
| SDA | SDA |

## Příprava
K vývoji bylo používáno ARDUINO IDE společně s využitím Arduino knihoven. Zvoleno z důvodů dobré kompatibility s ESP32 deskou a s velkou možností rozvíjet projekt lehčeji směrem k větším možnostem a složitějším věcem.

Na příkladech dodaných společně se zadáním[[4]](#4) bylo vyzkoušen zapojení displeje a jeho funkčnost. Následně byl stejný proces proveden i se senzorem, tam bylo však použito jiného základního zdrojového kódu[[5]](#5). Tyto kódy nadále nefigurují v projektu a byly nahrazeny vlastními upravenými zdrojovými kódy.

Po ověření funkčnosti byl zahájen vývoj dle zadání.

## Implementační detaily

Implementace má základní obrazovku, ze které se uživatel dostane jakýmkoliv gestem. Následně vchází do menu, kde má výběr v posuvném menu ze 3 položek: "Time", "Person" a "Height". 

Implementovány jsou pouze 4 směry pohybu. Nahoru, dolů, doleva, doprava. Tyto směry implementují logické kroky v systému

| Směr | Reakce programu |
| --- | --- |
| /\ Nahoru | Posunutí vzhůru, navýšení hodnoty |
| \\/ Dolů | Posunutí dolů, snížení hodnoty |
| < do leva | Navrácení zpět v krkoku, neuložení hodnoty |
| > Do prava | Posun vpřed, uložení hodnoty, výběr z možností |

### MQTT klient
Jako kleinta, kde jsou zasílána data, byla zvolena online služba MyQttHub[[6]](#6). Umožňuje přijímat a odesílat data pomocí MQTT a v režimu zdarma umožňuje velkou škálu věcí, ovšem s omezenými možnostmi.

### MENU - Time
Po zvolení si nabídky z menu se zobrazí nové menu, tentokrát se dvěmi možnostmi, a to s předdefinovaným časem a datem. Tyto hodnoty po zvolení je možno měnit. Aktuálně měněná hodnota je podtržena a zvolená hodnota se mění pomocí pohybů nahorů a dolů (z pohledu k disploji. Přičtení a odečtení hodnoty). Následně po změnení hodnoty a jejím uložením pohybem doprava se ESP32 pokusí připojit k aktuálně nastavené WiFi uvnitř v zařízení. Po úspěšném připojení k WiFi se nastavená hodnota času nebo data odešlou na MQTT klienta. Při odesílání se objeví na displeji "Sending".

### MENU - Person
Pouhé zobrazení dat. Zobrazuje přednastavené hodnoty a majiteli. Jde pouze o jeho jméno a příjmení.

### MENU - Height
Zobrazí další menu, tentokrát se zobrazením aktuální přibližné výšky - menší nebo větší jak 180cm. Ze začátku ovšem zobrazuje možnost "No Data", dokud není uživatele pomocí druhého políčka v menu nastaveno. 

Druhé políčko v menu otevírá možnost volby s otázkou, jestli je uživatel větší nebo menší jak 180cm a uživatel má pouze 2 možnosti volby, a to potvrdit nebo vyvrátit toto tvrzení. Tato volba se v zařízení uloží a je nadále zobrazována uživateli na prvním místě v menu podle toho, jakou z možností zvolil.

## Obsluha 
Pro spuštění programu stačí pouze pomocí micro USB připojit k počítači. Bohužel z implementačních důvodů pro použití MQTT klienta je potřeba tyto údaje ke kleintovi měnit ve zdrojovém kódu, stejně tak přihlašovací údaje na WiFi. Po zadání však funguje vše jak má.

Jak už bylo zmíněno v [Implementaci](#implementační-detaily), prostředí je ovládáno pouze pomocí 4 gest.

## Podmínky zadání
Dle zadání je potřeba:
| Zadání | Čím je to splněno |
| --- | --- |
| Úvodní obrazovka | Hned při spuštění se objeví |
| Minimálně 2 úrovně menu | Základní menu a následně se nachází další (2.) vrstva menu po otevření "Time" nebo "Height" |
| Informativní položky pro čtení | V MENU - "Person" |
| Akce pro potvrzení | Zvolení ano/ne u volby výšky |
| Nastavení číselné hodnoty | Nastavení "Time" nebo "Date" |
| Nastavenou hodnotu odešlete do MQTT | Hodnota se odesílá do MQTT |

## Autor
```
Jméno: Michal Zapletal
Email: xzaple41@stud.fit.vutbr.cz
```


## Reference
<a id="1">[1]</a> SPI displej: <https://dratek.cz/arduino/3202-spi-oled-1-3-displej-128x64-bily.html?gad_source=1&gclid=Cj0KCQiAyeWrBhDDARIsAGP1mWROvW9umWldmhVxzEb_l8powMIDs41wAlr9tiu1CpiPFxX4aVVT9R4aAjZcEALw_wcB>.

<a id="2">[2]</a> I2C senzor: <https://www.laskakit.cz/arduino-senzor-priblizeni-a-gest-gy-9960llc-apds-9960--i2c/>.

<a id="3">[3]</a> Zapojení displeje: <https://moodle.vut.cz/mod/page/view.php?id=338880>.

<a id="4">[4]</a> Demonstrační příklady: <https://github.com/espressif/esp-idf/tree/master/examples/>.

<a id="5">[5]</a> I2C příklady na zapojení displeje: <https://github.com/arduino-libraries/Arduino_APDS9960>.

<a id="6">[6]</a> MyQttHub: <https://myqtthub.com/en>.