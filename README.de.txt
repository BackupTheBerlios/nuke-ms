Die ist die Readme datei f�r das Nuclear Messaging System Projekt. In dieser datei finden sie Information zum Projekt und dar�ber wie sie nuke-ms bekommen und benutzen k�nnen.

Das Nuclear Messaging System (nuke-ms) ist gedacht als ein verl�ssliches und sicheres verteiltes Sofortnachrichtensystem, das auf vielen Plattformen verf�gbar ist.
Zurzeit bietet es nur sehr rudiment�re Funktionen und ist wahrscheinlich nur zum Testen und Evaluieren zu gebrauchen.

nuke-ms ist ein freies Softwareprojekt geschrieben von Alexander Korsunsky. Sie k�nnen es unter den Bedingungen der GNU General Public License version 3 (GPLv3) verwenden , ver�ndern und weitergeben. Diese Bedingungen k�nnen sie in der Datei LICENSE.txt im doc/-Verzeichnis nachsehen.

Das Projekt ist auf BerliOS Developer, eine deutsche Organisation zur Unterst�tzung der Entwicklung von Open Source Projekten, gehostet. Verschiedene Dienste wie zum Beispiel ein Git Repository, ein Bugtracker oder ein Wiki werden zur Verf�gung gestellt. Sie k�nnen sich die nuke-ms Projektseite hier ansehen:  http://developer.berlios.de/projects/nuke-ms/


================================================================================
Inhalt dieser Datei:

1. Woher man nuke-ms bekommen kann
    1.1 Git Repository
    1.2 Herunterladen des letzten Releases von der BerliOS Projektseite
    
2. Installation

3. Benutzung

4. Anmerkungen zur Kompatibilit�t


---------------------------------------------------------------------------------

1) Woher man nuke-ms bekommen kann

1.1) Git Repository

Sie k�nnen nuke-ms aus dem Git Versinskontrollsystem auf BerliOS herunterladen.
Dazu, installieren sie Git (anweisungen k�nnen sie hier: http://git.or.cz/gitwiki/Installation oder hier: http://progit.org/book/de/ch1-7.html) und wechseln sie in das Verzeichnis in das sie den Projektordner ablegen wollen. Sie k�nnen das Repository auschecken indem sie folgenden Befehl in die Kommandozeile eingeben:

    git clone git://git.berlios.de/nuke-ms
    
Dieser Befehl erstellt einen Ordner mit dem Namen "nuke-ms" mit der neuesten Version aller Projektdateien. Um mit der Entwicklung von nuke-ms Schritt zu halten, k�nnen sie die neuesten �nderung herunterladen indem sie ins Projektverzeichnis wechseln und aus dem Repository "ziehen":

    $ cd nuke-ms
    $ git pull
    
1.2) Herunterladen des letzten releases von der BerliOS Projektseite

Sie k�nnen den Quellcode und eine f�r Windows kompilierte Version von nuke-ms von der BerliOS Projektseite unter folgenden Addressen herunterladen:

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.1-src.tar.gz
        Die ist die Quellcodeversion von nuke-ms mit Unix Zeilenenden (LF).

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.1-win32.zip
        Dies ist die kompilierte win32 Version, alle Dateien haben Windows Zeilenenden (CR LF).


---------------------------------------------------------------------------------

2) Installation

F�r die kompilierte win32 Version, laden sie das Archiv aus dem Link von oben herunter und entpacken sie es. Die ausf�hrbaren Dateien k�nnen sie im bin/-Verzeichnis des Projektordners finden, sie hei�en "nuke-ms-client.exe" und "nuke-ms-serv.exe". Eine DLL wird von nuke-ms ben�tigt, sie hei�t "mingwm10.dll". Stellen sie sicher dass die DLL-Datei im gleichen Verzeichnis wie die EXE-Dateien liegt.

F�r die Quellcodeversion sehen die Kurzanweisungen zum Kompilieren unter unix�hnlichen Systemen wie folgt aus:
    - Installieren sie CMake >= 2.6, wxWidgets >= 2.8 and Boost >= 1.35
    - Wechseln sie in ein Verzeichnis wo sie nuke-ms bauen wollen, starten sie "cmake" mit dem src/-Verzeichnis des Projektordners und starten sie schlie�lich "make":
        $ cd nuke-ms/build
        $ cmake ../src
        $ make
    Die ausf�hrbaren Dateien k�nnen sie im bin/-Verzeichnis des Verzeichnisses wo sie nuke-ms gebaut haben finden.
    
F�r detaillierte Anweisungen zur Installation der Voraussetzungen und zum Kompilieren auf unix�hnlichen- oder Windowssystemen, sehen sie in der Datei "INSTALL.de.txt" im doc/-Verzeichnis des Projektordners nach.


---------------------------------------------------------------------------------

3. Benutzung

nuke.ms besteht aus zwei Programmen: einem Client der Nachrichten an den Server sended und von dort empf�ngt und "nuke-ms-client" client hei�t und einem einfachen Server, der die Nachrichten von Clients empf�ngt und sie weiterleitet der "nuke-ms-serv" genannt wird.


Starten sie den Server indem sie einfach die Datei nuke-ms-serv ausf�hren. Der Server �bernimmt keine Parameter, zeigt weder eine grafische- noch eine kommandozeilenumgebung sondern lauscht auf dem Port 34443 auf eingehende Verbindungen. Wenn sie eine "n�rgelnde" Firewall haben, m�ssen sie dem Server das Binden an den Port erlauben, also auf den Button "Erlauben", "Nicht blocken", "Entblocken" oder etwas �hnliches im Firewallfenster klicken.
Um den Server zu stoppen m�ssen sie ihn von au�en unterbrechen, das hei�t entweder dadurch dass sie Strg-C im Konsolenfenster angeben oder die Anwendung mit dem "kill"-Programm oder mit dem Task Manager beenden.

Starten sie dann die nuke-ms-client Anwendung. Es sollte nun ein Fenster mit zwei Textfeldern zu sehen sein. Verbinden sie sich mit einem Server, indem sie ins untere Textfeld folgendes eingeben:
    /connect <host>:<port>
Ersetzen sie <host> mit dem Hostnamen des Zielrechners (IP-Addresse oder DNS Name). Wenn das Ihr lokaler Rechner ist, geben sie "localhost" ein. 
Der <port> ist immer 34443. Das erste Zeichen des kommandos muss '/' (ein Slash) sein, ansonsten wird das kommando nicht angenommen.
Wenn sie eine Antwort so �hnlich wie "Connection succeeded." sehen, k�nnen sie nun anfangen Nachrichten einzutippen. Sobald sie die Entertaste gedr�ckt haben wid die Nachricht abgeschickt. Sie k�nnen einen Zeileinschub mit der Tastenkombination Shift-Enter einf�gen. Die Nachricht die sie verschickt haben wird ihnen vom server sofort wieder zur�ckgeschickt.
Wenn der Verbindungsversuch scheitert wird eine Nachricht mit dem Grund angezeigt.
Um eine Verbindung abzubrechen, geben sie das Kommando "/disconnect" ins Texteingabefeld ein.
Um die Anwendung zu beenden, dr�cken sie entweder das X in der oberen rechten Ecke des Fensters, w�hlen sie File->Quit aus der Men�leiste oder geben sie das Kommando "/exit" ein.

Eine Anmerkung zur Sicherheit:
ACHTUNG! nuke-ms bietet keine M�glichkeiten zur Verschl�sselung oder Authentifizierung! Die Identit�t eines Teilnehmers ist �berhaupt nicht gew�hrleistet und die Nachrichten werden v�llig unverschl�sselt �ber das Netzwerk geschickt und k�nnen daher abgeh�rt werden.
Lassen sie den Server nicht auf Sicherheitskritischen ger�ten laufen, denn es ist wahrscheinlich sehr einfach verschiede Angriffe gegen ihn auszuf�hren. Bitte benutzen sie nuke-ms nur in einer sicheren Umgebung (wie z.B. ihrem Heimnetzwerk hinter einer Firewall) und nur zum Testen.
Diese Probleme beruhen auf der Tatsache, dass in dieser fr�hen Entwicklungsphase noch keine Ma�nahmen zur absicherung getroffen wurden. Sicherheit ist allerdings ein Hauptziel dieses Projekts und wird in Zukunft hoffentlich stark verbessert werden.


---------------------------------------------------------------------------------

4. Anmerkungen zur Kompatibilit�t

Die Kommunikation zwischen Clients auf verschieden System, besonders zwischen Linux- und Windowssystemen ist leider nicht m�glich. Der Grund daf�r liegt in den Unterschieden der Standard UTF-Kodierung - Linux benutzt UTF-32 wohingegen Windows UTF-16 benutzt. Dieses Problem wird in naher zukunft behandelt werden.

nuke-ms ist noch keine reife Software. Als solche ist auch das Kommunikationsprotokoll ziwschen den Clients nicht reif und unterliegt andauernden Ver�nderungen. Kompatibilit�t zwischen verschiedenen Versionen ist in dieser fr�hen Phase der Entwicklung nicht gew�hrleistet. Tats�chlich ist ein inkompatible �nderung des Protokolls in naher zukunft geplant. Um also das Programm zur kommunikation zu verwenden sollten sie immer die gleiche Version f�r jeden Teilnehmer verwenden.











