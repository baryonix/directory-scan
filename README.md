# Systemvoraussetzungen

Das Projekt wird mit CMake gebaut und setzt einen C++17-fähigen Compiler,
die Boost-Library
sowie das GTest-Framework voraus. Die exakten Minimalanforderungen an die
Versionen sind nicht bekannt. Entwickelt und getestet wurde unter Linux
(Fedora 30) mit Boost 1.69.0 und GTest 1.8.1.

Es wird außerdem vorausgesetzt, dass das Betriebssystem den von POSIX definierten
System-Call `lstat()` zur Verfügung stellt. Eine Impementierung nur mit den
portablen Schnittstellen aus `std::filesystem` bzw. `boost::filesystem` hat sich
als nicht praktikabel herausgestellt. Siehe hierzu auch den Kommentar über der
Funktion `do_lstat()` im Quelltext. Es ist daher möglich, dass der Code in der
vorliegenden Form unter Windows nicht lauffähig ist. Dies wurde nicht getestet.

# Design

Gerade beim Zugriff auf Remote-Dateisysteme können u.U. recht hohe Latenzen
auftreten. Zum Erreichen einer guten Performance (hier: eines hohen Durchsatzes)
ist es daher essentiell, ein gewisses Maß an Parallelität beim Auslesen der
Verzeichnisinhalte zu erreichen -- auch dann, wenn nur wenige oder gar nur ein
einzelnes Verzeichnis gescannt werden soll.

Aus diesem Grund wurde der sequentiell arbeitende `recursive_directory_iterator`
aus der Standardlibrary hier bewusst nicht verwendet.

Andererseits ist es wichtig, den Grad der Parallelität zu begrenzen, um das
I/O-Subsystem des Zielsystems nicht zu überlasten. Somit bietet sich die
Verwendung eines Thread-Pools mit einer festen bzw. begrenzten Anzahl von
Threads an. Ein solcher ist in der Klasse `boost::asio::thread_pool`
implementiert und wird vom hier vorliegenden Code verwendet.

Der Thread-Pool verarbeitet als Tasks Instanzen der Klasse `PathScanner`,
die bei Ausführung jeweils nicht-rekursiv ein einzelnes Verzeichnis durchsuchen.
Für alle dabei gefundenen Dateien wird ein Callback aufgerufen, und für jedes
gefundene Unterverzeichnis wird ein neuer `PathScanner` zur
Ausführung an den Thread-Pool übergeben.

Um den Prozess zu starten, wird der Thread-Pool mit einem einzelnen
`PathScanner` für das Verzeichnis initialisiert, in dem die Suche beginnen
soll. Das gleichzeitige Durchsuchen mehrerer Verzeichnisse ist somit trivial
zu implementieren: Hierzu werden bei der Initialisierung einfach mehrere
`PathScanner` in den Thread-Pool gegeben.

Um eine möglichst hohe Wiederverwendbarkeit zu erreichen, werden die Suchergebnisse
nicht in einer festgelegten Container-Klasse zurückgegeben, sondern es wird
bereits während des Suchvorgangs für jede gefundene Datei ein Callback aufgerufen.
Im Beispielprogramm `scandir` werden diese Callbacks von einer Instanz der Klasse
`VectorCollector` entgegengenommen, die alle Ergebnisse in einem `std::vector`
sammelt, um sie vor der Ausgabe sortieren zu können. Hierbei ist zu beachten,
dass das Callback parallel aus verschiedenen Threads aufgerufen wird und das
Anhängen an den Vektor daher durch einen Mutex synchronisiert werden muss.

# Aufruf

Das Programm `scandir` nimmt eine beliebige Anzahl zu durchsuchender Verzeichnisse
als Argumente von der Kommandozeile entgegen. Außerdem kann mit der Option `-p`
die Größe des Thread-Pools konfiguriert werden. Die Ausgabe der Suchergebnisse
erfolgt per Default auf `stdout`, kann jedoch mit der Option `-o` in eine Datei
umgeleitet werden. Eine Kommandozeilen-Hilfe ist mit der Option `-h` verfügbar.

# Ausgabeformat

Das Programm `scandir` erzeugt als Ausgabe eine Textdatei, die für jede gefundene
Datei eine Zeile enthält. Jede Zeile besteht aus 3 Feldern, die jeweils durch
ein einzelnes Leerzeichen voneinander getrennt sind:

* Zeitstempel der letzten Änderung (`mtime`) im ISO8601-Format mit Zeitzone
* Größe der Datei als Dezimalzahl
* Vollständiger Pfad der Datei

Die Ausgabe kann absolute oder relative Pfade enthalten. Dies ist abhängig von den
eingegebenen, zu durchsuchenden Verzeichnissen. Werden diese als absolute Pfade
spezifiziert, so sind es die Pfade in der Ausgabe ebenfalls.

Da es sich beim Pfad um das letzte Feld in der Zeile handelt, sind Leerzeichen im
Pfad nicht als Trennzeichen zu betrachten und werden daher nicht gequotet ausgegeben.
Es ist allerdings theoretisch auch möglich, dass Pfadnamen Zeilentrenner (`CR` oder
`LF`) enthalten. Damit dies nicht zu einer korrupten Ausgabedatei führt, werden
`CR` und `LF` durch einen vorangestellten Backslash gequotet. Der Backslash selbst
wird durch Verdoppelung ebenfalls gequotet ausgegeben.

# Performance

Es wurde eine Performancemessung im Vergleich mit dem `find`-Befehl unter Linux
durchgeführt. Dabei wurde ein einzelnes Verzeichnis auf SSD-Storage mit insgesamt
ca. 2.5 Millionen Dateien durchsucht.

Vor dem Test wurde jeweils der Dateisystem-Cache über den Befehl
`sysctl -w vm.drop_caches=3` geleert.

## `find`-Befehl

Der Befehl wurde mit folgender Kommandozeile ausgeführt:

```
find /home \! -type d -ls >find.out
```

Die Laufzeit schwankte bei mehrfacher Ausführung etwas, lag im Mittel aber
bei ca. 50 Sekunden.

## `scandir`

Der Befehl wurde wie folgt aufgerufen:

```
./scandir -p 16 -o scandir.out /home
```

Die Gesamtlaufzeit lag bei 17 Sekunden. Der größte Teil der Laufzeit wurde jedoch
für die (nicht parallelisierte) Sortierung und Formatierung der Ergebnisse
aufgewendet. Das eigentliche Durchsuchen des Verzeichnisses war bereits nach ca.
4.1 Sekunden abgeschlossen.

Der maximale Arbeitsspeicherverbrauch von `scandir` lag bei ca. 390 MB bei einer
Größe der Ausgabedatei von 260 MB.

Der Vergleich der Ausgabedateien zeigt, dass `find` und `scandir` exakt gleich
viele Ergebnisse zurückgeliefert haben.
